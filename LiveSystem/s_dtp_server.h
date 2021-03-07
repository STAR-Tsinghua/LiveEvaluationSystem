#ifndef DTP_SERVER_H
#define DTP_SERVER_H

// Copyright (C) 2018, Cloudflare, Inc.
// Copyright (C) 2018, Alessandro Ghedini
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <ev.h>
#include <uthash.h>

#include <quiche.h>

#include <vector>
#include <util_url_file.h>
#include <sodtp_block.h>
#include <bounded_buffer.h>
#include <util_log.h>

#define LOCAL_CONN_ID_LEN 16

#define MAX_DATAGRAM_SIZE 1350

#define MAX_BLOCK_SIZE 10000000

// #define MAX_SEND_TIMES 25
#define MAX_SEND_TIMES 4       // max send times before recving.

#define MAX_TOKEN_LEN \
    sizeof("quiche") - 1 + \
    sizeof(struct sockaddr_storage) + \
    QUICHE_MAX_CONN_ID_LEN

class CONN_IO {
public:
    ev_timer timer;
    ev_timer sender;

    int sock;

    uint8_t cid[LOCAL_CONN_ID_LEN];

    quiche_conn *conn;

    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;

    UT_hash_handle hh;

    uint32_t send_round;
    // std::vector<StreamCtxPtr> vStmCtxPtrs;
    BoundedBuffer<StreamPktVecPtr> buffer;

    std::thread *thd_produce;
};

struct connections {
    int sock;

    ev_io *watcher;

    const char *conf; // file name of stream resource configuration.

    CONN_IO *h;
};

static quiche_config *config = NULL;

static struct connections *conns = NULL;

static void timeout_cb(EV_P_ ev_timer *w, int revents);

static void debug_log(const char *line, void *argp) {
    fprintf(stderr, "%s\n", line);
}

static void flush_egress(struct ev_loop *loop, CONN_IO *conn_io) {
    static uint8_t out[MAX_DATAGRAM_SIZE];

    int send_times = 1;
    while (1) {
        ssize_t written = quiche_conn_send(conn_io->conn, out, sizeof(out));

        if (written == QUICHE_ERR_DONE) {
            fprintf(stderr, "done writing\n");
            break;
        }

        if (written < 0) {
            fprintf(stderr, "failed to create packet: %zd\n", written);
            return;
        }

        ssize_t sent = sendto(conn_io->sock, out, written, 0,
                              (struct sockaddr *) &conn_io->peer_addr,
                              conn_io->peer_addr_len);
        if (sent != written) {
            perror("failed to send");
            return;
        }

        fprintf(stderr, "sent %zd bytes\n", sent);

        if (++send_times > MAX_SEND_TIMES) {
            // feed the 'READ' state to watcher.
            ev_feed_event(loop, conns->watcher, EV_READ);

            // if (ev_is_pending(conns->watcher)) {
            if (true) {
                fprintf(stderr, "break the sending.\n");
                break;
            }
            send_times = 1;
        }
    }

    double t = quiche_conn_timeout_as_nanos(conn_io->conn) / 1e9f;
    conn_io->timer.repeat = t;
    ev_timer_again(loop, &conn_io->timer);
}

static void send_meta_data(CONN_IO *conn_io, StreamCtxPtr sctx, uint64_t quiche_sid) {
    uint8_t *buf = new uint8_t[sizeof(SodtpStreamHeader) + sizeof(SodtpMetaData)];

    if (!buf) {
        fprintf(stderr, "fail to allocate buffer for meta data.\n");
        return;
    }

    SodtpStreamHeader *header = (SodtpStreamHeader*)buf;
    SodtpMetaData *meta = (SodtpMetaData *)(buf + sizeof(*header));

    memset(header, 0, sizeof(*header));
    header->flag = HEADER_FLAG_META;
    header->stream_id = sctx->stream_id;
    header->block_ts = current_mtime();
    header->block_id = -1;

    meta->width = sctx->pFmtCtx->streams[0]->codecpar->width;
    meta->height = sctx->pFmtCtx->streams[0]->codecpar->height;

    // deadline = 1000ms
    // priority = 0
    if (quiche_conn_stream_send_full(conn_io->conn, quiche_sid, buf,
                sizeof(*header) + sizeof(*meta), true, 1000, 0) < 0) {
        fprintf(stdout, "failed round %d,\t stream %d,\t meta data\n",
                conn_io->send_round, header->stream_id);
    } else {
        fprintf(stdout, "send round %d,\t stream %d,\t meta data\n",
                conn_io->send_round, header->stream_id);
    }

    delete[] buf;
}

// 真正发送的地方
// To simplify this process.
// We can send frames each 1/25 second.
static void sender_cb(EV_P_ ev_timer *w, int revents) {
    CONN_IO *conn_io = (CONN_IO *)w->data;
    static uint8_t buf[2000000];    // 2Mbytes
    static uint64_t quiche_sid = 1; // stream id of quiche

    int ret = 0;
    int size = 0;
    int priority = 0;
    int deadline = 0;
    StreamPktVecPtr pStmPktVec = NULL;

    if (quiche_conn_is_established(conn_io->conn)) {
        pStmPktVec = conn_io->buffer.consume();
        if (!pStmPktVec) {
            Print2File("stop sending");
            fprintf(stderr, "stop sending\n");
            ev_timer_stop(loop, &conn_io->sender);
        }
        else {
            // Print2File("else stop sending else else");
            size = pStmPktVec->size();
            fprintf(stderr, "send frame once. stream num %d\n", size);
            // Print2File("befor for (auto &item : *pStmPktVec)");
            for (auto &item : *pStmPktVec) {
                // Print2File("for (auto &item : *pStmPktVec)");
                // printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

                // if ((*it)->flag_meta == false) {
                //     send_meta_data(conn_io, *it, quiche_sid);
                //     (*it)->flag_meta = true;
                //     quiche_sid += 4;
                // }

                // set the value of priority and deadline.
                // key frame, priority = sid * 1024 + 10
                if (item->header.flag & HEADER_FLAG_KEY) {
                    // Print2File("item->header.flag & HEADER_FLAG_KEY");
                    priority = (item->header.stream_id << 10) + 10;
                }
                else {
                    priority = (item->header.stream_id << 10) + 20;
                }

                // for the first block, we set it with the highest priority and enough deadline.
                // 如果拉流端I帧丢失 decoding: waiting for the key block of stream
                if (item->header.block_id == 0) {
                    priority = 2;
                    deadline = 3000;//原来的值
                    deadline = 300000;//lhs改过的
                }
                else {
                    deadline = 300;
                }

                // tag the time stamp.
                item->header.block_ts = current_mtime();
                // Print2File("item->header.block_ts = current_mtime();");
                //深拷贝修改
                memcpy(buf, &item->header, sizeof(item->header));
                // uint8_t     *codecParExtradata;
                // Print2File("*(item->codecParExtradata) : "+std::to_string(sizeof(*(item->codecParExtradata))));
                int extradataSize = item->header.codecPar.extradata_size;
                // Print2File("extradataSize ====== : "+std::to_string(extradataSize));
                // codecParExtradataPtr = (uint8_t*)av_mallocz(extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
                memcpy(buf + sizeof(item->header), item->codecParExtradata, extradataSize);
                memcpy(buf + sizeof(item->header) + extradataSize, item->packet.data, item->packet.size);
                // Print2File("if (quiche_conn_stream_send_full(conn_io->conn, quiche_sid, buf");
                if (quiche_conn_stream_send_full(conn_io->conn, quiche_sid, buf,
                            sizeof(item->header) + sizeof(item->codecParExtradata) + item->packet.size, true, deadline, priority) < 0) {
                    fprintf(stdout, "failed round %d,\t stream %d,\t block %d,\t size %d\n",
                            conn_io->send_round, item->header.stream_id,
                            item->header.block_id, item->packet.size);
                    timeFrameServer.evalTimeStamp("Net_Consume_failed","s",std::to_string(item->header.block_id));
                } else {
                    fprintf(stdout, "send round %d,\t stream %d,\t block %d,\t size %d\n",
                            conn_io->send_round, item->header.stream_id,
                            item->header.block_id, item->packet.size);

                    if (item->packet.flags & AV_PKT_FLAG_KEY) {
                        timeFrameServer.evalTimeStamp("Net_Consume","I_frame",std::to_string(item->header.block_id),std::to_string(item->packet.size));
                    }else{
                        timeFrameServer.evalTimeStamp("Net_Consume","P_frame",std::to_string(item->header.block_id),std::to_string(item->packet.size));
                    }
                }
                quiche_sid += 4;

                ///
                // Print2File("static void sender_cb(EV_P_ ev_timer *w, int revents):  !! av_packet_unref(&item->packet);");
                // 此处对应于的 av_packet_ref() 指向同一内存释放
                av_packet_unref(&item->packet);
                // packet内存释放代替上一句也可以？
                // av_free_packet(&item->packet);
                // printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
                ///
            }
        }
        fflush(stdout);
        conn_io->send_round ++;
        ///
        ///
        // 临时设定，用于debug，需要修改。
        // if (conn_io->send_round > 500) {
        //     ev_timer_stop(loop, &conn_io->sender);
        // }
        ///
        ///

        flush_egress(loop, conn_io);
    }
}
static void mint_token(const uint8_t *dcid, size_t dcid_len,
                       struct sockaddr_storage *addr, socklen_t addr_len,
                       uint8_t *token, size_t *token_len) {
    memcpy(token, "quiche", sizeof("quiche") - 1);
    memcpy(token + sizeof("quiche") - 1, addr, addr_len);
    memcpy(token + sizeof("quiche") - 1 + addr_len, dcid, dcid_len);

    *token_len = sizeof("quiche") - 1 + addr_len + dcid_len;
}

static bool validate_token(const uint8_t *token, size_t token_len,
                           struct sockaddr_storage *addr, socklen_t addr_len,
                           uint8_t *odcid, size_t *odcid_len) {
    if ((token_len < sizeof("quiche") - 1) ||
         memcmp(token, "quiche", sizeof("quiche") - 1)) {
        return false;
    }

    token += sizeof("quiche") - 1;
    token_len -= sizeof("quiche") - 1;

    if ((token_len < addr_len) || memcmp(token, addr, addr_len)) {
        return false;
    }

    token += addr_len;
    token_len -= addr_len;

    if (*odcid_len < token_len) {
        return false;
    }

    memcpy(odcid, token, token_len);
    *odcid_len = token_len;

    return true;
}

static CONN_IO *create_conn(EV_P_ uint8_t *odcid, size_t odcid_len) {
    // The frame rate should be updated as you need.
    // To be more accurate, there should be a dynamic variable.
    static const int FRAME_RATE = 30;
    static const double interval = 0.040;
    static const double frameRate = 0.033;
    

    CONN_IO *conn_io = new CONN_IO();
    if (conn_io == NULL) {
        fprintf(stderr, "failed to allocate connection IO\n");
        return NULL;
    }

    int rng = open("/dev/urandom", O_RDONLY);
    if (rng < 0) {
        perror("failed to open /dev/urandom");
        return NULL;
    }

    ssize_t rand_len = read(rng, conn_io->cid, LOCAL_CONN_ID_LEN);
    if (rand_len < 0) {
        perror("failed to create connection ID");
        return NULL;
    }

    quiche_conn *conn = quiche_accept(conn_io->cid, LOCAL_CONN_ID_LEN,
                                      odcid, odcid_len, config);
    if (conn == NULL) {
        fprintf(stderr, "failed to create connection\n");
        return NULL;
    }

    conn_io->sock = conns->sock;
    conn_io->conn = conn;
    conn_io->send_round = 0;

    ev_init(&conn_io->timer, timeout_cb);
    conn_io->timer.data = conn_io;
    conn_io->buffer.reset(MAX_BOUNDED_BUFFER_SIZE);
    // Init the stream format context.
    // init_resource(&conn_io->vStmCtxPtrs, conns->conf);

    // 原接口！！！
    // Print2File("-----------------------------原接口-----------------------------");
    // conn_io->thd_produce = new std::thread(produce, &conn_io->buffer, conns->conf);

    // 三种协议处
    // 改的接口！！！
    // Print2File("-----------------------------改的接口-----------------------------");
    Print2FileInfo("(s)启动live_produce线程处");
    timeMainServer.evalTime("s","before_live_produce");
    conn_io->thd_produce = new std::thread(live_produce, &conn_io->buffer, conns->conf);

    // The magic number 1/25 = 0.4. should be updated according to the frame
    // rate of each stream. 
    Print2FileInfo("(s)启动sender_cb线程处");
    ev_timer_init(&conn_io->sender, sender_cb, 0., frameRate);
    ev_timer_start(loop, &conn_io->sender);
    conn_io->sender.data = conn_io;

    HASH_ADD(hh, conns->h, cid, LOCAL_CONN_ID_LEN, conn_io);

    fprintf(stderr, "new connection\n");

    return conn_io;
}

static void recv_cb(EV_P_ ev_io *w, int revents) {
    CONN_IO *tmp, *conn_io = NULL;
    static uint8_t buf[MAX_BLOCK_SIZE];
    static uint8_t out[MAX_DATAGRAM_SIZE];
    
    while (1) {
        struct sockaddr_storage peer_addr;
        socklen_t peer_addr_len = sizeof(peer_addr);
        memset(&peer_addr, 0, peer_addr_len);

        ssize_t read = recvfrom(conns->sock, buf, sizeof(buf), 0,
                                (struct sockaddr *) &peer_addr,
                                &peer_addr_len);

        if (read < 0) {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                fprintf(stderr, "recv would block\n");
                break;
            }

            perror("failed to read");
            return;
        }

        uint8_t type;
        uint32_t version;

        uint8_t scid[QUICHE_MAX_CONN_ID_LEN];
        size_t scid_len = sizeof(scid);

        uint8_t dcid[QUICHE_MAX_CONN_ID_LEN];
        size_t dcid_len = sizeof(dcid);

        uint8_t odcid[QUICHE_MAX_CONN_ID_LEN];
        size_t odcid_len = sizeof(odcid);

        uint8_t token[MAX_TOKEN_LEN];
        size_t token_len = sizeof(token);

        int rc = quiche_header_info(buf, read, LOCAL_CONN_ID_LEN, &version,
                                    &type, scid, &scid_len, dcid, &dcid_len,
                                    token, &token_len);
        if (rc < 0) {
            fprintf(stderr, "failed to parse header: %d\n", rc);
            return;
        }

        HASH_FIND(hh, conns->h, dcid, dcid_len, conn_io);

        if (conn_io == NULL) {
            if (version != QUICHE_PROTOCOL_VERSION) {
                fprintf(stderr, "version negotiation\n");

                ssize_t written = quiche_negotiate_version(scid, scid_len,
                                                           dcid, dcid_len,
                                                           out, sizeof(out));

                if (written < 0) {
                    fprintf(stderr, "failed to create vneg packet: %zd\n",
                            written);
                    return;
                }

                ssize_t sent = sendto(conns->sock, out, written, 0,
                                      (struct sockaddr *) &peer_addr,
                                      peer_addr_len);
                if (sent != written) {
                    perror("failed to send");
                    return;
                }

                fprintf(stderr, "sent %zd bytes\n", sent);
                return;
            }

            if (token_len == 0) {
                fprintf(stderr, "stateless retry\n");

                mint_token(dcid, dcid_len, &peer_addr, peer_addr_len,
                           token, &token_len);

                ssize_t written = quiche_retry(scid, scid_len,
                                               dcid, dcid_len,
                                               dcid, dcid_len,
                                               token, token_len,
                                               out, sizeof(out));

                if (written < 0) {
                    fprintf(stderr, "failed to create retry packet: %zd\n",
                            written);
                    return;
                }

                ssize_t sent = sendto(conns->sock, out, written, 0,
                                      (struct sockaddr *) &peer_addr,
                                      peer_addr_len);
                if (sent != written) {
                    perror("failed to send");
                    return;
                }

                fprintf(stderr, "sent %zd bytes\n", sent);
                return;
            }


            if (!validate_token(token, token_len, &peer_addr, peer_addr_len,
                               odcid, &odcid_len)) {
                fprintf(stderr, "invalid address validation token\n");
                return;
            }
            Print2FileInfo("(s)create_conn函数");
            timeMainServer.evalTime("s","before_create_conn");
            // 之前准备
            conn_io = create_conn(loop, odcid, odcid_len);
            if (conn_io == NULL) {
                return;
            }

            memcpy(&conn_io->peer_addr, &peer_addr, peer_addr_len);
            conn_io->peer_addr_len = peer_addr_len;
        }

        ssize_t done = quiche_conn_recv(conn_io->conn, buf, read);

        if (done == QUICHE_ERR_DONE) {
            fprintf(stderr, "done reading\n");
            break;
        }

        if (done < 0) {
            fprintf(stderr, "failed to process packet: %zd\n", done);
            return;
        }

        fprintf(stderr, "recv %zd bytes\n", done);

        if (quiche_conn_is_established(conn_io->conn)) {
            uint64_t s = 0;

            quiche_stream_iter *readable = quiche_conn_readable(conn_io->conn);

            while (quiche_stream_iter_next(readable, &s)) {
                fprintf(stderr, "stream %" PRIu64 " is readable\n", s);

                bool fin = false;
                ssize_t recv_len = quiche_conn_stream_recv(conn_io->conn, s,
                                                           buf, sizeof(buf),
                                                           &fin);
                if (recv_len < 0) {
                    break;
                }
                fprintf(stderr, "quiche recv_len = %d\n", (int)recv_len);

                if (fin) {
                    fprintf(stderr, "stream %" PRIu64 " has been received\n", s);
                    fprintf(stderr, "bct of stream is %" PRIu64 "ms.\n", quiche_conn_get_bct(conn_io->conn, s));
                }
            }

            quiche_stream_iter_free(readable);
        }
    }

    HASH_ITER(hh, conns->h, conn_io, tmp) {
        flush_egress(loop, conn_io);

        if (quiche_conn_is_closed(conn_io->conn)) {
            HASH_DELETE(hh, conns->h, conn_io);

            ev_timer_stop(loop, &conn_io->timer);
            quiche_conn_free(conn_io->conn);
            delete conn_io;
        }
    }
}

static void timeout_cb(EV_P_ ev_timer *w, int revents) {
    CONN_IO *conn_io = (CONN_IO *)w->data;
    quiche_conn_on_timeout(conn_io->conn);

    fprintf(stderr, "timeout\n");

    flush_egress(loop, conn_io);

    if (quiche_conn_is_closed(conn_io->conn)) {
        quiche_stats stats;

        quiche_conn_stats(conn_io->conn, &stats);
        fprintf(stderr, "connection closed, recv=%zu sent=%zu lost=%zu rtt=%" PRIu64 "ns\n",
                stats.recv, stats.sent, stats.lost, stats.rtt);

        HASH_DELETE(hh, conns->h, conn_io);

        ev_timer_stop(loop, &conn_io->timer);
        ev_timer_stop(loop, &conn_io->sender);
        quiche_conn_free(conn_io->conn);
        delete conn_io;

        return;
    }
}

int dtp_server(const char *host, const char *port, const char *conf) {
    struct addrinfo hints;

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    // quiche_enable_debug_logging(debug_log, NULL);

    struct addrinfo *local;
    if (getaddrinfo(host, port, &hints, &local) != 0) {
        perror("failed to resolve host");
        return -1;
    }

    int sock = socket(local->ai_family, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("failed to create socket");
        return -1;
    }

    if (fcntl(sock, F_SETFL, O_NONBLOCK) != 0) {
        perror("failed to make socket non-blocking");
        return -1;
    }

    if (bind(sock, local->ai_addr, local->ai_addrlen) < 0) {
        perror("failed to connect socket");
        return -1;
    }

    config = quiche_config_new(QUICHE_PROTOCOL_VERSION);
    if (config == NULL) {
        fprintf(stderr, "failed to create config\n");
        return -1;
    }

    quiche_config_load_cert_chain_from_pem_file(config, "config/cert.crt");
    quiche_config_load_priv_key_from_pem_file(config, "config/cert.key");

    quiche_config_set_application_protos(config,
        (uint8_t *) "\x05hq-27\x05hq-25\x05hq-24\x05hq-23\x08http/0.9", 21);

    quiche_config_set_max_idle_timeout(config, 5000);
    quiche_config_set_max_packet_size(config, MAX_DATAGRAM_SIZE);
    quiche_config_set_initial_max_data(config, 10000000000);
    quiche_config_set_initial_max_stream_data_bidi_local(config, 1000000000);
    quiche_config_set_initial_max_stream_data_bidi_remote(config, 1000000000);
    quiche_config_set_initial_max_streams_bidi(config, 1000000);
    quiche_config_set_cc_algorithm(config, QUICHE_CC_RENO);

    struct connections c;
    c.sock = sock;
    c.conf = conf;
    c.h = NULL;

    conns = &c;

    ev_io watcher;

    struct ev_loop *loop = ev_default_loop(0);

    timeMainServer.evalTime("s","Before_recv_cb");
    Print2FileInfo("(s)启动recv_cb函数处");
    ev_io_init(&watcher, recv_cb, sock, EV_READ);
    ev_io_start(loop, &watcher);
    watcher.data = &c;
    conns->watcher = &watcher;

    ev_loop(loop, 0);

    freeaddrinfo(local);

    quiche_config_free(config);

    return 0;
}



#endif // DTP_SERVER_H