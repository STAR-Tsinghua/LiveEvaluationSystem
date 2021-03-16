#ifndef UDP_SERVER_H
#define UDP_SERVER_H

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


#include <vector>
#include <util_url_file.h>
#include <sodtp_block.h>
#include <bounded_buffer.h>


#define LOCAL_CONN_ID_LEN 16


class CONN_IO {
public:
    ev_timer timer;
    ev_timer sender;

    int sock;

    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;

    UT_hash_handle hh;

    uint32_t send_round;
    std::vector<StreamCtxPtr> vStmCtxPtrs;
    BoundedBuffer<StreamPktVecPtr> buffer;

    std::thread *thd_produce;
};

struct connections {
    int sock;

    const char *conf; // file name of stream resource configuration.

    CONN_IO *h;
};

static struct connections *conns = NULL;

static void timeout_cb(EV_P_ ev_timer *w, int revents);


static void flush_egress(struct ev_loop *loop, CONN_IO *conn_io) {

    ssize_t sent = sendto(conn_io->sock, "hello", 6, 0,
                              (struct sockaddr *) &conn_io->peer_addr,
                              conn_io->peer_addr_len);
    if (sent != 6) {
        perror("failed to send");
        return;
    }

    fprintf(stderr, "sent %zd bytes\n", sent);

    // timeout = 5.0s
    conn_io->timer.repeat = 5.0;
    ev_timer_again(loop, &conn_io->timer);
}

static void send_meta_data(CONN_IO *conn_io, StreamCtxPtr sctx) {
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

    ssize_t sent = sendto(conn_io->sock, buf, sizeof(*header) + sizeof(*meta), 0,
                        (struct sockaddr *) &conn_io->peer_addr,
                        conn_io->peer_addr_len);

    if (sent != sizeof(*header) + sizeof(*meta)) {
        fprintf(stdout, "failed round %d,\t stream %d,\t meta data\n",
                conn_io->send_round, header->stream_id);
    } else {
        fprintf(stdout, "send round %d,\t stream %d,\t meta data\n",
                conn_io->send_round, header->stream_id);
    }

    delete[] buf;
}

// 不走这里，往下看！！！
// To simplify this process.
// We can send frames each 1/25 second.
static void sender_cb1(EV_P_ ev_timer *w, int revents) {
    CONN_IO *conn_io = (CONN_IO *)w->data;
    static AVPacket packet;
    static SodtpStreamHeader header;
    static uint8_t buf[2000000];    // 2Mbytes

    int ret = 0;
    int size = conn_io->vStmCtxPtrs.size();
    fprintf(stderr, "send frame once. fmt num %d\n", size);

    for (auto it = conn_io->vStmCtxPtrs.begin(); it != conn_io->vStmCtxPtrs.end(); ) {
        // printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

        // if ((*it)->flag_meta == false) {
        //     send_meta_data(conn_io, *it);
        //     (*it)->flag_meta = true;
        // }

        ret = file_read_packet2((*it)->pFmtCtx, &packet);

        // printf("send round %d,\t size = %d\n", send_round, packet.size);

        if (ret < 0) {
            header.flag = HEADER_FLAG_FIN;  // end of stream
        } else {
            header.flag = HEADER_FLAG_NULL; // normal state
        }

        header.stream_id = (*it)->stream_id;
        // header.block_ts = packet.dts;
        header.block_ts = current_mtime();
        header.block_id = conn_io->send_round;
        // duration in milli-seconds
        header.duration = packet.duration * 1000;
        header.duration *= (*it)->pFmtCtx->streams[0]->time_base.num;
        header.duration /= (*it)->pFmtCtx->streams[0]->time_base.den;
        fprintf(stderr, "duration %dms\n", header.duration);

        // printf("frame duration %lld, time base: num %d den %d\n", packet.duration,
        //         conn_io->vFmtCtxPtrs[i]->streams[0]->time_base.num,
        //         conn_io->vFmtCtxPtrs[i]->streams[0]->time_base.den);

        if (packet.flags & AV_PKT_FLAG_KEY) {
            header.flag |= HEADER_FLAG_KEY;
        }

        memcpy(buf, &header, sizeof(header));
        memcpy(buf + sizeof(header), packet.data, packet.size);

        ssize_t sent = sendto(conn_io->sock, buf, sizeof(header) + packet.size, 0,
                          (struct sockaddr *) &conn_io->peer_addr,
                          conn_io->peer_addr_len);

        if (sent != sizeof(header) + packet.size) {
            fprintf(stdout, "failed round %d,\t stream %d,\t block %d,\t size %d\n",
                    conn_io->send_round, header.stream_id, conn_io->send_round, packet.size);
        } else {
            fprintf(stdout, "send round %d,\t stream %d,\t block %d,\t size %d\n",
                    conn_io->send_round, header.stream_id, conn_io->send_round, packet.size);
        }

        // timeout = 5.0s
        conn_io->timer.repeat = 5.0;
        ev_timer_again(loop, &conn_io->timer);



        if (ret < 0) {
            fprintf(stderr, "remove a format context\n");
            it = conn_io->vStmCtxPtrs.erase(it);
            // continue;

            // ///
            // // Here, we clear all the context when one file meets EOF.
            // // Because we use vector index, which is i, as the stream id.
            // for (auto &cptr : conn_io->vFmtCtxPtrs) {
            //     avformat_close_input(&cptr);
            // }
            // conn_io->vFmtCtxPtrs.clear();
            // break;
        }
        else {
            it++;
        }

        ///
        av_packet_unref(&packet);
        // printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
        ///
    }
    fflush(stdout);
    conn_io->send_round ++;


    // printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
    if (conn_io->vStmCtxPtrs.empty()) {
        fprintf(stderr, "stop sending\n");
        ev_timer_stop(loop, &conn_io->sender);
    }
    ///
    ///
    // 临时设定，用于debug，需要修改。
    // if (conn_io->send_round > 10) {
    //     ev_timer_stop(loop, &conn_io->sender);
    // }
    ///
    ///

    // flush_egress(loop, conn_io);
    // printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
}

// 走这里
static void sender_cb(EV_P_ ev_timer *w, int revents) {
    CONN_IO *conn_io = (CONN_IO *)w->data;
    static uint8_t buf[2000000];    // 2Mbytes

    int ret = 0;
    int size = 0;
    StreamPktVecPtr pStmPktVec = NULL;

    pStmPktVec = conn_io->buffer.consume();

    if (!pStmPktVec) {
        fprintf(stderr, "stop sending\n");
        ev_timer_stop(loop, &conn_io->sender);
    }
    else {
        size = pStmPktVec->size();
        fprintf(stderr, "send frame once. stream num %d\n", size);
        // timeFrameServer.evalTimeStamp("Net_Consume","s","FrameTime");
        // for (auto &item : *pStmPktVec) {
        // Reverse the iteration of packets.
        for (auto iter = pStmPktVec->rbegin(); iter != pStmPktVec->rend(); iter++) {
            auto &item = *iter;
            // printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

            // if ((*it)->flag_meta == false) {
            //     send_meta_data(conn_io, *it, quiche_sid);
            //     (*it)->flag_meta = true;
            //     quiche_sid += 4;
            // }

            // tag the time stamp.
            item->header.block_ts = current_mtime();

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

            ssize_t sent = sendto(conn_io->sock, buf, sizeof(item->header) + item->packet.size, 0,
                              (struct sockaddr *) &conn_io->peer_addr,
                              conn_io->peer_addr_len);

            if (sent != sizeof(item->header) + item->packet.size) {
                fprintf(stdout, "failed round %d,\t stream %d,\t block %d,\t size %d\n",
                        conn_io->send_round, item->header.stream_id,
                        item->header.block_id, item->packet.size);
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

            ///XData EncodeVideo(XData frame)
            av_packet_unref(&item->packet);
            // printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
            ///
        }
        // timeout = 5.0s
        conn_io->timer.repeat = 5.0;
        ev_timer_again(loop, &conn_io->timer);
    }
    fflush(stdout);
    conn_io->send_round ++;

    ///
    ///
    // 临时设定，用于debug，需要修改。
    // if (conn_io->send_round > 10) {
    //     ev_timer_stop(loop, &conn_io->sender);
    // }
    ///
    ///
}


static CONN_IO *create_conn(EV_P_ struct sockaddr_storage *addr, socklen_t addr_len) {
    // The frame rate should be updated as you need.
    // To be more accurate, there should be a dynamic variable.
    // static const int FRAME_RATE = 25;
    static const int FRAME_RATE = 30;
    static const double interval = 0.040;
    static const double frameRate = 0.033;
    CONN_IO *conn_io = new CONN_IO();
    if (conn_io == NULL) {
        fprintf(stderr, "failed to allocate connection IO\n");
        return NULL;
    }

    conn_io->sock = conns->sock;
    conn_io->send_round = 0;

    memcpy(&conn_io->peer_addr, addr, addr_len);
    conn_io->peer_addr_len = addr_len;


    ev_init(&conn_io->timer, timeout_cb);
    conn_io->timer.data = conn_io;

    // The magic number 1/25 = 0.4. should be updated according to the frame
    // rate of each stream. 
    ev_timer_init(&conn_io->sender, sender_cb, 0., frameRate);
    ev_timer_start(loop, &conn_io->sender);
    conn_io->sender.data = conn_io;
    conn_io->buffer.reset(MAX_BOUNDED_BUFFER_SIZE);
    // Init the stream format context.
    // init_resource(&conn_io->vStmCtxPtrs, conns->conf);

    // conn_io->thd_produce = new std::thread(produce, &conn_io->buffer, conns->conf);
    
    // 改的接口
    conn_io->thd_produce = new std::thread(live_produce, &conn_io->buffer, conns->conf);

    HASH_ADD(hh, conns->h, peer_addr, addr_len, conn_io);

    fprintf(stderr, "new connection\n");

    return conn_io;
}

static void recv_cb(EV_P_ ev_io *w, int revents) {
    CONN_IO *conn_io = NULL;

    static uint8_t buf[2000000];

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

        HASH_FIND(hh, conns->h, &peer_addr, peer_addr_len, conn_io);

        if (conn_io == NULL) {
            conn_io = create_conn(loop, &peer_addr, peer_addr_len);
            if (conn_io == NULL) {
                fprintf(stderr, "fail to create connection.\n");
                return;
            }
        }
        else {
            fprintf(stderr, "connection found!\n");
        }
    }
}

static void timeout_cb(EV_P_ ev_timer *w, int revents) {
    CONN_IO *conn_io = (CONN_IO *)w->data;

    HASH_DELETE(hh, conns->h, conn_io);

    ev_timer_stop(loop, &conn_io->timer);
    fprintf(stderr, "timeout\n");
    delete conn_io;
}

int udp_server(const char *host, const char *port, const char *conf) {

    struct addrinfo hints;

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;


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


    struct connections c;
    c.sock = sock;
    c.conf = conf;
    c.h = NULL;

    conns = &c;


    int maximumBufferSize = 65535;
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char*)&maximumBufferSize, sizeof(int)) < 0)
    {
        perror("failed to set send buffer size");
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char*)&maximumBufferSize, sizeof(int)) < 0)
    {
        perror("failed to set receive buffer size");
        return -1;
    }


    struct ev_loop *loop = ev_default_loop(0);

    ev_io watcher;
    ev_io_init(&watcher, recv_cb, sock, EV_READ);
    ev_io_start(loop, &watcher);
    watcher.data = &c;

    ev_loop(loop, 0);

    freeaddrinfo(local);

    return 0;
}



#endif // UDP_SERVER_H