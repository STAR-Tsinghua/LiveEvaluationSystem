ps -ef|grep r_dtp_play |grep -v grep|awk '{print "kill -9 "$2}'|sh
ps -ef|grep r_dtp_server |grep -v grep|awk '{print "kill -9 "$2}'|sh