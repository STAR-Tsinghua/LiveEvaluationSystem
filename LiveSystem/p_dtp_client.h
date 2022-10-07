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

#include "../submodule/DTP/include/quiche.h"
#include <sodtp/p_sodtp_jitter.h>
#include <util_log.h>

#include <argp.h>
#include "helper.h"

/***** Argp configs *****/

static bool TOS_ENABLE = false;
static bool MULIP_ENABLE = false;
static int ip_cfg[8] = {0, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0};

const char *argp_program_version = "dtp-client 0.0.1";
static char doc[] = "live dtp client for test";
static char args_doc[] = "SERVER_IP SERVER_PORT CLIENT_IP CLIENT_PORT [CONFIG_FILE]";
#define ARGS_NUM 5

static bool FEC_ENABLE = false;
static uint32_t TAILSIZE = 5;
static double RATE = 0.1;
static enum quiche_cc_algorithm CC = QUICHE_CC_CUBIC;
static char SCHEDULER[100] = "dyn";
static enum log_level DEBUG_LEVEL = Debug; // debug

static struct argp_option options[] = {
    // {"log", 'l', "FILE", 0, "log file with debug and error info"},
    // {"out", 'o', "FILE", 0, "output file with received file info"},
    // {"log-level", 'v', "LEVEL", 0, "log level ERROR 1 -> TRACE 5"},
    // {"color", 'c', 0, 0, "log with color"},
    {"tos", 't', 0, 0, "set tos"},
    {"mulip", 'm', 0, 0, "set multi ip"},

    {"fec_enable", 'f', 0, 0, "enable fec"},
    {"tailsize", 'z', "TAILSIZE", 0, "set tail size"},
    {"rate", 'r', "RATE", 0, "set redundancy rate"},
    {"cc-algorithm", 'a', "CC", 0, "set the cc algorithm ['reno', 'bbr', 'cubic']"},
    {"debug-level", 'L', "LEVEL", 0, "set debug logging level ['off', 'error', 'warn', 'info', 'debug', 'trace']"},
    {"scheduler-algorithm", 's', "SCHEDULER", 0, "set scheduler algorithm ['basic', 'dtp']"},

    {0}
};

struct arguments {
    FILE *log;
    FILE *out;
    char *args[ARGS_NUM];
};

static struct arguments args;

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = (struct arguments*) state->input;
    switch (key) {
        case 'l':
            arguments->log = fopen(arg, "w+");
            break;
        case 'o':
            arguments->out = fopen(arg, "w+");
            break;
        case 'v':
            LOG_LEVEL = arg ? atoi(arg) : 3;
            break;
        case 'c':
            LOG_COLOR = 1;
            break;
        case 't':
            TOS_ENABLE = true;
            break;
        case 'm':
            MULIP_ENABLE = true;
            break;
        case 'f':
            FEC_ENABLE = true;
            fprintf(stderr, "FEC enabled\n");
            break;
        case 'z':
            TAILSIZE = atoi(arg);
            fprintf(stderr, "set tail size to %d\n", TAILSIZE);
            break;
        case 'r':
            RATE = atof(arg);
            fprintf(stderr, "set redundancy rate to %f\n", RATE);
            break;
        case 'a':
            if(strcmp(arg, "reno") == 0) {
                CC = QUICHE_CC_RENO;               
                fprintf(stderr, "set cc reno\n");
            } else if (strcmp(arg, "bbr") == 0) {
                CC = QUICHE_CC_BBR;
                fprintf(stderr, "set cc bbr\n");
            } else if (strcmp(arg, "cubic") == 0) {
                CC = QUICHE_CC_CUBIC;
                fprintf(stderr, "set cc cubic\n");
            } else {
                fprintf(stderr, "default cc algo reno\n");
                CC = QUICHE_CC_RENO;
            }
            break;
        case 's':
            if(strcmp(arg, "basic") == 0 || strcmp(arg, "dtp") == 0) {
                strcpy(SCHEDULER, arg);
                fprintf(stderr, "copy scheduler %s\n", SCHEDULER);
            } else {
                fprintf(stderr, "default scheduler dyn\n");
            }
            break;
        case 'L': {
           if(!(strcmp(arg, "5") && strcmp(arg, "off"))) {
                DEBUG_LEVEL = log_level::Off;
                fprintf(stderr, "set debug logging level: Off\n");
            } else if (!(strcmp(arg, "4") && strcmp(arg, "error"))) {
                DEBUG_LEVEL = log_level::Error;
                fprintf(stderr, "set debug logging level: Error\n");
            } else if (!(strcmp(arg, "3") && strcmp(arg, "warn"))) {
                DEBUG_LEVEL = log_level::Warn;
                fprintf(stderr, "set debug logging level: Warn\n");
            } else if (!(strcmp(arg, "2") == 0 && strcmp(arg, "info"))) {
                DEBUG_LEVEL = log_level::Info;
                fprintf(stderr, "set debug logging level: Info\n");
            } else if (!(strcmp(arg, "1") == 0 && strcmp(arg, "debug"))) {
                DEBUG_LEVEL = log_level::Debug;
                fprintf(stderr, "set debug logging level: Debug\n");
            } else if (!(strcmp(arg, "0") == 0 && strcmp(arg, "trace"))) {
                DEBUG_LEVEL = log_level::Trace;
                fprintf(stderr, "set debug logging level: Trace\n");
            } else {
                DEBUG_LEVEL = log_level::Debug;
                fprintf(stderr, "set debug logging level: Debug\n"); 
            }
            break;
        }
        case ARGP_KEY_ARG:
            if (state->arg_num >= ARGS_NUM) argp_usage(state);
            arguments->args[state->arg_num] = arg;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < ARGS_NUM - 1) argp_usage(state);
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

#define LOCAL_CONN_ID_LEN 16

#define MAX_DATAGRAM_SIZE 1350

#define MAX_BLOCK_SIZE 10000000 // 10 mbytes

// #define MAX_SEND_TIMES 25
#define MAX_SEND_TIMES 4       // max send times before recving.

class CONN_IO {
public:
    ev_timer timer;
    // ev_io *watcher;

    // int sock;
    int *socks;
    int ai_family;
    ev_io *watchers;

    quiche_conn *conn;

    uint32_t recv_round;
    JitterBuffer *jitter;
};

static void debug_log(const char *line, void *argp) {
    fprintf(stderr, "%s\n", line);
}
static void on_recv_0(EV_P_ ev_io *w, int revents);
static void on_recv_1(EV_P_ ev_io *w, int revents);
static void on_recv_2(EV_P_ ev_io *w, int revents);
static void on_recv_3(EV_P_ ev_io *w, int revents);
static void on_recv_4(EV_P_ ev_io *w, int revents);
static void on_recv_5(EV_P_ ev_io *w, int revents);
static void on_recv_6(EV_P_ ev_io *w, int revents);
static void on_recv_7(EV_P_ ev_io *w, int revents);
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

        int t = 5 << 5;
        int sid = MULIP_ENABLE ? 5 : 0;
        // set_tos(conn_io->ai_family, conn_io->socks[sid], t);
        ssize_t sent = send(conn_io->socks[sid], out, written, 0);
        if (sent != written) {
            perror("failed to send");
            return;
        }

        fprintf(stderr, "sent %zd bytes\n", sent);

        if (++send_times > MAX_SEND_TIMES) {
            // feed the 'READ' state to watcher.
            // ev_feed_event(loop, conn_io->watcher, EV_READ);
            if(MULIP_ENABLE) {
                for(int i = 0;i < 8; ++i) {
                    ev_feed_event(loop, &conn_io->watchers[i], EV_READ);
                }
            } else {
                ev_feed_event(loop, &conn_io->watchers[0], EV_READ);
            }

            // if (ev_is_pending(conn_io->watcher)) {
            if (true) {
                fprintf(stderr, "break the sending.\n");
                break;
            }
            send_times = 1;
        }
    }

    double t = quiche_conn_timeout_as_nanos(conn_io->conn) / 1e9f;
    if(t < 0.00000001) {
      t = 0.001;
    }
    conn_io->timer.repeat = t;
    ev_timer_again(loop, &conn_io->timer);
}
//  接收函数
static void recv_cb(EV_P_ ev_io *w, int revents, int path) {
    // Print2File("dtp_client.h recv_cb :=======");
    static bool req_sent = false;

    CONN_IO *conn_io = (CONN_IO *)w->data;

    static uint8_t buf[65535];
    static SodtpStreamHeader header;
    static BlockDataBuffer bk_buf;

    while (1) {
        //这里是不是读头info信息？
        ssize_t read = recv(conn_io->socks[path], buf, sizeof(buf), 0);
        // Print2File("ssize_t read = recv(conn_io->sock, buf, sizeof(buf), 0); %ld");
        // printf("read: %ld\n", read);
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
    // 循环进入
    // timeMainPlayer.evalTime("p","recv_cb_while_pass");

    if (quiche_conn_is_closed(conn_io->conn)) {
        fprintf(stderr, "connection closed\n");
        Print2File("connection closed");
        ev_break(EV_A_ EVBREAK_ONE);
        return;
    }

    if (quiche_conn_is_established(conn_io->conn) && !req_sent) {
        // timeMainPlayer.evalTime("p","(conn_io->conn) && !req_sent");
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

        req_sent = true
;
    }


    if (quiche_conn_is_established(conn_io->conn)) {
        // timeMainPlayer.evalTime("p","quiche_conn_is_established(conn_io->conn)");
        // 循环进入
        timeMainPlayer.evalTime("p","conn_io->conn");
        // Print2File("if (quiche_conn_is_established(conn_io->conn))");
        uint64_t s = 0;

        quiche_stream_iter *readable = quiche_conn_readable(conn_io->conn);
        timeMainPlayer.evalTime("p","quiche_conn_readable");

        while (quiche_stream_iter_next(readable, &s)) {
            fprintf(stderr, "stream %" PRIu64 " is readable\n", s);
            timeMainPlayer.evalTime("p","quiche_stream_iter_next");
            // Print2File("while (quiche_stream_iter_next(readable, &s))");
            bool fin = false;
            ssize_t recv_len = quiche_conn_stream_recv(conn_io->conn, s,
                                                       buf, sizeof(buf),
                                                       &fin);
            if (recv_len < 0) {
                // 弱网环境出现
                // Print2File("if (recv_len < 0) break ");
                break;
            }
            timeMainPlayer.evalTime("p","recv_len < 0 Pass");

            // printf("%.*s", (int) recv_len, buf);
            fprintf(stderr, "quiche recv len = %d\n", (int)recv_len);
            
            timeFramePlayer.start();
            ///
            ///
            /// Read the frames.
            ///
            ///
            if(bk_buf.write(s, buf, recv_len) != recv_len) {
                fprintf(stderr, "fail to write data to buffer.\n");
                Print2File("fail to write data to buffer ");
            }
            timeMainPlayer.evalTime("p","bk_buf.write");

            // quiche_conn_get_block_info

            // SodtpBlockPtr bk_ptr = SodtpBlockCreate(ptr, recv_len - sizeof(header), false,
            //                 s, header.stream_id, header.block_ts);


            //开始读数据
            if (fin) {
                // Print2File("SodtpBlockPtr BlockDataBuffer::read(uint32_t id, SodtpStreamHeader *head) {");
                SodtpBlockPtr bk_ptr = bk_buf.read(s, &header);
                if (bk_ptr) {
                    long delay = ((long)current_mtime() - header.block_ts);
                    delay = delay < 0 ? 0 : delay;
                    fprintf(stderr, "block ts %lld\n", header.block_ts);
                    fprintf(stderr, "recv round %d,\t stream %u,\t block %u,\t size %d,\t delay %ld\n",
                        conn_io->recv_round, header.stream_id,
                        bk_ptr->block_id, bk_ptr->size,
                        delay);
                    if (header.stream_id > 10) {
                        fprintf(stderr, "Error block, drop it.\n");
                    } else if(delay > 400 && s != 1) {
                        fprintf(stderr, "Block %u miss ddl %ld, drop it.\n", bk_ptr->block_id, delay);
                    }
                    else {
                        conn_io->jitter->push_back(&header, bk_ptr);
                    }

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
    timeMainPlayer.evalTime("p","flush_egress(loop, conn_io);");
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
    timeMainPlayer.evalTime("p","dtp_client");
    Print2FileInfo("(p)dtp_client函数");
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

    struct addrinfo *local;
    if (getaddrinfo(args.args[2], args.args[3], &hints, &local) != 0) {
        log_error("failed to resolve local");
        freeaddrinfo(local);
        return -1;
    }
    
    int socks[8];

    if (MULIP_ENABLE) {
        for (int i = 0; i < 8; i++) {
            socks[i] = socket(local->ai_family, SOCK_DGRAM | SOCK_NONBLOCK, 0);
            if (socks[i] < 0) {
                log_error("failed to create socket");
                return -1;
            }
            struct sockaddr_in6 *local_addr =
                (struct sockaddr_in6 *)local->ai_addr;
            in6_addr_set_byte(&local_addr->sin6_addr, 8, ip_cfg[i]);
            if (bind(socks[i], (struct sockaddr *)local_addr,
                     local->ai_addrlen) != 0) {
                log_error("failed to bind socket %s", strerror(errno));
                return -1;
            }
            struct sockaddr_in6 *peer_addr =
                (struct sockaddr_in6 *)peer->ai_addr;
            in6_addr_set_byte(&peer_addr->sin6_addr, 8, ip_cfg[i]);
            if (connect(socks[i], peer->ai_addr, peer->ai_addrlen) != 0) {
                log_error("failed to connect socket");
                return -1;
            }
        }
    } else {
        socks[0] = socket(local->ai_family, SOCK_DGRAM | SOCK_NONBLOCK, 0);
        if (socks[0] < 0) {
            log_error("failed to create socket");
            return -1;
        }
        if (bind(socks[0], local->ai_addr, local->ai_addrlen) != 0) {
            log_error("failed to bind socket");
            return -1;
        }
        if (connect(socks[0], peer->ai_addr, peer->ai_addrlen) != 0) {
            log_error("failed to connect socket");
            return -1;
        }
    }

    // timeMainPlayer.evalTime("p","socket(peer->ai_family");
    // int sock = socket(peer->ai_family, SOCK_DGRAM, 0);
    // if (sock < 0) {
    //     perror("failed to create socket");
    //     return -1;
    // }

    // if (fcntl(sock, F_SETFL, O_NONBLOCK) != 0) {
    //     perror("failed to make socket non-blocking");
    //     return -1;
    // }

    // if (connect(sock, peer->ai_addr, peer->ai_addrlen) < 0) {
    //     perror("failed to connect socket");
    //     return -1;
    // }

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
    quiche_config_set_cc_algorithm(config, CC);
    quiche_config_set_scheduler_name(config, SCHEDULER);
    if(FEC_ENABLE) {
        quiche_config_set_redundancy_rate(config, RATE);
    } else {
        quiche_config_set_redundancy_rate(config, 0);
    }
    quiche_set_debug_logging_level(DEBUG_LEVEL);

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
    timeMainPlayer.evalTime("p","quiche_connect");
    if (conn == NULL) {
        fprintf(stderr, "failed to create connection\n");
        return -1;
    }

    CONN_IO *conn_io = new CONN_IO();
    if (conn_io == NULL) {
        fprintf(stderr, "failed to allocate connection IO\n");
        return -1;
    }

    if(FEC_ENABLE) {
        quiche_conn_set_tail(conn, TAILSIZE);
    }

    // conn_io->sock = sock;
    conn_io->socks = socks;
    conn_io->conn = conn;
    conn_io->recv_round = 1;
    conn_io->jitter = pJBuffer;

    // ev_io watcher;

    // struct ev_loop *loop = ev_default_loop(0);
    struct ev_loop *loop = ev_loop_new(EVFLAG_AUTO);
    
    ev_io watcher[8];
    if (MULIP_ENABLE) {
        ev_io_init(&watcher[0], on_recv_0, conn_io->socks[0], EV_READ);
        ev_io_start(loop, &watcher[0]);
        watcher[0].data = conn_io;  

        ev_io_init(&watcher[1], on_recv_1, conn_io->socks[1], EV_READ);
        ev_io_start(loop, &watcher[1]);
        watcher[1].data = conn_io;

        ev_io_init(&watcher[2], on_recv_2, conn_io->socks[2], EV_READ);
        ev_io_start(loop, &watcher[2]);
        watcher[2].data = conn_io;

        ev_io_init(&watcher[3], on_recv_3, conn_io->socks[3], EV_READ);
        ev_io_start(loop, &watcher[3]);
        watcher[3].data = conn_io;

        ev_io_init(&watcher[4], on_recv_4, conn_io->socks[4], EV_READ);
        ev_io_start(loop, &watcher[4]);
        watcher[4].data = conn_io;

        ev_io_init(&watcher[5], on_recv_5, conn_io->socks[5], EV_READ);
        ev_io_start(loop, &watcher[5]);
        watcher[5].data = conn_io;

        ev_io_init(&watcher[6], on_recv_6, conn_io->socks[6], EV_READ);
        ev_io_start(loop, &watcher[6]);
        watcher[6].data = conn_io;

        ev_io_init(&watcher[7], on_recv_7, conn_io->socks[7], EV_READ);
        ev_io_start(loop, &watcher[7]);
        watcher[7].data = conn_io;
    } else {
        ev_io_init(&watcher[0], on_recv_0, conn_io->socks[0], EV_READ);
        ev_io_start(loop, &watcher[0]);
        watcher[0].data = conn_io;
        // conn_io->watcher = &watcher[0];
    }
    conn_io->watchers = watcher;

    // recv_cb refrence to dtp_client , important!
    
    Print2FileInfo("(p)启动recv_cb函数处");
    timeMainPlayer.evalTime("p","recv_cb_Start");
    // ev_io_init(&watcher, recv_cb, conn_io->sock, EV_READ);
    // ev_io_start(loop, &watcher);
    // watcher.data = conn_io;
    // conn_io->watcher = &watcher;

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

    for(int i = 0; i < 8; ++i) {
        close(socks[i]);
    }

    return 0;
}

static void on_recv_0(EV_P_ ev_io *w, int revents) {
    recv_cb(loop, w, revents, 0);
}

static void on_recv_1(EV_P_ ev_io *w, int revents) {
    recv_cb(loop, w, revents, 1);
}

static void on_recv_2(EV_P_ ev_io *w, int revents) {
    recv_cb(loop, w, revents, 2);
}

static void on_recv_3(EV_P_ ev_io *w, int revents) {
    recv_cb(loop, w, revents, 3);
}

static void on_recv_4(EV_P_ ev_io *w, int revents) {
    recv_cb(loop, w, revents, 4);
}

static void on_recv_5(EV_P_ ev_io *w, int revents) {
    recv_cb(loop, w, revents, 5);
}

static void on_recv_6(EV_P_ ev_io *w, int revents) {
    recv_cb(loop, w, revents, 6);
}

static void on_recv_7(EV_P_ ev_io *w, int revents) {
    recv_cb(loop, w, revents, 7);
}




#endif // DTP_CLIENT_H

