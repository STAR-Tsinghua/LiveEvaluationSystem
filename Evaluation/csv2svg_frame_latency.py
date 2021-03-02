# coding=utf-8
import sys
import csv
import matplotlib.pyplot as plt
import tool_filter as ft;
from matplotlib.pyplot import figure
import numpy as np
import matplotlib.gridspec as gridspec
import more_itertools
import pandas as pd

def example_plot_PerDeltime_Race(ax,title,fontsize=12):
    # Prepare Data
    df_raw = pd.read_csv("./data/data_frame_deltime.csv")
    # print(df_raw)

    # return
    # df = df_raw[['cty', 'manufacturer']].groupby('manufacturer').apply(lambda x: x.mean())
    df = df_raw.mean()
    print(df)
    print(sorted(df.items(), key = lambda kv:(kv[1], kv[0])))
    sdf = sorted(df.items(), key = lambda kv:(kv[1], kv[0]))
    # df.sort_values('cty', inplace=True)
    # df.reset_index(inplace=True)
    # Draw plot
    # fig, ax = plt.subplots(figsize=(16,10), facecolor='white', dpi= 80)
    # ax.vlines(x=df.index, ymin=0, ymax=df.cty, alpha=0.7, linewidth=20)
    dataKey = list()
    dataValue = list()
    for key,value in sdf:
        print 'key : %s  ;  value : %s'%(key,value)
        # if key == 'All_Deltatime':
            # continue
        dataKey.append(key)
        dataValue.append(value)
        ax.text(i, value+0.5, round(value, 1), horizontalalignment='center')

    # del sdf['All_Deltatime']
    ax.vlines(x=dataKey, ymin=0, ymax=dataValue, alpha=0.8,color='firebrick', linewidth=20)

    # Title, Label, Ticks and Ylim
    # ax.set_title('Bar Chart for Highway Mileage', fontdict={'size':22})
    ax.set_title(title, fontsize=fontsize)
    ax.set_ylabel('time(ms)', fontsize=fontsize)
    # ax.set(ylabel='Miles Per Gallon', ylim=(0, 30))
    plt.xticks(dataKey, rotation=30, horizontalalignment='right', fontsize=10)
    ax.set_yticks(np.arange(0,200,20))
    # Add patches to color the X axis labels
    # p1 = patches.Rectangle((.57, -0.005), width=.33, height=.13, alpha=.1, facecolor='green', transform=fig.transFigure)
    # p2 = patches.Rectangle((.124, -0.005), width=.446, height=.13, alpha=.1, facecolor='red', transform=fig.transFigure)
    # fig.add_artist(p1)
    # fig.add_artist(p2)
    # plt.show()
    ax.grid(linestyle="--")  # 设置背景网格线为虚线

def example_plot_Deltime_Total(ax,All_Deltatime,Server_Deltatime,Net_Deltatime,Player_Deltatime,\
    title, fontsize=12):
    data1 = list()
    data2 = list()
    data3 = list()
    data4 = list()
    dataTotal = list()
    frams = list()
    num = 0
    print("len(All_Deltatime):"+str(len(All_Deltatime)))
    print("len(Server_Deltatime):"+str(len(Server_Deltatime)))
    print("len(Net_Deltatime):"+str(len(Net_Deltatime)))
    print("len(Player_Deltatime):"+str(len(Player_Deltatime)))
    for i in range(0,600):  # 从第二行开始读取
        # print("len(dataDeltime1):"+dataDeltime1[i][2])
        tmp1 = float(All_Deltatime[i][2])
        tmp2 = float(Server_Deltatime[i][2])
        tmp3 = float(Net_Deltatime[i][2])
        tmp4 = float(Player_Deltatime[i][2])
        data1.append(tmp1)
        data2.append(tmp2)
        data3.append(tmp3)
        data4.append(tmp4)
        # dataTotal.append(tmp1+tmp2+tmp3+tmp4+tmp5)
        num = num + 1
        frams.append(num)

    l1 ,= ax.plot(frams,data1)
    l2 ,= ax.plot(frams,data2)
    l3 ,= ax.plot(frams,data3)
    l4 ,= ax.plot(frams,data4)

    ax.locator_params(nbins=3)
    ax.set_xlabel('blocks(num)', fontsize=fontsize)
    ax.set_ylabel('time(ms)', fontsize=fontsize)
    ax.set_xlim(0)
    ax.set_ylim(0)
    ax.legend([l1,l2,l3,l4],['All_Deltatime','Server_Deltatime',\
    'Net_Deltatime','Player_Deltatime'],loc='upper right')
    ax.grid(linestyle="--")  # 设置背景网格线为虚线
    ax.set_xticks(np.arange(0,max(frams),25))
    ax.set_yticks(np.arange(0,240,20))
    # ax.legend((rect,),("time ms",))
    ax.set_title(title, fontsize=fontsize)

def example_plot_Deltime_Player(ax,buffer_read_To_pJitter_Push,pJitter_Push_To_pJitter_Pop,pJitter_Pop_To_pYUV_Get,\
    pYUV_Get_To_pRGB_Get,pRGB_Get_To_SDL_RenderPresent,title, fontsize=12):
    data1 = list()
    data2 = list()
    data3 = list()
    data4 = list()
    data5 = list()
    dataTotal = list()
    frams = list()
    num = 0
    print("len(buffer_read_To_pJitter_Push):"+str(len(buffer_read_To_pJitter_Push)))
    print("len(pJitter_Push_To_pJitter_Pop):"+str(len(pJitter_Push_To_pJitter_Pop)))
    print("len(pJitter_Pop_To_pYUV_Get):"+str(len(pJitter_Pop_To_pYUV_Get)))
    print("len(pYUV_Get_To_pRGB_Get):"+str(len(pYUV_Get_To_pRGB_Get)))
    print("len(pRGB_Get_To_SDL_RenderPresent):"+str(len(pRGB_Get_To_SDL_RenderPresent)))
    for i in range(0,600):  # 从第二行开始读取
        # print("len(dataDeltime1):"+dataDeltime1[i][2])
        tmp1 = float(buffer_read_To_pJitter_Push[i][2])
        tmp2 = float(pJitter_Push_To_pJitter_Pop[i][2])
        tmp3 = float(pJitter_Pop_To_pYUV_Get[i][2])
        tmp4 = float(pYUV_Get_To_pRGB_Get[i][2])
        tmp5 = float(pRGB_Get_To_SDL_RenderPresent[i][2])
        data1.append(tmp1)
        data2.append(tmp2)
        data3.append(tmp3)
        data4.append(tmp4)
        data5.append(tmp5)
        # dataTotal.append(tmp1+tmp2+tmp3+tmp4+tmp5)
        num = num + 1
        frams.append(num)

    l1 ,= ax.plot(frams,data1)
    l2 ,= ax.plot(frams,data2)
    l3 ,= ax.plot(frams,data3)
    l4 ,= ax.plot(frams,data4)
    l5 ,= ax.plot(frams,data5)

    ax.locator_params(nbins=3)
    ax.set_xlabel('blocks(num)', fontsize=fontsize)
    ax.set_ylabel('time(ms)', fontsize=fontsize)
    ax.set_xlim(0)
    ax.set_ylim(0)
    ax.legend([l1,l2,l3,l4,l5],['buffer_read_To_pJitter_Push','pJitter_Push_To_pJitter_Pop',\
    'pJitter_Pop_To_pYUV_Get','pYUV_Get_To_pRGB_Get','pRGB_Get_To_SDL_RenderPresent'],loc='upper right')
    ax.grid(linestyle="--")  # 设置背景网格线为虚线
    ax.set_xticks(np.arange(0,max(frams),25))
    ax.set_yticks(np.arange(0,140,20))
    # ax.legend((rect,),("time ms",))
    ax.set_title(title, fontsize=fontsize)

def example_plot_Deltime_Server(ax,CatchFrame_To_RGB_Push,RGB_Push_Data_To_RGB_Pop,RGB_Pop_To_FrameToYUV,\
    FrameToYUV_To_Net_Produce,Net_Produce_To_Net_Consume,title, fontsize=12):
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
    for i in range(0,600):  # 从第二行开始读取
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
        # dataTotal.append(tmp1+tmp2+tmp3+tmp4+tmp5)
        num = num + 1
        frams.append(num)

    l1 ,= ax.plot(frams,data1)
    l2 ,= ax.plot(frams,data2)
    l3 ,= ax.plot(frams,data3)
    l4 ,= ax.plot(frams,data4)
    l5 ,= ax.plot(frams,data5)
    # l6 ,= ax.plot(frams,dataTotal)
    # 绘制中位数值线
    # ax.hlines(np.median(dataTotal), frams[0], frams[-1:],
    #       linestyles='-.', colors='#dc5034')

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
    ax.legend([l1,l2,l3,l4,l5],['CatchFrame_To_RGB_Push','RGB_Push_To_RGB_Pop',\
    'RGB_Pop_To_FrameToYUV','FrameToYUV_To_Net_Produce','Net_Produce_To_Net_Consume'],loc='upper right')
    ax.grid(linestyle="--")  # 设置背景网格线为虚线
    ax.set_xticks(np.arange(0,max(frams),25))
    ax.set_yticks(np.arange(0,140,20))
    # ax.legend((rect,),("time ms",))
    ax.set_title(title, fontsize=fontsize)

def draw_frame_latency(CatchFrame_To_RGB_Push,RGB_Push_Data_To_RGB_Pop,\
    RGB_Pop_To_FrameToYUV,FrameToYUV_To_Net_Produce,Net_Produce_To_Net_Consume,\
    buffer_read_To_pJitter_Push_Data,pJitter_Push_To_pJitter_Pop,pJitter_Pop_To_pYUV_Get,\
    pYUV_Get_To_pRGB_Get,pRGB_Get_To_SDL_RenderPresent,\
    All_Deltatime,Server_Deltatime,Net_Deltatime,Player_Deltatime,saveName):
    # fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(nrows=2, ncols=2)
    fig = plt.figure()
    gs1 = gridspec.GridSpec(2, 2)
    ax1 = fig.add_subplot(gs1[0])
    ax2 = fig.add_subplot(gs1[1])
    ax3 = fig.add_subplot(gs1[2])
    ax4 = fig.add_subplot(gs1[3])
    example_plot_Deltime_Server(ax1,CatchFrame_To_RGB_Push,RGB_Push_Data_To_RGB_Pop,\
    RGB_Pop_To_FrameToYUV,FrameToYUV_To_Net_Produce,Net_Produce_To_Net_Consume,"Frame_Deltime_Server")
    example_plot_Deltime_Player(ax2,buffer_read_To_pJitter_Push_Data,pJitter_Push_To_pJitter_Pop,pJitter_Pop_To_pYUV_Get,\
    pYUV_Get_To_pRGB_Get,pRGB_Get_To_SDL_RenderPresent,"Frame_Deltime_Player")
    example_plot_Deltime_Total(ax3,All_Deltatime,Server_Deltatime,\
    Net_Deltatime,Player_Deltatime,"Frame_Deltime_Total")

    example_plot_PerDeltime_Race(ax4,"Frame_Detime_Compare")
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
    # ============= Server =============
    CatchFrame_Data = ft.filteByLogType(root,"start_CatchFrame")
    RGB_Push_Data = ft.filteByLogType(root,"RGB_Push")
    RGB_Pop_Data = ft.filteByLogType(root,"RGB_Pop")
    FrameToYUV_Data = ft.filteByLogType(root,"FrameToYUV")
    Net_Produce_Data = ft.filteByLogType(root,"Net_Produce")
    Net_Consume_Data = ft.filteByLogType(root,"Net_Consume")

    # ============= Player =============
    buffer_read_Data = ft.filteByLogType(root,"buffer_read")
    pJitter_Push_Data = ft.filteByLogType(root,"pJitter_Push")
    pJitter_Pop_Data = ft.filteByLogType(root,"pJitter_Pop")
    pYUV_Get_Data = ft.filteByLogType(root,"pYUV_Get")
    pRGB_Get_Data = ft.filteByLogType(root,"pRGB_Get")
    SDL_RenderPresent_Data = ft.filteByLogType(root,"SDL_RenderPresent")

    # Net_Consume到数据完全发送出去还有一段时间别忘了！！！

    # ============= Server =============
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

    # ============= Player =============
    # buffer_read_Data->pJitter_Push_Data
    buffer_read_To_pJitter_Push = ft.calculate2DeltimeList(buffer_read_Data,pJitter_Push_Data)
    # pJitter_Push_Data->pJitter_Pop_Data
    pJitter_Push_To_pJitter_Pop = ft.calculate2DeltimeList(pJitter_Push_Data,pJitter_Pop_Data)
    # pJitter_Pop_Data->pYUV_Get_Data
    pJitter_Pop_To_pYUV_Get = ft.calculate2DeltimeList(pJitter_Pop_Data,pYUV_Get_Data)
    # pYUV_Get_Data->pRGB_Get_Data
    pYUV_Get_To_pRGB_Get = ft.calculate2DeltimeList(pYUV_Get_Data,pRGB_Get_Data)
    # pRGB_Get_Data->SDL_RenderPresent_Data
    pRGB_Get_To_SDL_RenderPresent = ft.calculate2DeltimeList(pRGB_Get_Data,SDL_RenderPresent_Data)

    # ============= Total =============
    # All Total
    All_Deltatime = ft.calculate2DeltimeList(CatchFrame_Data,SDL_RenderPresent_Data)
    # Server Total
    Server_Deltatime = ft.calculate2DeltimeList(CatchFrame_Data,Net_Consume_Data)
    # Net Total
    Net_Deltatime = ft.calculate2DeltimeList(Net_Consume_Data,buffer_read_Data)
    # Player Total
    Player_Deltatime = ft.calculate2DeltimeList(buffer_read_Data,SDL_RenderPresent_Data)

    data1 = list()
    data2 = list()
    data3 = list()
    data4 = list()
    data5 = list()
    data6 = list()
    data7 = list()
    data8 = list()
    data9 = list()
    data10 = list()
    data11 = list()
    data12 = list()
    data13 = list()
    data14 = list()
    #字典中的key值即为csv中列名
    for i in range(0,600):  # 从第二行开始读取
        # print("len(dataDeltime1):"+dataDeltime1[i][2])
        # Server
        tmp1 = float(CatchFrame_To_RGB_Push[i][2])
        tmp2 = float(RGB_Push_Data_To_RGB_Pop[i][2])
        tmp3 = float(RGB_Pop_To_FrameToYUV[i][2])
        tmp4 = float(FrameToYUV_To_Net_Produce[i][2])
        tmp5 = float(Net_Produce_To_Net_Consume[i][2])
        # Player
        tmp6 = float(buffer_read_To_pJitter_Push[i][2])
        tmp7 = float(pJitter_Push_To_pJitter_Pop[i][2])
        tmp8 = float(pJitter_Pop_To_pYUV_Get[i][2])
        tmp9 = float(pYUV_Get_To_pRGB_Get[i][2])
        tmp10 = float(pRGB_Get_To_SDL_RenderPresent[i][2])

        # Total
        tmp11 = float(All_Deltatime[i][2])
        tmp12 = float(Server_Deltatime[i][2])
        tmp13 = float(Net_Deltatime[i][2])
        tmp14 = float(Player_Deltatime[i][2])

        data1.append(tmp1)
        data2.append(tmp2)
        data3.append(tmp3)
        data4.append(tmp4)
        data5.append(tmp5)
        data6.append(tmp6)
        data7.append(tmp7)
        data8.append(tmp8)
        data9.append(tmp9)
        data10.append(tmp10)
        data11.append(tmp11)
        data12.append(tmp12)
        data13.append(tmp13)
        data14.append(tmp14)

    dataframe = pd.DataFrame({'CatchFrame_To_RGB_Push':data1,'RGB_Push_Data_To_RGB_Pop':data2,\
    'RGB_Pop_To_FrameToYUV':data3,'FrameToYUV_To_Net_Produce':data4,'Net_Produce_To_Net_Consume':data5,\
    'buffer_read_To_pJitter_Push':data6,'pJitter_Push_To_pJitter_Pop':data7,'pJitter_Pop_To_pYUV_Get':data8,\
    'pYUV_Get_To_pRGB_Get':data9,'pRGB_Get_To_SDL_RenderPresent':data10,'All_Deltatime':data11,\
    'Server_Deltatime':data12,'Net_Deltatime':data13,'Player_Deltatime':data14,\
    })
    # dataframe = pd.DataFrame({'CatchFrame_To_RGB_Push':np.mean(CatchFrame_To_RGB_Push)})
    #将DataFrame存储为csv,index表示是否显示行名，default=True
    dataframe.to_csv("./data/data_frame_deltime.csv",index=False,sep=',')

    draw_frame_latency(CatchFrame_To_RGB_Push,RGB_Push_Data_To_RGB_Pop,RGB_Pop_To_FrameToYUV,\
    FrameToYUV_To_Net_Produce,Net_Produce_To_Net_Consume,\
    buffer_read_To_pJitter_Push,pJitter_Push_To_pJitter_Pop,pJitter_Pop_To_pYUV_Get,\
    pYUV_Get_To_pRGB_Get,pRGB_Get_To_SDL_RenderPresent,\
    All_Deltatime,Server_Deltatime,Net_Deltatime,Player_Deltatime,save+'data_frame_latency.svg')
    print("--done!--")