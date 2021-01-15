#ifndef P_DTP_CLIENT_H
#define P_DTP_CLIENT_H

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

#include <quiche.h>
#include <p_sodtp_jitter.h>
#include <util_log.h>

#define LOCAL_CONN_ID_LEN 16

#define MAX_DATAGRAM_SIZE 1350

#define MAX_BLOCK_SIZE 10000000 // 10 mbytes

// #define MAX_SEND_TIMES 25
#define MAX_SEND_TIMES 4       // max send times before recving.

class CONN_IO {
public:
    ev_timer timer;
    ev_io *watcher;

    int sock;

    quiche_conn *conn;

    uint32_t recv_round;
    JitterBuffer *jitter;
};

static void debug_log(const char *line, void *argp) {
    fprintf(stderr, "%s\n", line);
}

//这个函数用意还需要理解
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

        ssize_t sent = send(conn_io->sock, out, written, 0);
        if (sent != written) {
            perror("failed to send");
            return;
        }

        fprintf(stderr, "sent %zd bytes\n", sent);

        if (++send_times > MAX_SEND_TIMES) {
            // feed the 'READ' state to watcher.
            ev_feed_event(loop, conn_io->watcher, EV_READ);

            // if (ev_is_pending(conn_io->watcher)) {
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
//  接收函数
static void recv_cb(EV_P_ ev_io *w, int revents) {
    // Print2File("dtp_client.h recv_cb :=======");
    static bool req_sent = false;

    CONN_IO *conn_io = (CONN_IO *)w->data;

    static uint8_t buf[65535];
    static SodtpStreamHeader header;
    static BlockDataBuffer bk_buf;

    while (1) {
        //这里是不是读头info信息？
        ssize_t read = recv(conn_io->sock, buf, sizeof(buf), 0);
        // Print2File("ssize_t read = recv(conn_io->sock, buf, sizeof(buf), 0);");
        if (read < 0) {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                // Print2File("recv would block");
                fprintf(stderr, "recv would block\n");
                break;
            }

            perror("failed to read");
            return;
        }

        ssize_t done = quiche_conn_recv(conn_io->conn, buf, read);

        if (done == QUICHE_ERR_DONE) {
            fprintf(stderr, "done reading\n");
            // Print2File("recv would block");
            break;
        }

        if (done < 0) {
            fprintf(stderr, "failed to process packet\n");
            Print2File("failed to process packet");
            return;
        }

        fprintf(stderr, "recv %zd bytes\n", done);
    }

    if (quiche_conn_is_closed(conn_io->conn)) {
        fprintf(stderr, "connection closed\n");
        Print2File("connection closed");
        ev_break(EV_A_ EVBREAK_ONE);
        return;
    }

    if (quiche_conn_is_established(conn_io->conn) && !req_sent) {
        const uint8_t *app_proto;
        size_t app_proto_len;

        quiche_conn_application_proto(conn_io->conn, &app_proto, &app_proto_len);

        fprintf(stderr, "connection established: %.*s\n",
                (int) app_proto_len, app_proto);

        const static uint8_t r[] = "GET /index.html\r\n";
        if (quiche_conn_stream_send(conn_io->conn, 4, r, sizeof(r), true) < 0) {
            fprintf(stderr, "failed to send HTTP request\n");
            return;
        }

        fprintf(stderr, "sent HTTP request\n");

        req_sent = true;
    }


    if (quiche_conn_is_established(conn_io->conn)) {
        // Print2File("if (quiche_conn_is_established(conn_io->conn))");
        uint64_t s = 0;

        quiche_stream_iter *readable = quiche_conn_readable(conn_io->conn);

        while (quiche_stream_iter_next(readable, &s)) {
            fprintf(stderr, "stream %" PRIu64 " is readable\n", s);

            // Print2File("while (quiche_stream_iter_next(readable, &s))");
            bool fin = false;
            ssize_t recv_len = quiche_conn_stream_recv(conn_io->conn, s,
                                                       buf, sizeof(buf),
                                                       &fin);
            if (recv_len < 0) {
                Print2File("if (recv_len < 0) break ");
                break;
            }

            // printf("%.*s", (int) recv_len, buf);
            fprintf(stderr, "quiche recv len = %d\n", (int)recv_len);

            ///
            ///
            /// Read the frames.
            ///
            ///
            if(bk_buf.write(s, buf, recv_len) != recv_len) {
                fprintf(stderr, "fail to write data to buffer.\n");
                Print2File("fail to write data to buffer ");
            }

            // quiche_conn_get_block_info

            // SodtpBlockPtr bk_ptr = SodtpBlockCreate(ptr, recv_len - sizeof(header), false,
            //                 s, header.stream_id, header.block_ts);


            if (fin) {
                // Print2File("SodtpBlockPtr BlockDataBuffer::read(uint32_t id, SodtpStreamHeader *head) {");
                SodtpBlockPtr bk_ptr = bk_buf.read(s, &header);
                if (bk_ptr) {
                    fprintf(stderr, "block ts %lld\n", header.block_ts);
                    fprintf(stderr, "recv round %d,\t stream %d,\t block %d,\t size %d,\t delay %d\n",
                        conn_io->recv_round, header.stream_id,
                        bk_ptr->block_id, bk_ptr->size,
                        (int)(current_mtime() - header.block_ts));
                        // Print2File("conn_io->jitter->push_back(&header, bk_ptr)");
                    // Print2File("conn_io->jitter->push_back(&header, bk_ptr); 真正");
                    conn_io->jitter->push_back(&header, bk_ptr);
                    conn_io->recv_round++;
                }
            }

            // send back an ack in application layer.
            static const uint8_t echo[] = "echo\n";
            if (quiche_conn_stream_send(conn_io->conn, 8, echo, sizeof(echo), false) < sizeof(echo)) {
                fprintf(stderr, "fail to echo back.\n");
                Print2File("fail to echo back");
            }

            // fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
        }

        quiche_stream_iter_free(readable);
    }

    flush_egress(loop, conn_io);
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

        ev_break(EV_A_ EVBREAK_ONE);

        delete conn_io;
        return;
    }
}

// int main(int argc, char *argv[]) {
//     const char *host = argv[1];
//     const char *port = argv[2];
int dtp_client(const char *host, const char *port, JitterBuffer *pJBuffer) {
    Print2File("dtp_client.h dtp_client :=======");
    struct addrinfo hints;

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    // quiche_enable_debug_logging(debug_log, NULL);

    struct addrinfo *peer;
    if (getaddrinfo(host, port, &hints, &peer) != 0) {
        perror("failed to resolve host");
        return -1;
    }

    int sock = socket(peer->ai_family, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("failed to create socket");
        return -1;
    }

    if (fcntl(sock, F_SETFL, O_NONBLOCK) != 0) {
        perror("failed to make socket non-blocking");
        return -1;
    }

    if (connect(sock, peer->ai_addr, peer->ai_addrlen) < 0) {
        perror("failed to connect socket");
        return -1;
    }

    quiche_config *config = quiche_config_new(0xbabababa);
    if (config == NULL) {
        fprintf(stderr, "failed to create config\n");
        return -1;
    }

    quiche_config_set_application_protos(config,
        (uint8_t *) "\x05hq-27\x05hq-25\x05hq-24\x05hq-23\x08http/0.9", 15);

    quiche_config_set_max_idle_timeout(config, 5000);
    quiche_config_set_max_packet_size(config, MAX_DATAGRAM_SIZE);
    quiche_config_set_initial_max_data(config, 10000000000);
    quiche_config_set_initial_max_stream_data_bidi_remote(config, 1000000000);
    quiche_config_set_initial_max_stream_data_bidi_local(config, 1000000000);
    quiche_config_set_initial_max_stream_data_uni(config, 1000000000);
    quiche_config_set_initial_max_streams_bidi(config, 1000000);
    quiche_config_set_initial_max_streams_uni(config, 1000000);
    quiche_config_set_disable_active_migration(config, true);

    if (getenv("SSLKEYLOGFILE")) {
      quiche_config_log_keys(config);
    }

    uint8_t scid[LOCAL_CONN_ID_LEN];
    int rng = open("/dev/urandom", O_RDONLY);
    if (rng < 0) {
        perror("failed to open /dev/urandom");
        return -1;
    }

    ssize_t rand_len = read(rng, &scid, sizeof(scid));
    if (rand_len < 0) {
        perror("failed to create connection ID");
        return -1;
    }

    quiche_conn *conn = quiche_connect(host, (const uint8_t *) scid,
                                       sizeof(scid), config);
    if (conn == NULL) {
        fprintf(stderr, "failed to create connection\n");
        return -1;
    }

    CONN_IO *conn_io = new CONN_IO();
    if (conn_io == NULL) {
        fprintf(stderr, "failed to allocate connection IO\n");
        return -1;
    }

    conn_io->sock = sock;
    conn_io->conn = conn;
    conn_io->recv_round = 1;
    conn_io->jitter = pJBuffer;

    ev_io watcher;

    // struct ev_loop *loop = ev_default_loop(0);
    struct ev_loop *loop = ev_loop_new(EVFLAG_AUTO);

    // recv_cb refrence to dtp_client , important!
    
    ev_io_init(&watcher, recv_cb, conn_io->sock, EV_READ);
    ev_io_start(loop, &watcher);
    watcher.data = conn_io;
    conn_io->watcher = &watcher;

    ev_init(&conn_io->timer, timeout_cb);
    conn_io->timer.data = conn_io;

    flush_egress(loop, conn_io);


    int time1 = current_mtime();
    ev_loop(loop, 0);
    fprintf(stderr, "client interval: %ds\n", ((int)current_mtime()-time1)/1000);

    freeaddrinfo(peer);

    quiche_conn_free(conn);

    quiche_config_free(config);

    // Notification for the buffer clearing.
    // We prefer not to explicitly notify the close of connection.
    // When jitter buffer is empty, the thread will stop automatically.
    // But this is for the corner case, e.g. the breaking of connection.
    sem_post(pJBuffer->sem);
    ev_feed_signal(SIGUSR1);

    return 0;
}





#endif // DTP_CLIENT_H