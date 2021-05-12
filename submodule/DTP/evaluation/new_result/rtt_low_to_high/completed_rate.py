#!/usr/bin/python3
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick

lab_times=1 #重复实验次数
step=20 #rtt间隔
rtt_num=10 #假设测5种rtt
deadline=200 #deadline 200ms
block_num=300 #总共发包量
def completion_rate(f):
    sum=0
    f.readline()# 第一行：StreamID goodbytes bct BlockSize Priority Deadline
    for line in f.readlines():
        arr=line.split()
        t=arr[2]
        id=arr[0]
        if int(id)>(1000-300)*4 and float(t)<200:
            sum+=1
    return sum/block_num
def get_y(case,y):
    for i in range(1,rtt_num+1):
        # 2020.2.2：每种bandwidth跑了5次，5次实验的实验结果，分别在result1~5/data/中，算完后取平均值
        average=0
        for j in range(lab_times):
            fpath='result'+str(j+1)+'/data/'+case+'/'+case+'-rtt'+'-'+str(20*i-20)+'ms.log'
            f=open(fpath,'r')
            average+=completion_rate(f)
        y[i-1]=average/lab_times
    return y

x = np.arange(1,rtt_num+1)
y = np.arange(1.0, rtt_num+1)
for i in range(rtt_num):
    x[i]=i*step
fig, ax = plt.subplots(dpi=200)#figsize=(16, 12)
case=['DTP','QUIC','Deadline','Priority']
line_style = ['-','--','-.',':']
for i in range(len(case)):
    get_y(case[i],y)
    y=y*100 # 为了之后百分比显示
    ax.plot(x,y,label=case[i],linestyle=line_style[i])
# 设置百分比显示
fmt = '%.0f%%' # Format you want the ticks, e.g. '40%'
yticks = mtick.FormatStrFormatter(fmt)
ax.yaxis.set_major_formatter(yticks)
# ax.set_title('rtt low->high',size=24)
ax.set_xlabel('rtt/ms',size='xx-large')
ax.set_ylabel('completion rate before deadline',size='xx-large')
ax.legend(fontsize='xx-large')
ax.set_xticks(x)
plt.xticks(size='xx-large')
plt.yticks(size='xx-large')
plt.grid()  # 生成网格
plt.savefig('completed_rate.png',bbox_inches='tight') # 核心是设置成tight

