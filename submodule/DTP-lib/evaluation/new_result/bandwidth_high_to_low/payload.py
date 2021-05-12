#!/usr/bin/python3
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick

lab_times = 1  # 重复实验次数
deadline = 200  # deadline 200ms
block_num = 300  # 总共发包量
cases = ['QUIC', 'DTP',  'Deadline', 'Priority']

def sum_good_bytes(f):
    sum=0
    f.readline()# 第一行：StreamID goodbytes bct BlockSize Priority Deadline
    for line in f.readlines():
        arr=line.split()
        # 文件格式：每一行第一个参数为ID，参数二：deadline之前到的大小，
        # 参数三：为complete time，参数四：BlockSize；
        # 参数五：priority
        good_bytes=arr[1]
        sum+=int(good_bytes)
    return sum

def get_good_bytes(case,bw):
    # 2020.2.2：每种bandwidth跑了5次，5次实验的实验结果，分别在result1~5/data/中，算完后取平均值
    average=0
    for j in range(lab_times):
        fpath='result'+str(j+1)+'/data/'+case+'/'+case+'-bandwidth'+'-'+str(bw)+'M.log'
        f=open(fpath,'r')
        average+=sum_good_bytes(f)
    return average/lab_times

def payload_send(f):
    sended = 0
    for line in f.readlines():
        if line.find('payload send') != -1:
            s_arr = line.split()
            buf_len = int(s_arr[3])
            # print(line)
            sended += buf_len
    return sended


def get_payload_send(case, bw):
    # 2020.2.2：每种bandwidth跑了5次，5次实验的实验结果，分别在result1~5/data/中，算完后取平均值
    average = 0
    for j in range(lab_times):
        # result1/client_log/Deadline-client-8M.txt
        fpath = 'result'+str(j+1)+'/client_log/'+case + \
            '-client'+'-'+str(bw)+'M.log'
        f = open(fpath, 'r')
        average += payload_send(f)
    return average/lab_times


if __name__ == "__main__":
    # 选定三种带宽画图，共4*3=12根柱子
    # bw = [8, 4, 2]
    # x = bw

    x = np.arange(1, len(cases)+1)
    y_bad = np.arange(1.0, len(cases)+1)
    y_good = np.arange(1.0, len(cases)+1)
    for i in range(len(cases)):
        y_good[i]=get_good_bytes(cases[i],4)
        y_bad[i]=get_payload_send(cases[i], 4)-y_good[i]
    print(y_bad)
    print(y_good)


    fig, ax = plt.subplots(dpi=200)  # figsize=(16, 12)
    plt.bar(x, y_good, width=0.5, color="#66c2a5", tick_label=cases, label="goodbytes")
    plt.bar(x, y_bad, width=0.5,bottom=y_good, color="#F41F5A", label="badbytes")
    # ax.set_xlabel('Case', size='xx-large')
    ax.set_ylabel('payload send size(Bytes)', size='xx-large')
    ax.legend(fontsize='xx-large')
    plt.xticks(size='xx-large')
    plt.yticks(size='xx-large')
    plt.savefig('payload-4Mbps.png', bbox_inches='tight')  # 核心是设置成tight
