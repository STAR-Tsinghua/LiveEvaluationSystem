#include <argp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ev.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netdb.h>
#include <quiche.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "dtp_config.h"
#include "helper.h"
#include "uthash.h"

static bool TOS_ENABLE = false;
static bool MULIP_ENABLE = false;
static int ip_cfg[8] = {0, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0};

/***** Argp configs *****/

const char *argp_program_version = "server-test 0.0.1";
static char doc[] = "libev dtp server for test";
static char args_doc[] = "SERVER_IP SERVER_PORT DTP_CONFIG";
#define ARGS_NUM 3

static bool FEC_ENABLE = false;
static uint32_t TAILSIZE = 5;
static double RATE = 0.1;
static enum quiche_cc_algorithm CC = QUICHE_CC_CUBIC;
static char SCHEDULER[100] = "dyn";
static enum log_level DEBUG_LEVEL = Debug; // debug

static struct argp_option options[] = {
    {"log", 'l', "FILE", 0, "log file with debug and error info"},
    {"out", 'o', "FILE", 0, "output file with received file info"},
    {"log-level", 'v', "LEVEL", 0, "log level ERROR 1 -> TRACE 5"},
    {"color", 'c', 0, 0, "log with color"},
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
    struct arguments *arguments = state->input;
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
                DEBUG_LEVEL = Off;
                fprintf(stderr, "set debug logging level: Off\n");
            } else if (!(strcmp(arg, "4") && strcmp(arg, "error"))) {
                DEBUG_LEVEL = Error;
                fprintf(stderr, "set debug logging level: Error\n");
            } else if (!(strcmp(arg, "3") && strcmp(arg, "warn"))) {
                DEBUG_LEVEL = Warn;
                fprintf(stderr, "set debug logging level: Warn\n");
            } else if (!(strcmp(arg, "2") == 0 && strcmp(arg, "info"))) {
                DEBUG_LEVEL = Info;
                fprintf(stderr, "set debug logging level: Info\n");
            } else if (!(strcmp(arg, "1") == 0 && strcmp(arg, "debug"))) {
                DEBUG_LEVEL = Debug;
                fprintf(stderr, "set debug logging level: Debug\n");
            } else if (!(strcmp(arg, "0") == 0 && strcmp(arg, "trace"))) {
                DEBUG_LEVEL = Trace;
                fprintf(stderr, "set debug logging level: Trace\n");
            } else {
                DEBUG_LEVEL = Debug;
                fprintf(stderr, "set debug logging level: Debug\n"); 
            }
            break;
        }
        case ARGP_KEY_ARG:
            if (state->arg_num >= ARGS_NUM) argp_usage(state);
            arguments->args[state->arg_num] = arg;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < ARGS_NUM) argp_usage(state);
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

#undef HELPER_LOG
#undef HELPER_OUT
#define HELPER_LOG args.log
#define HELPER_OUT args.out

/***** DTP configs *****/

#define LOCAL_CONN_ID_LEN 16

#define MAX_DATAGRAM_SIZE 1350

#define MAX_BLOCK_SIZE 1000000  // 1Mbytes

uint64_t total_udp_bytes = 0;
uint64_t started_at = 0;

char *dtp_cfg_fname;
int cfgs_len;
struct dtp_config *cfgs = NULL;

#define MAX_TOKEN_LEN                                        \
    sizeof("quiche") - 1 + sizeof(struct sockaddr_storage) + \
        QUICHE_MAX_CONN_ID_LEN

struct connections {
    int *socks;
    int ai_family;

    struct conn_io *h;
};

struct conn_io {
    ev_timer timer;
    ev_timer sender;
    int send_round;
    int configs_len;
    dtp_config *configs;

    int *socks;
    int ai_family;

    uint64_t t_last;
    ssize_t can_send;
    bool done_writing;
    ev_timer pace_timer;

    uint8_t cid[LOCAL_CONN_ID_LEN];

    quiche_conn *conn;

    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;

    UT_hash_handle hh;
};

static quiche_config *config = NULL;

static struct connections *conns = NULL;

uint8_t tos(uint64_t ddl, uint64_t prio) {
    uint8_t d, p;

    // 0x64 100ms
    // 0xC8 200ms
    // 0x1F4 500ms
    // 0x3E8 1m
    // 0xEA60 1min
    if (ddl < 0x64) {
        d = 5;
    } else if (ddl < 0xC8) {
        d = 4;
    } else if (ddl < 0x1F4) {
        d = 3;
    } else if (ddl < 0x3E8) {
        d = 2;
    } else if (ddl < 0xEA60) {
        d = 1;
    } else {
        d = 0;
    }

    if (prio < 2) {
        p = 5 - prio;
    } else if (prio < 4) {
        p = 3;
    } else if (prio < 8) {
        p = 2;
    } else if (prio < 16) {
        p = 1;
    } else {
        p = 0;
    }

    return (d > p) ? d : p;
}

void set_tos(int ai_family, int sock, int tos) {
    if (!TOS_ENABLE) {
        return;
    }

    switch (ai_family) {
        case AF_INET:
            if (setsockopt(sock, IPPROTO_IP, IP_TOS, &tos, sizeof(int)) < 0) {
                log_debug("Warning: Cannot set TOS!");
            }
            break;

        case AF_INET6:
            if (setsockopt(sock, IPPROTO_IPV6, IPV6_TCLASS, &tos, sizeof(int)) <
                0) {
                log_debug("Warning: Cannot set TOS!");
            }
            break;

        default:
            break;
    }
}

static void timeout_cb(EV_P_ ev_timer *w, int revents);

static void flush_egress(struct ev_loop *loop, struct conn_io *conn_io) {
    // log_debug("enter flush");
    static uint8_t out[MAX_DATAGRAM_SIZE];
    uint64_t rate = quiche_bbr_get_pacing_rate(conn_io->conn);  // bits/s
    if (conn_io->done_writing) {
        conn_io->can_send = 1350;
        conn_io->t_last = getCurrentUsec();
        conn_io->done_writing = false;
    }

    struct Block *stream_blocks = NULL;
    size_t stream_blocks_num = 0;

    while (1) {
        stream_blocks_num = 0;
        uint64_t t_now = getCurrentUsec();
        conn_io->can_send += rate * (t_now - conn_io->t_last) /
                             8000000;  //(bits/8)/s * s = bytes
        // log_debug("%ld us time went, %ld bytes can send",
        //         t_now - conn_io->t_last, conn_io->can_send);
        conn_io->t_last = t_now;
        if (conn_io->can_send < 1350) {
            // log_debug("can_send < 1350");
            conn_io->pace_timer.repeat = 0.001;
            ev_timer_again(loop, &conn_io->pace_timer);
            break;
        }
        ssize_t written = quiche_conn_send(conn_io->conn, out, sizeof(out),
                                           &stream_blocks, &stream_blocks_num);

        if (written == QUICHE_ERR_DONE) {
            // log_debug("done writing");
            conn_io->done_writing = true;  // app_limited
            conn_io->pace_timer.repeat = 99999.0;
            ev_timer_again(loop, &conn_io->pace_timer);
            break;
        }

        if (written < 0) {
            // log_debug("failed to create packet: %zd", written);
            return;
        }

        // if (stream_blocks_num > 0) {
        //   for (int i = 0; i < stream_blocks_num; ++i) {
        //     printf("%d: stream_id: %ld, ddl: %ld, priority: %ld\n", i,
        //            stream_blocks[i].block_id,
        //            stream_blocks[i].block_deadline,
        //            stream_blocks[i].block_priority);
        //   }
        //   free(stream_blocks);
        // }

        int t = 0;
        int sid = 0;
        if (stream_blocks_num > 0) {
            sid = tos(stream_blocks[0].block_deadline,
                      stream_blocks[0].block_priority);
            t = sid << 5;
            sid = MULIP_ENABLE ? sid : 0;
            set_tos(conn_io->ai_family, conn_io->socks[sid], t);
            in6_addr_set_byte(
                &((struct sockaddr_in6 *)&conn_io->peer_addr)->sin6_addr, 8,
                ip_cfg[sid]);
        }
        ssize_t sent = sendto(conn_io->socks[sid], out, written, 0,
                              (struct sockaddr *)&conn_io->peer_addr,
                              conn_io->peer_addr_len);
        if (sent != written) {
            // log_error("failed to send");
            return;
        }

        // log_debug("sent %zd bytes", sent);
        conn_io->can_send -= sent;
    }

    double t = quiche_conn_timeout_as_nanos(conn_io->conn) / 1e9f;
    // timer.repeat can't be 0.0
    if (t <= 0.00000001) {
        t = 0.001;
    }
    conn_io->timer.repeat = t;
    ev_timer_again(loop, &conn_io->timer);
}

static void flush_egress_pace(EV_P_ ev_timer *pace_timer, int revents) {
    struct conn_io *conn_io = pace_timer->data;
    // log_debug("begin flush_egress_pace");
    flush_egress(loop, conn_io);
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

static void sender_cb(EV_P_ ev_timer *w, int revents) {
    // log_debug("enter sender cb");
    struct conn_io *conn_io = w->data;

    if (quiche_conn_is_established(conn_io->conn)) {
        int deadline = 0;
        int priority = 0;
        int block_size = 0;
        int depend_id = 0;
        int stream_id = 0;
        float send_time_gap = 0.0;
        static uint8_t buf[MAX_BLOCK_SIZE];

        for (int i = conn_io->send_round; i < conn_io->configs_len; i++) {
            send_time_gap = conn_io->configs[i].send_time_gap;  // sec
            deadline = conn_io->configs[i].deadline;
            priority = conn_io->configs[i].priority;
            block_size = conn_io->configs[i].block_size;
            stream_id = 4 * (conn_io->send_round + 1) + 1;
            // if ((stream_id + 1) % 8 == 0)
            //     depend_id = stream_id - 4;
            // else
            // depend_id = stream_id;
            depend_id = stream_id;
            if (block_size > MAX_BLOCK_SIZE) block_size = MAX_BLOCK_SIZE;

            if (quiche_conn_stream_send_full(conn_io->conn, stream_id, buf,
                                             block_size, true, deadline,
                                             priority, depend_id) < 0) {
                // log_debug("failed to send data round %d",
                // conn_io->send_round);
            } else {
                // log_debug("send round %d", conn_io->send_round);
            }
            dump_file("%d,start,%ld\n", stream_id, getCurrentUsec() - started_at);

            conn_io->send_round++;
            if (conn_io->send_round >= conn_io->configs_len) {
                ev_timer_stop(loop, &conn_io->sender);
                break;
            }

            // if (send_time_gap > 0.005) {
            conn_io->sender.repeat = send_time_gap;
            ev_timer_again(loop, &conn_io->sender);
            // log_debug("time gap: %f", send_time_gap);
            break;  //每次只发一个block
            // } else {
            //     continue;  //如果间隔太小，则接着发
            // }
        }
    }
    flush_egress(loop, conn_io);
}

static struct conn_io *create_conn(struct ev_loop *loop, uint8_t *odcid,
                                   size_t odcid_len) {
    // log_debug("enter create_conn");
    struct conn_io *conn_io = malloc(sizeof(*conn_io));
    if (conn_io == NULL) {
        log_debug("failed to allocate connection IO");
        return NULL;
    }

    int rng = open("/dev/urandom", O_RDONLY);
    if (rng < 0) {
        log_error("failed to open /dev/urandom");
        return NULL;
    }

    ssize_t rand_len = read(rng, conn_io->cid, LOCAL_CONN_ID_LEN);
    if (rand_len < 0) {
        log_error("failed to create connection ID");
        return NULL;
    }

    quiche_conn *conn = quiche_accept(conn_io->cid, LOCAL_CONN_ID_LEN, odcid,
                                      odcid_len, config);
    if (conn == NULL) {
        log_debug("failed to create connection");
        return NULL;
    }

    conn_io->socks = conns->socks;
    conn_io->ai_family = conns->ai_family;
    conn_io->conn = conn;

    conn_io->send_round = -1;

    cfgs = parse_dtp_config(dtp_cfg_fname, &cfgs_len);
    if (cfgs_len <= 0) {
        log_debug("No valid configuration");
    } else {
        conn_io->configs_len = cfgs_len;
        conn_io->configs = cfgs;
    }
    conn_io->configs_len = cfgs_len;
    conn_io->configs = cfgs;

    conn_io->t_last = getCurrentUsec();
    conn_io->can_send = 1350;
    conn_io->done_writing = false;

    if(FEC_ENABLE) {
        quiche_conn_set_tail(conn, TAILSIZE);
    }

    ev_init(&conn_io->timer, timeout_cb);
    conn_io->timer.data = conn_io;

    ev_init(&conn_io->sender, sender_cb);
    conn_io->sender.data = conn_io;

    ev_init(&conn_io->pace_timer, flush_egress_pace);
    conn_io->pace_timer.data = conn_io;

    HASH_ADD(hh, conns->h, cid, LOCAL_CONN_ID_LEN, conn_io);

    log_debug("new connection,  timestamp: %lu",
              getCurrentUsec() / 1000 / 1000);

    return conn_io;
}

static void recv_cb(EV_P_ ev_io *w, int revents, int path) {
    // log_debug("enter recv");
    struct conn_io *tmp, *conn_io = NULL;

    static uint8_t buf[MAX_BLOCK_SIZE];
    static uint8_t out[MAX_DATAGRAM_SIZE];

    uint8_t i = 3;

    while (i--) {
        struct sockaddr_storage peer_addr;
        socklen_t peer_addr_len = sizeof(peer_addr);
        memset(&peer_addr, 0, peer_addr_len);

        ssize_t read = recvfrom(conns->socks[path], buf, sizeof(buf), 0,
                                (struct sockaddr *)&peer_addr, &peer_addr_len);
        in6_addr_set_byte(&((struct sockaddr_in6 *)&peer_addr)->sin6_addr, 8,
                          0);

        if (read < 0) {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                // log_debug("recv would block");
                break;
            }

            log_error("failed to read");
            return;
        }

        total_udp_bytes += read;

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
            log_debug("failed to parse header: %d", rc);
            return;
        }

        HASH_FIND(hh, conns->h, dcid, dcid_len, conn_io);

        if (conn_io == NULL) {
            if (!quiche_version_is_supported(version)) {
                log_debug("version negotiation");

                ssize_t written = quiche_negotiate_version(
                    scid, scid_len, dcid, dcid_len, out, sizeof(out));

                if (written < 0) {
                    // log_debug("failed to create vneg packet: %zd",
                    // written);
                    return;
                }

                int t = 5 << 5;
                int sid = MULIP_ENABLE ? 5 : 0;
                set_tos(conns->ai_family, conns->socks[sid], t);
                in6_addr_set_byte(
                    &((struct sockaddr_in6 *)&peer_addr)->sin6_addr, 8,
                    ip_cfg[sid]);

                ssize_t sent =
                    sendto(conns->socks[sid], out, written, 0,
                           (struct sockaddr *)&peer_addr, peer_addr_len);
                if (sent != written) {
                    // log_error("failed to send");
                    return;
                }

                // log_debug("sent %zd bytes", sent);
                return;
            }

            if (token_len == 0) {
                log_debug("stateless retry");

                mint_token(dcid, dcid_len, &peer_addr, peer_addr_len, token,
                           &token_len);

                ssize_t written =
                    quiche_retry(scid, scid_len, dcid, dcid_len, dcid, dcid_len,
                                 token, token_len, out, sizeof(out));

                if (written < 0) {
                    // log_debug("failed to create retry packet: %zd",
                    //         written);
                    return;
                }

                int t = 5 << 5;
                int sid = MULIP_ENABLE ? 5 : 0;
                set_tos(conns->ai_family, conns->socks[sid], t);
                in6_addr_set_byte(
                    &((struct sockaddr_in6 *)&peer_addr)->sin6_addr, 8,
                    ip_cfg[sid]);

                ssize_t sent =
                    sendto(conns->socks[sid], out, written, 0,
                           (struct sockaddr *)&peer_addr, peer_addr_len);
                if (sent != written) {
                    // log_error("failed to send");
                    return;
                }

                // log_debug("sent %zd bytes", sent);
                return;
            }

            if (!validate_token(token, token_len, &peer_addr, peer_addr_len,
                                odcid, &odcid_len)) {
                log_debug("invalid address validation token");
                return;
            }

            conn_io = create_conn(loop, odcid, odcid_len);
            if (conn_io == NULL) {
                return;
            }
            started_at = getCurrentUsec();
            dump_file("block_id,status,duration\n");

            memcpy(&conn_io->peer_addr, &peer_addr, peer_addr_len);
            conn_io->peer_addr_len = peer_addr_len;
        }

        ssize_t done = quiche_conn_recv(conn_io->conn, buf, read);

        if (done == QUICHE_ERR_DONE) {
            // log_debug("done reading");
            break;
        }

        if (done < 0) {
            log_debug("failed to process packet: %zd", done);
            return;
        }

        // log_debug("recv %zd bytes", done);

        if (quiche_conn_is_established(conn_io->conn)) {
            // begin send data: block trace
            // start sending first block immediately.
            if (conn_io->send_round == -1) {
                conn_io->send_round = 0;
                conn_io->sender.repeat = cfgs[0].send_time_gap > 0.0001
                                             ? cfgs[0].send_time_gap
                                             : 0.0001;
                ev_timer_again(loop, &conn_io->sender);
            }

            uint64_t s = 0;

            quiche_stream_iter *readable = quiche_conn_readable(conn_io->conn);

            while (quiche_stream_iter_next(readable, &s)) {
                // log_debug("stream %" PRIu64 " is readable", s);

                bool fin = false;
                ssize_t recv_len = quiche_conn_stream_recv(
                    conn_io->conn, s, buf, sizeof(buf), &fin);
                if (recv_len < 0) {
                    break;
                }
            }

            quiche_stream_iter_free(readable);
        }
    }

    HASH_ITER(hh, conns->h, conn_io, tmp) {
        flush_egress(loop, conn_io);

        if (quiche_conn_is_closed(conn_io->conn)) {
            quiche_stats stats;

            quiche_conn_stats(conn_io->conn, &stats);
            log_debug(
                "[RECVcb] connection closed, you can see result in client.log");
            // fprintf(stderr,
            //         "connection closed, recv=%zu sent=%zu lost=%zu rtt=%"
            //         PRIu64 "ns cwnd=%zu\n", stats.recv, stats.sent,
            //         stats.lost, stats.rtt, stats.cwnd);

            HASH_DELETE(hh, conns->h, conn_io);

            ev_timer_stop(loop, &conn_io->timer);
            ev_timer_stop(loop, &conn_io->sender);
            ev_timer_stop(loop, &conn_io->pace_timer);
            quiche_conn_free(conn_io->conn);
            free(conn_io);
        }
    }
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

static void timeout_cb(EV_P_ ev_timer *w, int revents) {
    // log_debug("enter timeout");
    struct conn_io *conn_io = w->data;
    quiche_conn_on_timeout(conn_io->conn);

    // log_debug("timeout");

    flush_egress(loop, conn_io);

    if (quiche_conn_is_closed(conn_io->conn)) {
        quiche_stats stats;

        quiche_conn_stats(conn_io->conn, &stats);
        // fprintf(stderr,
        //         "connection closed, recv=%zu sent=%zu lost=%zu rtt=%" PRIu64
        //         "ns cwnd=%zu\n",
        //         stats.recv, stats.sent, stats.lost, stats.rtt, stats.cwnd);
        log_info("[TOcb] connection closed, you can see result in client.log");

        // fflush(stdout);

        HASH_DELETE(hh, conns->h, conn_io);

        ev_timer_stop(loop, &conn_io->timer);
        ev_timer_stop(loop, &conn_io->sender);
        ev_timer_stop(loop, &conn_io->pace_timer);
        quiche_conn_free(conn_io->conn);
        free(conn_io);

        return;
    }
}

static void dump_quiche_log(const char *line, void *argp)
{
    dump_file("%s,%ld\n", line, getCurrentUsec() - started_at);
    // log_debug("%s,%ld", line, getCurrentUsec() - started_at);
}

int main(int argc, char *argv[]) {
    args.out = stdout;
    args.log = stderr;
    argp_parse(&argp, argc, argv, 0, 0, &args);
    log_info("SERVER_IP %s SERVER_PORT %s DTP_CONFIG %s\n", args.args[0],
             args.args[1], args.args[2]);
    dtp_cfg_fname = args.args[2];

    const struct addrinfo hints = {.ai_family = PF_UNSPEC,
                                   .ai_socktype = SOCK_DGRAM,
                                   .ai_protocol = IPPROTO_UDP};

    quiche_enable_debug_logging(dump_quiche_log, NULL);

    struct addrinfo *local;
    if (getaddrinfo(args.args[0], args.args[1], &hints, &local) != 0) {
        log_error("failed to resolve host");
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
                log_error("failed to bind socket");
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
    }

    config = quiche_config_new(QUICHE_PROTOCOL_VERSION);
    if (config == NULL) {
        // log_debug("failed to create config");
        return -1;
    }

    quiche_config_load_cert_chain_from_pem_file(config, "cert.crt");
    quiche_config_load_priv_key_from_pem_file(config, "cert.key");

    quiche_config_set_application_protos(
        config, (uint8_t *)"\x05hq-25\x05hq-24\x05hq-23\x08http/0.9", 21);

    quiche_config_set_max_idle_timeout(config, 5000);
    quiche_config_set_max_packet_size(config, MAX_DATAGRAM_SIZE);
    quiche_config_set_initial_max_data(config, 10000000000);
    quiche_config_set_initial_max_stream_data_bidi_local(config, 1000000000);
    quiche_config_set_initial_max_stream_data_bidi_remote(config, 1000000000);
    quiche_config_set_initial_max_streams_bidi(config, 40000);
    quiche_config_set_cc_algorithm(config, CC);
    quiche_config_set_scheduler_name(config, SCHEDULER);
    if(FEC_ENABLE) {
        quiche_config_set_redundancy_rate(config, RATE);
    } else {
        quiche_config_set_redundancy_rate(config, 0);
    }
    quiche_set_debug_logging_level(DEBUG_LEVEL);
    // quiche_config_set_ntp_server(config, "192.168.0.1");

    struct connections c;
    c.socks = socks;
    c.ai_family = local->ai_family;
    c.h = NULL;

    conns = &c;

    struct ev_loop *loop = ev_default_loop(0);

    if (MULIP_ENABLE) {
        ev_io watcher[8];
        ev_io_init(&watcher[0], on_recv_0, c.socks[0], EV_READ);
        ev_io_start(loop, &watcher[0]);
        watcher[0].data = &c;

        ev_io_init(&watcher[1], on_recv_1, c.socks[1], EV_READ);
        ev_io_start(loop, &watcher[1]);
        watcher[1].data = &c;

        ev_io_init(&watcher[2], on_recv_2, c.socks[2], EV_READ);
        ev_io_start(loop, &watcher[2]);
        watcher[2].data = &c;

        ev_io_init(&watcher[3], on_recv_3, c.socks[3], EV_READ);
        ev_io_start(loop, &watcher[3]);
        watcher[3].data = &c;

        ev_io_init(&watcher[4], on_recv_4, c.socks[4], EV_READ);
        ev_io_start(loop, &watcher[4]);
        watcher[4].data = &c;

        ev_io_init(&watcher[5], on_recv_5, c.socks[5], EV_READ);
        ev_io_start(loop, &watcher[5]);
        watcher[5].data = &c;

        ev_io_init(&watcher[6], on_recv_6, c.socks[6], EV_READ);
        ev_io_start(loop, &watcher[6]);
        watcher[6].data = &c;

        ev_io_init(&watcher[7], on_recv_7, c.socks[7], EV_READ);
        ev_io_start(loop, &watcher[7]);
        watcher[7].data = &c;
    } else {
        ev_io watcher;
        ev_io_init(&watcher, on_recv_0, socks[0], EV_READ);
        ev_io_start(loop, &watcher);
        watcher.data = &c;
    }

    ev_loop(loop, 0);

    freeaddrinfo(local);

    quiche_config_free(config);

    return 0;
}