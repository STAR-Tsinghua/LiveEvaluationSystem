# coding=utf-8
import sys
import csv
import matplotlib.pyplot as plt
import tool_filter as ft;
from matplotlib.pyplot import figure
import numpy as np

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
    # 此图计入帧间抖动,下图是摄像头录制抖动，上图是产出YUV的抖动
    RGB_Buffer_Count_Data = ft.filteByLogType(root,"RGB_Buffer_Count")
    exDataRGB_Push_Data = ft.filteByDetail(root,"RGB_Push")
    exDataRGB_Pop_Data = ft.filteByDetail(root,"RGB_Pop")
    FrameDeltime_Data = ft.calculate2DeltimeList(exDataRGB_Push_Data,exDataRGB_Pop_Data)
    # exDataRGB_Push = ft.filteByDetail(root,"RGB_Push")
    # exDataRGB_ToYUV1 = ft.filteByDetail(root,"FrameToYUV")
    # exDataRGB_ToYUV2 = exDataRGB_ToYUV1[1:]
    # startToRGB = ft.calculate2DeltimeList(exDataStart,exDataRGB_Push)
    # exDataRGB_ToYUV_Delta = ft.calculate2DeltimeList(exDataRGB_ToYUV1,exDataRGB_ToYUV2)
    drawRGB_YUV_buffer(RGB_Buffer_Count_Data,FrameDeltime_Data,save+'data_buffer_container.svg')
    print("--done!--")