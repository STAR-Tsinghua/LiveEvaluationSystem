#include <s_quiche_server.h>

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

    quiche_server(host, port, conf);
}