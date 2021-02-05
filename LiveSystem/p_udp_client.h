#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

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
#include <p_sodtp_jitter.h>
#include <sodtp_block.h>


class CONN_IO {
public:
    ev_timer timer;

    int sock;

    uint32_t recv_round;
    JitterBuffer *jitter;
};

static void flush_egress(struct ev_loop *loop, CONN_IO *conn_io) {

    ssize_t sent = send(conn_io->sock, "hello", 6, 0);
    if (sent != 6) {
        perror("failed to send");
        return;
    }

    fprintf(stderr, "sent %zd bytes\n", sent);

    // timeout = 5.0s
    conn_io->timer.repeat = 5.0;
    ev_timer_again(loop, &conn_io->timer);
}


static void recv_cb(EV_P_ ev_io *w, int revents) {

    CONN_IO *conn_io = (CONN_IO *)w->data;

    static uint8_t buf[65535];
    static SodtpStreamHeader header;
    static BlockDataBuffer bk_buf;
    static int sid = 0;
    int32_t size;
    uint8_t *data;

    while (1) {
        ssize_t read = recv(conn_io->sock, buf, sizeof(buf), 0);

        if (read < 0) {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                fprintf(stderr, "recv would block\n");
                break;
            }

            perror("failed to read");
            return;
        }

        fprintf(stderr, "udp recv len = %d\n", (int)read);

        ///
        ///
        /// Read the frames.
        ///
        ///
        if(bk_buf.write(sid, buf, read) != read) {
            fprintf(stderr, "fail to write data to buffer.\n");
        }

        if (true) {
            SodtpBlockPtr bk_ptr = bk_buf.read(sid, &header);
            if (bk_ptr) {
                fprintf(stderr, "block ts %lld\n", header.block_ts);
                fprintf(stderr, "recv round %d,\t stream %d,\t block %d,\t size %d,\t delay %d\n",
                    conn_io->recv_round, header.stream_id,
                    bk_ptr->block_id, bk_ptr->size,
                    (int)(current_mtime() - header.block_ts));
                conn_io->jitter->push_back(&header, bk_ptr);

                conn_io->recv_round++;
                fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
            }
        }
        sid += 1;

        // send back an ack in application layer.
        flush_egress(loop, conn_io);

        // timeout = 5.0s
        conn_io->timer.repeat = 5.0;
        ev_timer_again(loop, &conn_io->timer);
    }
}


static void timeout_cb(EV_P_ ev_timer *w, int revents) {
    CONN_IO *conn_io = (CONN_IO *)w->data;
    fprintf(stderr, "timeout\n");

    delete conn_io;
    ev_break(EV_A_ EVBREAK_ONE);
}

int udp_client(const char *host, const char *port, JitterBuffer *pJBuffer) {
    struct addrinfo hints;

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;


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


    CONN_IO *conn_io = new CONN_IO();
    if (conn_io == NULL) {
        fprintf(stderr, "failed to allocate connection IO\n");
        return -1;
    }
    conn_io->sock = sock;
    conn_io->recv_round = 1;
    conn_io->jitter = pJBuffer;

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


    struct ev_loop *loop = ev_loop_new(EVFLAG_AUTO);

    ev_io watcher;
    ev_io_init(&watcher, recv_cb, conn_io->sock, EV_READ);
    ev_io_start(loop, &watcher);
    watcher.data = conn_io;

    ev_init(&conn_io->timer, timeout_cb);
    conn_io->timer.data = conn_io;

    flush_egress(loop, conn_io);

    int time1 = current_mtime();
    ev_loop(loop, 0);
    fprintf(stderr, "client interval: %ds\n", ((int)current_mtime()-time1)/1000);

    freeaddrinfo(peer);


    // Notification for the buffer clearing.
    // We prefer not to explicitly notify the close of connection.
    // When jitter buffer is empty, the thread will stop automatically.
    // But this is for the corner case, e.g. the breaking of connection.
    sem_post(pJBuffer->sem);
    ev_feed_signal(SIGUSR1);


    return 0;
}


#endif // UDP_CLIENT_H