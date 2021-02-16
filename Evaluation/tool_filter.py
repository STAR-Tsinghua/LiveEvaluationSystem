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