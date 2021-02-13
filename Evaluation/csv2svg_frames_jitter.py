# coding=utf-8
import sys
import csv
import matplotlib.pyplot as plt
import tool_filter as ft;
from matplotlib.pyplot import figure
import numpy as np
import matplotlib.gridspec as gridspec

def example_plot(ax,dataEx,title, fontsize=12):
    data = list()
    frams = list()
    num = 0
    print("len(dataEx):"+str(len(dataEx)))
    for i in range(0,240):  # 从第二行开始读取
        data.append(float(dataEx[i][2])) 
        num = num + 1
        frams.append(num)

    ax.plot(frams,data)
    # 绘制平均值线
    ax.hlines(np.mean(data), frams[0], frams[-1:],
          linestyles='-.', colors='#dc5034')

    ax.locator_params(nbins=3)
    ax.set_xlabel('frames(num)', fontsize=fontsize)
    ax.set_ylabel('time(ms)', fontsize=fontsize)
    ax.set_xlim(0,240)
    ax.set_ylim(5,60)
    ax.grid(linestyle="--")  # 设置背景网格线为虚线
    ax.set_xticks(np.arange(0,240,10))
    ax.set_title(title, fontsize=fontsize)

def drawRGB_YUV_jitter(cameraStartGet_Data,RGBPush_Data,RGBPop_Data,YUVGet_Data,Net_Produce_Data,saveName):
    # fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(nrows=2, ncols=2)
    fig = plt.figure()
    gs1 = gridspec.GridSpec(5, 1)
    ax1 = fig.add_subplot(gs1[0])
    ax2 = fig.add_subplot(gs1[1])
    ax3 = fig.add_subplot(gs1[2])
    ax4 = fig.add_subplot(gs1[3])
    ax5 = fig.add_subplot(gs1[4])
    example_plot(ax1,cameraStartGet_Data,"cameraStartGet_Data")
    example_plot(ax2,RGBPush_Data,"RGBPush_Data")
    example_plot(ax3,RGBPop_Data,"RGBPop_Data")
    example_plot(ax4,YUVGet_Data,"YUVGet_Data")
    example_plot(ax5,Net_Produce_Data,"Net_Produce_Data")
    fig.set_size_inches(18,10)
    plt.tight_layout()
    # fig.set_title("frames_jitter", fontsize=20)
    #设置坐标轴刻度
    # my_x_ticks = np.arange(0,240,10)      #显示范围为-5至5，每0.5显示一刻度
    # my_y_ticks = np.arange(5,60,5)      #显示范围为-2至2，每0.2显示一刻度
    # fig.xticks(my_x_ticks)
    # fig.yticks(my_y_ticks)

    plt.savefig(saveName, format='svg') 

def drawRGB_YUV_buffer(startToRGB_Data,RGB2YUV_Data,saveName):
    s2RGB = list()
    RGB2YUV = list()
    frams = list()
    num = 0
    print("len(startToRGB_Data):"+str(len(startToRGB_Data)))
    print("len(RGB2YUV_Data):"+str(len(RGB2YUV_Data)))
    # 注意数据可能没有那么多
    for i in range(60,240):  # 从第二行开始读取
        s2RGB.append(float(startToRGB_Data[i][2]))  # 将第三列数据从第二行读取到最后一行赋给列表x
        RGB2YUV.append(float(RGB2YUV_Data[i][2]))  # 将第二列数据从第二行读取到最后一行赋给列表
        num = num + 1
        frams.append(num)

    fig1 = plt.subplot(211)
    fig1.set_ylim(5,60)
    fig1.plot(frams,RGB2YUV,color='b',label='RGB_to_YUV')  #也可以用RGB值表示颜色
    
    fig2 = plt.subplot(212)
    fig2.set_ylim(5,60)
    fig2.plot(frams,s2RGB,color='r',label='Get_RGB')        # r表示红色

    #####非必须内容#########
    # plt.text(0.5, 0, 'frames', ha='center')
    # plt.text(0, 0.5, 'time(ms)', va='center', rotation='vertical')
    plt.xlabel('frames')    #x轴表示
    plt.ylabel('time(ms)')   #y轴表示
    # plt.title("chart")      #图标标题表示
    fig1.legend()            #每条折线的label显示
    fig2.legend()            #每条折线的label显示
    plt.suptitle('frames_jitter')

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
    # OpenCV的Camera录制while的抖动(GetRGB)
    exDataStartGetRGB1 = ft.filteByDetail(root,"start_CatchFrame")
    exDataStartGetRGB2 = exDataStartGetRGB1[1:]
    cameraStartGet_Data = ft.calculate2DeltimeList(exDataStartGetRGB1,exDataStartGetRGB2)
    # RGB_buffer_out->RGB_buffer_In的Pop抖动
    exDataRGB_Push1 = ft.filteByDetail(root,"RGB_Push")
    exDataRGB_Push2 = exDataRGB_Push1[1:]
    RGBPush_Data = ft.calculate2DeltimeList(exDataRGB_Push1,exDataRGB_Push2)
    # RGB_buffer_In->RGB_buffer_Out的Pop抖动
    exDataRGB_Pop1 = ft.filteByDetail(root,"RGB_Pop")
    exDataRGB_Pop2 = exDataRGB_Pop1[1:]
    RGBPop_Data = ft.calculate2DeltimeList(exDataRGB_Pop1,exDataRGB_Pop2)
    # RGB_Pop->YUV的抖动
    exDataRGB_ToYUV1 = ft.filteByDetail(root,"FrameToYUV")
    exDataRGB_ToYUV2 = exDataRGB_ToYUV1[1:]
    YUVGet_Data = ft.calculate2DeltimeList(exDataRGB_ToYUV1,exDataRGB_ToYUV2)
    # YUV->NetYUV的抖动
    exDataNet_Produce1 = ft.filteByDetail(root,"Net_Produce")
    exDataNet_Produce2 = exDataNet_Produce1[1:]
    Net_Produce_Data = ft.calculate2DeltimeList(exDataNet_Produce1,exDataNet_Produce2)
    # startToRGB = ft.calculate2DeltimeList(exDataStart,exDataRGB_Push)
    # exDataRGB_ToYUV_Delta = ft.calculate2DeltimeList(exDataRGB_ToYUV1,exDataRGB_ToYUV2)
    # exDataRGB_Push_Delta = ft.calculate2DeltimeList(exDataRGB_Push1,exDataRGB_Push2)
    # drawRGB_YUV_buffer(exDataRGB_Push_Delta,exDataRGB_ToYUV_Delta,save+'data_frames_jitter.svg')
    drawRGB_YUV_jitter(cameraStartGet_Data,RGBPush_Data,RGBPop_Data,YUVGet_Data,Net_Produce_Data,save+'data_frames_jitter.svg')
    print("--done!--")
    