sudo throttle --up 330 --down 780 --rtt 100
sudo throttle --up 500000 --down 500000 --rtt 10

//4G 4G 网络的下行速度bai始终可以稳定在50Mbps左右，上行速度则达到8Mbps以上
sudo throttle --up 8000 --down 50000 --rtt 25 

//设置显卡
sudo pmset -a GPUSwitch 1

0，强制使用核显，
1， 强制使用独显，
2 ，自动切换图形卡

CAP_PROP_FPS

sudo throttle --up 1000 --down 1000 --rtt 125
sudo throttle --up 800 --down 800 --rtt 50
sudo throttle --up 500 --down 500 --rtt 50