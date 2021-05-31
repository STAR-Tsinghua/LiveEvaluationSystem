ps -ef|grep p_play |grep -v grep|awk '{print "kill -9 "$2}'|sh
ps -ef|grep p_server |grep -v grep|awk '{print "kill -9 "$2}'|sh
