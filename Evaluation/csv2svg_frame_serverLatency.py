# coding=utf-8
import sys
import csv
import matplotlib.pyplot as plt
import tool_filter as ft;
from matplotlib.pyplot import figure
import numpy as np
import matplotlib.gridspec as gridspec
import more_itertools

def example_plot_Deltime(ax,CatchFrame_To_RGB_Push,RGB_Push_Data_To_RGB_Pop,RGB_Pop_To_FrameToYUV,FrameToYUV_To_Net_Produce,Net_Produce_To_Net_Consume,title, fontsize=12):
    data1 = list()
    data2 = list()
    data3 = list()
    data4 = list()
    data5 = list()
    dataTotal = list()
    frams = list()
    num = 0
    print("len(CatchFrame_To_RGB_Push):"+str(len(CatchFrame_To_RGB_Push)))
    print("len(RGB_Push_Data_To_RGB_Pop):"+str(len(RGB_Push_Data_To_RGB_Pop)))
    print("len(RGB_Pop_To_FrameToYUV):"+str(len(RGB_Pop_To_FrameToYUV)))
    print("len(FrameToYUV_To_Net_Produce):"+str(len(FrameToYUV_To_Net_Produce)))
    print("len(Net_Produce_To_Net_Consume):"+str(len(Net_Produce_To_Net_Consume)))
    for i in range(30,600):  # 从第二行开始读取
        # print("len(dataDeltime1):"+dataDeltime1[i][2])
        tmp1 = float(CatchFrame_To_RGB_Push[i][2])
        tmp2 = float(RGB_Push_Data_To_RGB_Pop[i][2])
        tmp3 = float(RGB_Pop_To_FrameToYUV[i][2])
        tmp4 = float(FrameToYUV_To_Net_Produce[i][2])
        tmp5 = float(Net_Produce_To_Net_Consume[i][2])
        data1.append(tmp1)
        data2.append(tmp2)
        data3.append(tmp3)
        data4.append(tmp4)
        data5.append(tmp5)
        dataTotal.append(tmp1+tmp2+tmp3+tmp4+tmp5)
        num = num + 1
        frams.append(num)

    l1 ,= ax.plot(frams,data1)
    l2 ,= ax.plot(frams,data2)
    l3 ,= ax.plot(frams,data3)
    l4 ,= ax.plot(frams,data4)
    l5 ,= ax.plot(frams,data5)
    l6 ,= ax.plot(frams,dataTotal)
    # 绘制中位数值线
    ax.hlines(np.median(dataTotal), frams[0], frams[-1:],
          linestyles='-.', colors='#dc5034')

    # ax.hlines(np.median(data2), frams[0], frams[-1:],
    #       linestyles='-.', colors='#dc5034')

    # ax.hlines(np.median(dataDeltime), frams[0], frams[-1:],
    #     linestyles='-.', colors='#dc5034')

    # 标注平均值
    # ax.text(frams[-1:], float(np.median(data1)-2),
    #         '中位数值线: ',
    #         color='#dc5034', fontsize=15)

    ax.locator_params(nbins=3)
    ax.set_xlabel('blocks(num)', fontsize=fontsize)
    ax.set_ylabel('time(ms)', fontsize=fontsize)
    ax.set_xlim(0)
    ax.set_ylim(0)
    ax.legend([l1,l2,l3,l4,l5,l6],['CatchFrame_To_RGB_Push','RGB_Push_To_RGB_Pop','RGB_Pop_To_FrameToYUV','FrameToYUV_To_Net_Produce','Net_Produce_To_Net_Consume','Total'],loc='upper right')
    ax.grid(linestyle="--")  # 设置背景网格线为虚线
    ax.set_xticks(np.arange(0,max(frams),25))
    ax.set_yticks(np.arange(0,max(dataTotal),25))
    # ax.legend((rect,),("time ms",))
    ax.set_title(title, fontsize=fontsize)

def draw_buffer_container(CatchFrame_To_RGB_Push,RGB_Push_Data_To_RGB_Pop,RGB_Pop_To_FrameToYUV,FrameToYUV_To_Net_Produce,Net_Produce_To_Net_Consume,saveName):
    # fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(nrows=2, ncols=2)
    fig = plt.figure()
    gs1 = gridspec.GridSpec(1, 1)
    ax1 = fig.add_subplot(gs1[0])
    example_plot_Deltime(ax1,CatchFrame_To_RGB_Push,RGB_Push_Data_To_RGB_Pop,RGB_Pop_To_FrameToYUV,FrameToYUV_To_Net_Produce,Net_Produce_To_Net_Consume,"Buffer_Deltime")
    fig.set_size_inches(18,10)
    plt.tight_layout()
    # fig.set_title("frames_jitter", fontsize=20)
    #设置坐标轴刻度
    # my_x_ticks = np.arange(0,240,10)      #显示范围为-5至5，每0.5显示一刻度
    # my_y_ticks = np.arange(5,60,5)      #显示范围为-2至2，每0.2显示一刻度
    # fig.xticks(my_x_ticks)
    # fig.yticks(my_y_ticks)

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

    global colorG
    colorG = ['mediumblue']
    # 获取每个节点时间戳
    CatchFrame_Data = ft.filteByLogType(root,"start_CatchFrame")
    RGB_Push_Data = ft.filteByLogType(root,"RGB_Push")
    RGB_Pop_Data = ft.filteByLogType(root,"RGB_Pop")
    FrameToYUV_Data = ft.filteByLogType(root,"FrameToYUV")
    Net_Produce_Data = ft.filteByLogType(root,"Net_Produce")
    Net_Consume_Data = ft.filteByLogType(root,"Net_Consume")

    # Net_Consume到数据完全发送出去还有一段时间别忘了！！！

    # CatchFrame_Data->RGB_Push_Data
    CatchFrame_To_RGB_Push = ft.calculate2DeltimeList(CatchFrame_Data,RGB_Push_Data)
    # RGB_Push_Data->RGB_Pop_Data
    RGB_Push_Data_To_RGB_Pop = ft.calculate2DeltimeList(RGB_Push_Data,RGB_Pop_Data)
    # RGB_Pop_Data->FrameToYUV_Data
    RGB_Pop_To_FrameToYUV = ft.calculate2DeltimeList(RGB_Pop_Data,FrameToYUV_Data)
    # FrameToYUV_Data->Net_Produce_Data
    FrameToYUV_To_Net_Produce = ft.calculate2DeltimeList(FrameToYUV_Data,Net_Produce_Data)
    # Net_Produce_Data->Net_Consume_Data
    Net_Produce_To_Net_Consume = ft.calculate2DeltimeList(Net_Produce_Data,Net_Consume_Data)

    draw_buffer_container(CatchFrame_To_RGB_Push,RGB_Push_Data_To_RGB_Pop,RGB_Pop_To_FrameToYUV,FrameToYUV_To_Net_Produce,Net_Produce_To_Net_Consume,save+'data_frame_serverLatency.svg')
    print("--done!--")