# coding=utf-8
#最基础的柱状图
import random
import numpy as np
import sys
import csv
import tool_filter as ft;
import matplotlib
import matplotlib.pyplot as plt
import os
import matplotlib.gridspec as gridspec

def drawPerState(ax,dataE,dataP,dataI,labels,titleName):
    # 空在最上面
    # P 在中间
    # I 在最下面

    ax.bar(np.arange(0,600,6), dataI, align='center',width=[4]*len(dataE), alpha=0.7, color='lightcoral',label='I_Frame')
    ax.bar(np.arange(0,600,6), dataP, bottom=dataI, align='center',width=[4]*len(dataE), alpha=0.7, color='springgreen',label='P_Frame')
    ax.bar(np.arange(0,600,6), dataE, bottom=dataP, align='center',width=[4]*len(dataE), alpha=0.7, color='gray', tick_label=labels,label='Empty_Frame')
    ax.set_xlim(-8, 606)  # 限定横轴的范围
    ax.set_ylim(0,20000)
    ax.set_xticks(np.arange(0,600,6))
    ax.set_yticks(np.arange(0,26000,2000))
    ax.grid(linestyle="--")  # 设置背景网格线为虚线
    # l1 ,= ax.plot(frams,data1)
    # ax.legend([l1],['I_Frame'],loc='upper right')
    # ax.legend(loc='upper right', frameon=False)
    ax.legend()
    ax.set_title(titleName)
    # ax.bar(range(len(sale8)),sale8,tick_label=labels,label="8月")

    # # 九月的bottom是sale8，也就是八月，所以九月在上边
    # ax.bar(range(len(sale9)),sale9,bottom=sale8,tick_label=labels,label="9月")
    # ax.legend()

    # 原来=====================

    # ax.bar(np.arange(0,600,6), data, align='center',width=[4]*len(data), alpha=0.7, color=colors, tick_label=labels)
    # #控制y轴的刻度
    # ax.set_xlim(-8, 606)  # 限定横轴的范围
    # ax.set_ylim(0,20000)
    # ax.set_xticks(np.arange(0,600,6))
    # ax.set_yticks(np.arange(0,20000,2000))
    # ax.grid(linestyle="--")  # 设置背景网格线为虚线
    # # l1 ,= ax.plot(frams,data1)
    # # ax.legend([l1],['I_Frame'],loc='upper right')
    # # ax.legend(loc='upper right', frameon=False)

    # ax.set_title(titleName)

    # 原来=====================
    

def drawStatesAll(start,end,Net_Consume_Path,pJitter_Pop_Path,saveName):
    fig = plt.figure()
    gs1 = gridspec.GridSpec(2, 1)
    ax1 = fig.add_subplot(gs1[0])
    ax2 = fig.add_subplot(gs1[1])
    colors = list()
    labels = list()
    dataI = list()
    dataP = list()
    dataE = list()
    # 红色I'lightcoral' 绿色P 'springgreen'
    # LogType,Which,AlgoTime,TimeType,Detail
    # Net_Consume,I_frame,1615216530375,819,0

    # 推流端
    Net_Consume_data = ft.getcsv2List(Net_Consume_Path)
    print("推流端=========")
    print("state start:"+str(start))
    print("state end:"+str(end))
    for i in range(start,end):
        print(i)
        if Net_Consume_data[i][1] == "I_frame":
            # colors.append('lightcoral')
            labels.append('I')
            dataI.append(int(Net_Consume_data[i][3]))
            dataP.append(int(0))
            dataE.append(int(0))
        else:
            # colors.append('springgreen')
            labels.append('P')
            dataI.append(int(0))
            dataP.append(int(Net_Consume_data[i][3]))
            dataE.append(int(0))

        # data.append(int(Net_Consume_data[i][3]))

    print("dataI:"+str(len(dataI)))
    print("dataP:"+str(len(dataP)))
    print("dataE:"+str(len(dataE)))
    # print("colors:"+str(len(colors)))
    print("labels:"+str(len(labels)))
    print("播放端=========")
    print("state start:"+str(start))
    print("state end:"+str(end))
    drawPerState(ax1,dataE,dataP,dataI,labels,'server_'+str(start)+'~'+str(end))
    # 播放端
    pJitter_Pop_Path = ft.filteFrame(pJitter_Pop_Path)
    colors = list()
    labels = list()
    dataI = list()
    dataP = list()
    dataE = list()
    for i in range(start,end):
        if Net_Consume_data[i][1] == "I_frame":
            labels.append('I')
            if str(Net_Consume_data[i][4]) not in pJitter_Pop_Path:
                colors.append('gray')
                dataI.append(int(0))
                dataP.append(int(0))
                dataE.append(int(Net_Consume_data[i][3]))
            else:
                colors.append('lightcoral')
                dataI.append(int(Net_Consume_data[i][3]))
                dataP.append(int(0))
                dataE.append(int(0))
            
        else:
            labels.append('P')
            if str(Net_Consume_data[i][4]) not in pJitter_Pop_Path:
                colors.append('gray')
                dataI.append(int(0))
                dataP.append(int(0))
                dataE.append(int(Net_Consume_data[i][3]))
            else:
                colors.append('springgreen')
                dataI.append(int(0))
                dataP.append(int(Net_Consume_data[i][3]))
                dataE.append(int(0))

        # data.append(int(Net_Consume_data[i][3]))
    print("dataI:"+str(len(dataI)))
    print("dataP:"+str(len(dataP)))
    print("dataE:"+str(len(dataE)))
    # print("colors:"+str(len(colors)))
    print("labels:"+str(len(labels)))
    drawPerState(ax2,dataE,dataP,dataI,labels,'player_'+str(start)+'~'+str(end))
    fig.set_size_inches(18,10)
    plt.tight_layout()
    plt.savefig(saveName, format='svg')

if __name__ == '__main__':

    argv = sys.argv
    argc = len(argv)

    if argc > 2:
        root = argv[1]
        save = argv[2]
    else:
        print("please input root path of logs and csv name.")
        os._exit(0)

    print("root path: " + root)
    print("save name: " + save)
    print("----------\n")

    # data_Net_Consume = ft.getcsv2List("./data/data_Net_Consume.csv")
    # data_pJitter_Pop = ft.getcsv2List("./data/data_pJitter_Pop.csv")
    drawStatesAll(100,200,'./data/data_Net_Consume.csv','./data/data_pJitter_Pop.csv',save+'data_frames_states.svg')

    print("--done!--")