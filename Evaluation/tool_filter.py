# coding=UTF-8<code>
import sys
import csv
import matplotlib.pyplot as plt

def csv2Data(root):
    global exampleFile 
    global exampleReader
    global exampleData
    global length_zu
    global length_yuan
    # 打开csv文件
    exampleFile = open(root)
    # 读取csv文件
    exampleReader = csv.reader(exampleFile)
    # csv数据转换为列表
    exampleData = list(exampleReader)
    # 得到数据行数
    length_zu = len(exampleData)
    # 得到每行长度
    length_yuan = len(exampleData[0])
    return

def getcsv2List(root):
    csv2Data(root)
    rt = list()
    for i in range(1, length_zu):  # 从第二行开始读取
        rt.append(exampleData[i])
        # print(exampleData[i])

    return rt

def filteByLogTypeWhich(root,logType,which):
    csv2Data(root)
    rt = list()
    for i in range(1, length_zu):  # 从第二行开始读取
        if exampleData[i][0] != logType:
            continue
        if exampleData[i][1] != which:
            continue
        rt.append(exampleData[i])
        # print(exampleData[i])

    return rt

def filteByDetail(root,detail):
    csv2Data(root)
    rt = list()
    for i in range(1, length_zu):  # 从第二行开始读取
        if exampleData[i][4] != detail:
            continue
        rt.append(exampleData[i])
        # print(exampleData[i])

    return rt

def filteByLogType(root,logType):
    csv2Data(root)
    rt = list()
    for i in range(1, length_zu):  # 从第二行开始读取
        if exampleData[i][0] != logType:
            continue
        rt.append(exampleData[i])
        # print(exampleData[i])

    return rt

# LogType,Which,AlgoTime,TimeType,Detail
def calculate2DeltimeList(lst1,lst2):
    rt = list()
    minLen = min(len(lst1),len(lst2))
    # print('len(lst1)'+str(lst1[0][0])+' : '+str(len(lst1)))
    # print('len(lst2)'+str(lst2[0][0])+' : '+str(len(lst2)))
    # 输出一下数值
    # if minLen == len(lst1) :
    #     rt = lst1
    # else:
    #     rt = lst2

    for i in range(minLen):  # 从第二行开始读取
        rtLine = ['LogType','Which','AlgoTime','TimeType','Detail']
        rtLine[2] = str(int(lst2[i][2])-int(lst1[i][2]))
        rtLine[4] = lst1[i][4]+"_To_"+lst2[i][4]
        rt.append(rtLine)

    return rt

def filteFrame(root):
    csv2Data(root)
    rt = list()
    for i in range(1, length_zu):  # 从第二行开始读取
        rt.append(exampleData[i][4])
        # print(exampleData[i])

    return rt

def countIFrame(root,start,end):
    # start从0开始
    csv2Data(root)
    num = 0
    for i in range(1+start, end+1):  # 从第二行开始读取
        if exampleData[i][1] == "I_frame":
            num += 1

    return num

def countPFrame(root,start,end):
    # start从0开始
    csv2Data(root)
    num = 0
    for i in range(1+start, end+1):  # 从第二行开始读取
        if exampleData[i][1] == "P_frame":
            num += 1

    return num

def countPFrameInList(root,start,end,inList):
    # SDL_RenderPresent_n_List = ft.filteFrame('./data/data_SDL_RenderPresent.csv')
    # start从0开始
    csv2Data(root)
    num = 0
    for i in range(1, length_zu):  # 从第二行开始读取
        index = i-1
        if exampleData[i][1] == "P_frame":
            if int(inList[index])+1>=start and int(inList[index])+1<=end:
                if str(exampleData[i][4]) in inList:
                    # print("P_frame:"+inList[index])
                    num += 1

    return num

def countIFrameInList(root,start,end,inList):
    # start从0开始
    # inList Pop
    csv2Data(root)
    num = 0
    # print("====================")
    for i in range(1, length_zu):  # 从第二行开始读取
        index = i-1
        if exampleData[i][1] == "I_frame":
            if int(inList[index])+1>=start and int(inList[index])+1<=end:
                if str(exampleData[i][4]) in inList:
                    # print(inList[index])
                    num += 1

    return num

def filteByFrameList(root,inList):
    csv2Data(root)
    rt = list()
    # print(inList)
    # print("exampleData[1][4]:"+exampleData[1][4])
    # print("in")
    # if str(exampleData[1][4]) in inList:
        # print("in")
    for i in range(1, length_zu):  # 从第二行开始读取
        # print("filteByFrameList : "+exampleData[i][4])
        if str(exampleData[i][4]) in inList:
            # print("exampleData[i][4] : "+exampleData[i][4])
            rt.append(exampleData[i])
            continue

        # print("Name : "+str(exampleData[i][0])+" ; not in inList Num : "+str(exampleData[i][4]))

    return rt

    