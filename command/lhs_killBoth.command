ps -ef|grep dplay |grep -v grep|awk '{print "kill -9 "$2}'|sh
ps -ef|grep dtp_server |grep -v grep|awk '{print "kill -9 "$2}'|sh
killall Terminal