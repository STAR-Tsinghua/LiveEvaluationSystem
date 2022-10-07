#include <arpa/inet.h>
#include "../../submodule/DTP/include/quiche.h"
#include "dtp-argp.hpp"
#include "helper.h"

void in6_addr_set_byte(struct in6_addr *a, uint8_t i, uint8_t d)
{
    a->s6_addr[i] = d;
}

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
            log_error("tos ai_family cannot recognize");
            break;
    }
}

/**
 * @brief Set the sock tos and ip with blocks from dtp send api
 * 
 * @param stream_blocks: DTP blocks from send api
 * @param stream_blocks_num: number of blocks
 * @param ai_family
 * @param socks 
 * @return int: the socket id to send, -1 if any error
 */
int set_sock_tos_and_ip_with_blocks(struct Block* stream_blocks, int stream_blocks_num,
    int ai_family, int* socks
) {
    int t = 0;
    int sid = 0;
    if(!TOS_ENABLE) return sid;
    if (stream_blocks_num > 0) {
        sid = tos(stream_blocks[0].block_deadline,
                  stream_blocks[0].block_priority);
        t = sid << 5;
        sid = MULIP_ENABLE ? sid : 0;
        set_tos(ai_family, socks[sid], t);
        // TODO: in6_addr_set_byte(
        //        &((struct sockaddr_in6 *)&conn_io->peer_addr)->sin6_addr, 8,
        //        ip_cfg[sid]);
        return sid;
    } else {
        return 0;
    }
}
// 修改 DTP ，使之可以读取发送的块的队列
