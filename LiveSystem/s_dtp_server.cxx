#include "s_dtp_server.h"
#include <util/util_log.h>
#include <unistd.h> // 一段时间后中断程序alarm用
int main(int argc, char *argv[]) {
    //程序执行5s后自动关闭,方便调试,不用在dplay设置时间
    // alarm(5);
    argp_parse(&argp, argc, argv, 0, 0, &args);
    printf("SERVER_IP %s SERVER_PORT %s CONFIG_FILE %s\n", args.args[0],
             args.args[1], args.args[2]);
    const char *host = args.args[0];
    const char *port = args.args[1];
    const char *conf = args.args[2];
    
    if (conf) {
        conf = argv[3];
    }else {
        conf = "./config/dtp.conf";
    }
    logSysPrepare(conf);
    timeMainServer.startAndWrite("server");
    timeFrameServer.startAndWrite("server");
    Print2FileInfo("(s)server程序入口");
    dtp_server(host, port, conf);
}