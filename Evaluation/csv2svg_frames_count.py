# coding=utf-8
import sys
import csv
import tool_filter as ft;
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import os
import matplotlib.gridspec as gridspec
matplotlib.rcParams.update({'font.size': 20})
def auto_label(ax,rects):
    for rect in rects:
        height = rect.get_height()
        ax.annotate('{}'.format(height), # put the detail data
                    xy=(rect.get_x() + rect.get_width() / 2, height), # get the center location.
                    xytext=(0, 3),  # 3 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom')

def auto_text(ax,rects):
    for rect in rects:
        ax.text(rect.get_x(), rect.get_height(), rect.get_height(), ha='left', va='bottom')

def draw_frames_count(ax,path,titleName,fontsize=30):
    labels = [ '0-100', '100-200', '200-300', '300-400', '400-500', '500-600']
    # I_Frame = [4, 34, 30, 35, 27, 2, 1]
    # P_Frame = [96, 32, 34, 20, 25, 88, 66]
    print("=====================")
    print(path)
    I_Frame = list()
    P_Frame = list()
    # Produce
    # I_Frame.append(ft.countIFrame('./data/data_Net_Consume.csv',0,100))
    # P_Frame.append(ft.countPFrame('./data/data_Net_Consume.csv',0,100))

    data_pJitter_Pop_n_List = ft.filteFrame(path)

    I_Frame.append(ft.countIFrameInList(path,0,100,data_pJitter_Pop_n_List))
    P_Frame.append(ft.countPFrameInList(path,0,100,data_pJitter_Pop_n_List))

    I_Frame.append(ft.countIFrameInList(path,101,200,data_pJitter_Pop_n_List))
    P_Frame.append(ft.countPFrameInList(path,101,200,data_pJitter_Pop_n_List))

    I_Frame.append(ft.countIFrameInList(path,201,300,data_pJitter_Pop_n_List))
    P_Frame.append(ft.countPFrameInList(path,201,300,data_pJitter_Pop_n_List))

    I_Frame.append(ft.countIFrameInList(path,301,400,data_pJitter_Pop_n_List))
    P_Frame.append(ft.countPFrameInList(path,301,400,data_pJitter_Pop_n_List))

    I_Frame.append(ft.countIFrameInList(path,401,500,data_pJitter_Pop_n_List))
    P_Frame.append(ft.countPFrameInList(path,401,500,data_pJitter_Pop_n_List))

    I_Frame.append(ft.countIFrameInList(path,501,600,data_pJitter_Pop_n_List))
    P_Frame.append(ft.countPFrameInList(path,501,600,data_pJitter_Pop_n_List))

    index = np.arange(len(labels))
    width = 0.2

    # fig, ax = plt.subplots()
    rect1 = ax.bar(index - width / 2, I_Frame, color ='lightcoral', width=width, label ='I_Frame')
    rect2 = ax.bar(index + width / 2, P_Frame, color ='springgreen', width=width, label ='P_Frame')

    ax.set_title(titleName, fontsize=fontsize)
    ax.set_xticks(ticks=index)
    ax.set_xticklabels(labels)
    ax.set_ylabel('Frames',fontsize=fontsize)
    ax.tick_params(labelsize=20)
    ax.set_ylim(0, 160)
    ax.grid(linestyle="--")  # 设置背景网格线为虚线
    # auto_label(ax,rect1)
    # auto_label(ax,rect2)
    auto_text(ax,rect1)
    auto_text(ax,rect2)

    ax.legend(loc='upper right', frameon=False,prop={'family' : 'Times New Roman', 'size'   : 20})
    # fig.tight_layout()
    # plt.savefig(saveName, format='svg') 

def draw_frames_count_Main(Net_Consume_Path,pJitter_Pop_Path,saveName):
    # fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(nrows=2, ncols=2)
    fig = plt.figure()
    gs1 = gridspec.GridSpec(2, 1)
    ax1 = fig.add_subplot(gs1[0])
    ax2 = fig.add_subplot(gs1[1])
    draw_frames_count(ax1,Net_Consume_Path,'server_frame')
    draw_frames_count(ax2,pJitter_Pop_Path,'player_frame')
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
    draw_frames_count_Main('./data/data_Net_Consume.csv','./data/data_pJitter_Pop.csv',save+'data_frames_count.svg')

    print("--done!--")