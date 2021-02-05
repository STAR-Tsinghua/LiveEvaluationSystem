#include <s_udp_server.h>

int main(int argc, char *argv[]) {
    const char *host = argv[1];
    const char *port = argv[2];

    const char *conf = NULL;
    if (argc >= 4) {
        conf = argv[3];
    }
    else {
        conf = "./config/stream.conf";
    }

    udp_server(host, port, conf);
}