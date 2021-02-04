# coding=UTF-8<code>
import sys
import csv
import matplotlib.pyplot as plt

def drawLatencyBarServer(saveName):

    dt = list()
    w = list()
    numList = list()
    num = 0

    for i in range(1, length_zu):  # 从第二行开始读取
        if exampleData[i][1] == "p":
            continue
        w.append(float(exampleData[i][2]))  # 将第三列数据从第二行读取到最后一行赋给列表x
        dt.append(exampleData[i][4])  # 将第二列数据从第二行读取到最后一行赋给列表
        numList.append(num)
        num+=1

    #绘制水平条形图
    rect=plt.barh(bottom=numList,height=0.35,width=w,align="center",color=colorG)
    #显示汉字，前面加u，代表使用unicode
    #y轴标记
    plt.ylabel('function')

    #x轴标记
    plt.xlabel('time')

    #设置x轴可读显示
    plt.yticks(numList,dt)

    #显示柱状图信息
    plt.legend((rect,),("time ms",))

    # 防止图片被切割
    plt.tight_layout(pad=6, w_pad=0, h_pad=0.5)

    # 使用save存图
    plt.savefig(saveName, format='svg') 
    #显示绘制图片
    # plt.show()

def drawLatencyBarPlayer(saveName):

    dt = list()
    w = list()
    numList = list()
    num = 0

    for i in range(1, length_zu):  # 从第二行开始读取
        if exampleData[i][1] == "s":
            continue
        w.append(float(exampleData[i][2]))  # 将第三列数据从第二行读取到最后一行赋给列表x
        dt.append(exampleData[i][4])  # 将第二列数据从第二行读取到最后一行赋给列表
        numList.append(num)
        num+=1

    #绘制水平条形图
    rect=plt.barh(bottom=numList,height=0.35,width=w,align="center",color=colorG)
    #显示汉字，前面加u，代表使用unicode
    #y轴标记
    plt.ylabel('function')

    #x轴标记
    plt.xlabel('time')

    #设置x轴可读显示
    plt.yticks(numList,dt)

    #显示柱状图信息
    plt.legend((rect,),("time ms",))

    # 防止图片被切割
    plt.tight_layout(pad=6, w_pad=0, h_pad=0.5)

    # 使用save存图
    plt.savefig(saveName, format='svg') 
    #显示绘制图片
    # plt.show()

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

    global exampleFile # 打开csv文件
    exampleFile = open(root)

    global exampleReader
    exampleReader = csv.reader(exampleFile)  # 读取csv文件

    global exampleData
    exampleData = list(exampleReader)  # csv数据转换为列表

    global length_zu
    length_zu = len(exampleData)  # 得到数据行数

    global length_yuan
    length_yuan = len(exampleData[0])  # 得到每行长度

    global colorG
    colorG = ['mediumblue']
    drawLatencyBarPlayer(save+'data_latency_Player.svg')
    drawLatencyBarServer(save+'data_latency_Server.svg')
    print("--done!--")
    