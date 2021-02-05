#include <s_dtp_server.h>
#include <util_log.h>
#include <unistd.h> // 一段时间后中断程序alarm用
int main(int argc, char *argv[]) {
    // timeMain.start();
    timeMainServer.startAndWrite("server");
    // timeMainServer.evalTime("s","mainStart");
    //程序执行5s后自动关闭,方便调试,不用在dplay设置时间
    // alarm(5);
    const char *host = argv[1];
    const char *port = argv[2];
    const char *conf = NULL;
    
    if (argc >= 4) {
        conf = argv[3];
    }
    else {
        conf = "./config/stream.conf";
    }
    Print2FileInfo("(s)server程序入口");
    dtp_server(host, port, conf);
}