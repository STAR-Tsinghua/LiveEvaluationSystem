# coding=utf-8
import sys
import csv
import matplotlib.pyplot as plt
import tool_filter as ft;
from matplotlib.pyplot import figure
import numpy as np
import matplotlib.gridspec as gridspec
import more_itertools
# solve for a and b
def best_fit(X, Y):

    xbar = sum(X)/len(X)
    ybar = sum(Y)/len(Y)
    n = len(X) # or len(Y)

    numer = sum([xi*yi for xi,yi in zip(X, Y)]) - n * xbar * ybar
    denum = sum([xi**2 for xi in X]) - n * xbar**2

    b = numer / denum
    a = ybar - b * xbar

    print('best fit line:\
y = {:.2f} + {:.2f}x'.format(a, b))

    return a, b

def example_plot_Count(ax,dataCount1,dataCount2,title, fontsize=12):
    data1 = list()
    data2 = list()
    data_1_2 = list()
    frams = list()
    num = 0
    print("len(RGB_Buffer_Count):"+str(len(dataCount1)))
    print("len(Net_Buffer_Count):"+str(len(dataCount2)))
    for i in range(30,600):  # 从第二行开始读取
        tmp1 = float(dataCount1[i][4])
        tmp2 = float(dataCount2[i][4])
        data1.append(tmp1)
        data2.append(tmp2)
        data_1_2.append(tmp1+tmp2)
        num = num + 1
        frams.append(num)

    data1_mean = list(more_itertools.windowed(data1,n=2, step=1))
    data2_mean = list(more_itertools.windowed(data2,n=2, step=1))
    data_1_2_mean = list(more_itertools.windowed(data_1_2,n=2, step=1))
    dataMeanClear_1 = list()
    dataMeanClear_2 = list()
    dataMeanClear_1_2 = list()
    for n in data1_mean:
        dataMeanClear_1.append(np.mean(n))
    
    for n in data2_mean:
        dataMeanClear_2.append(np.mean(n))

    for n in data_1_2_mean:
        dataMeanClear_1_2.append(np.mean(n))
    # data4 = [sum(x) / len(x) for x in chunked(data2, 2)]
    l1 ,= ax.plot(frams[:-1],dataMeanClear_1,color='blue')
    l2 ,= ax.plot(frams[:-1],dataMeanClear_2,color='red')
    l3 ,= ax.plot(frams[:-1],dataMeanClear_1_2,color='green')

    ax.plot(frams,data1,linestyle=":",color='skyblue')
    ax.plot(frams,data2,linestyle=":",color='lightcoral')

    ax.legend([l3,l2,l1],['Sum_Net_RGB_Count','Net_Buffer_Count','RGB_Buffer_Count'],loc='upper right')
    # 绘制平均值线
    # ax.hlines(np.mean(data), frams[0], frams[-1:],
    #       linestyles='-.', colors='#dc5034')

    ax.locator_params(nbins=3)
    ax.set_xlabel('blocks(num)', fontsize=fontsize)
    ax.set_ylabel('number(num)', fontsize=fontsize)
    ax.set_xlim(0)
    ax.set_ylim(0)
    ax.grid(linestyle="--")  # 设置背景网格线为虚线
    # ax.set_xticks(np.arange(0,240,10))
    ax.set_title(title, fontsize=fontsize)

def example_plot_Deltime(ax,dataDeltime1,dataDeltime2,title, fontsize=12):
    data1 = list()
    data2 = list()
    dataDeltime = list()
    frams = list()
    num = 0
    print("len(FrameRGB_Deltime_Data):"+str(len(dataDeltime1)))
    print("len(FrameNet_Deltime_Data):"+str(len(dataDeltime2)))
    for i in range(30,600):  # 从第二行开始读取
        data1.append(float(dataDeltime1[i][2]))
        # print("len(dataDeltime1):"+dataDeltime1[i][2])
        data2.append(float(dataDeltime2[i][2]))
        dataDeltime.append(float(dataDeltime1[i][2])+float(dataDeltime2[i][2]))
        num = num + 1
        frams.append(num)

    l1 ,= ax.plot(frams,data1)
    l2 ,= ax.plot(frams,data2)
    l3 ,= ax.plot(frams,dataDeltime)
    # 绘制中位数值线
    ax.hlines(np.median(data1), frams[0], frams[-1:],
          linestyles='-.', colors='#dc5034')

    ax.hlines(np.median(data2), frams[0], frams[-1:],
          linestyles='-.', colors='#dc5034')

    ax.hlines(np.median(dataDeltime), frams[0], frams[-1:],
        linestyles='-.', colors='#dc5034')
    # 标注平均值
    # ax.text(frams[-1:], float(np.median(data1)-2),
    #         '中位数值线: ',
    #         color='#dc5034', fontsize=15)

    ax.locator_params(nbins=3)
    ax.tick_params(labelsize=20)
    ax.set_xlabel('blocks(num)', fontsize=fontsize)
    ax.set_ylabel('time(ms)', fontsize=fontsize)
    ax.set_xlim(0)
    ax.set_ylim(0)
    ax.legend([l3,l1,l2],['Total_buffer_time','RGB_buffer_time','Net_buffer_time'],loc='upper right',prop={'family' : 'Times New Roman', 'size'   : 20})
    ax.grid(linestyle="--")  # 设置背景网格线为虚线
    ax.set_xticks(np.arange(0,600,60))
    ax.set_yticks(np.arange(0,max(dataDeltime)+2,1))
    # ax.legend((rect,),("time ms",))
    ax.set_title(title, fontsize=fontsize)

def draw_buffer_container(RGB_Buffer_Count_Data,FrameRGB_Deltime_Data,Net_Buffer_Count_Data,\
    FrameNet_Deltime_Data,saveName):
    # fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(nrows=2, ncols=2)
    fig = plt.figure()
    gs1 = gridspec.GridSpec(2, 1)
    ax1 = fig.add_subplot(gs1[0])
    ax2 = fig.add_subplot(gs1[1])
    example_plot_Deltime(ax1,FrameRGB_Deltime_Data,FrameNet_Deltime_Data,"Buffer_Deltime",30)
    example_plot_Count(ax2,RGB_Buffer_Count_Data,Net_Buffer_Count_Data,"Buffer_Count_Num")
    # example_plot(ax2,RGBPush_Data,"RGBPush_Data")
    fig.set_size_inches(18,10)
    plt.tight_layout()
    # fig.set_title("frames_jitter", fontsize=20)
    #设置坐标轴刻度
    # my_x_ticks = np.arange(0,240,10)      #显示范围为-5至5，每0.5显示一刻度
    # my_y_ticks = np.arange(5,60,5)      #显示范围为-2至2，每0.2显示一刻度
    # fig.xticks(my_x_ticks)
    # fig.yticks(my_y_ticks)

    plt.savefig(saveName, format='svg') 

def drawRGB_YUV_buffer(RGB_Buffer_Count_Data,FrameDeltime_Data,saveName):
    RGB_Buffer_Count_List = list()
    FrameDeltime_List = list()
    frams = list()
    num = 0
    # 注意数据可能没有那么多
    print("len(RGB_Buffer_Count_Data):"+str(len(RGB_Buffer_Count_Data)))
    print("len(FrameDeltime_Data):"+str(len(FrameDeltime_Data)))
    for i in range(60,600):  # 从第二行开始读取
        RGB_Buffer_Count_List.append(float(RGB_Buffer_Count_Data[i][4]))  # 将第三列数据从第二行读取到最后一行赋给列表x
        FrameDeltime_List.append(float(FrameDeltime_Data[i][2]))  # 将第二列数据从第二行读取到最后一行赋给列表
        num = num + 1
        frams.append(num)

    fig1 = plt.subplot(211)
    # fig1.set_ylim(5,60)
    fig1.plot(frams,FrameDeltime_List,color='b',label='FrameDeltime')  #也可以用RGB值表示颜色
    
    fig2 = plt.subplot(212)
    # fig2.set_ylim(0,30)
    fig2.plot(frams,RGB_Buffer_Count_List,color='r',label='RGB_Buffer_Count')        # r表示红色

    #####非必须内容#########
    # plt.text(0.5, 0, 'frames', ha='center')
    # plt.text(0, 0.5, 'time(ms)', va='center', rotation='vertical')
    plt.xlabel('frames(Num)')    #x轴表示
    plt.ylabel('count(Num)')   #y轴表示
    # plt.title("chart")      #图标标题表示
    fig1.legend()            #每条折线的label显示
    fig2.legend()            #每条折线的label显示
    plt.suptitle('buffer_container')

    plt.savefig(saveName, format='svg') 

    # plt.show()
    #记住，如果你要show()的话，一定要先savefig，再show。如果你先show了，存出来的就是一张白纸。


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
    # RGB_Buffer持有数量和每一块待在里面的时间
    RGB_Buffer_Count_Data = ft.filteByLogType(root,"RGB_Buffer_Count")
    exDataRGB_Push_Data = ft.filteByLogType(root,"RGB_Push")
    exDataRGB_Pop_Data = ft.filteByLogType(root,"RGB_Pop")
    FrameRGB_Deltime_Data = ft.calculate2DeltimeList(exDataRGB_Push_Data,exDataRGB_Pop_Data)
    # Net_Buffer持有数量和每一块待在里面的时间
    Net_Buffer_Count_Data = ft.filteByLogType(root,"Net_Buffer_Count")
    Net_Buffer_Push_Data = ft.filteByLogType(root,"Net_Produce")
    Net_Buffer_Pop_Data = ft.filteByLogType(root,"Net_Consume")
    FrameNet_Deltime_Data = ft.calculate2DeltimeList(Net_Buffer_Push_Data,Net_Buffer_Pop_Data)
    # drawRGB_YUV_buffer(RGB_Buffer_Count_Data,FrameRGB_Deltime_Data,save+'data_buffer_container.svg')
    draw_buffer_container(RGB_Buffer_Count_Data,FrameRGB_Deltime_Data,Net_Buffer_Count_Data,\
    FrameNet_Deltime_Data,save+'data_buffer_container.svg')
    print("--done!--")