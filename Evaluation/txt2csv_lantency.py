# coding=UTF-8<code>
#!/usr/bin/env python3

import os
import re
import sys
import csv
import pandas as pd

def readTxtData(root,name):
	info_list = []   
	with open(root+name,encoding='utf-8') as f:
	    info_record = f.readlines()
	    for record in info_record:
	        info_rec_dic = eval(record)
	        info_list.append(info_rec_dic)

	return info_list
   
def writeData(root,name,save):
	headers = ['LogType', 'Which', 'AlgoTime', 'TimeType', 'Detail']
	data_list = readTxtData(root,name)
	with open(save,'w',newline='',encoding='utf-8') as fv:
	    writer = csv.DictWriter(fv, headers)
	    writer.writeheader()
	    writer.writerows(data_list)

# 合并同一个文件夹下多个txt
def MergeTxt(filepath,outfile):
    if os.path.exists(filepath+outfile):
        print("os.path.exists(filepath+outfile): true")
        os.remove(filepath+outfile)
    k = open(filepath+outfile, 'a+')
    for parent, dirnames, filenames in os.walk(filepath):
        for filepath in filenames:
            if filepath == "MainLogs.txt":
                continue
            print("filenames: " + str(filepath))
            txtPath = os.path.join(parent, filepath)  # txtpath就是所有文件夹的路径
            f = open(txtPath)
            # 换行写入
            k.write(f.read())

    k.close()


#csv的DictReader()和DictWriter()方法将以字典的方式处理csv文件，实质上是以OrderedDict方式进行处理
#默认将CSV文件的第一列作为字段名
def csvDictReader1(path):
    with open(path) as rf:
        reader = csv.DictReader(rf)
        items = list(reader)
    return items

#指定字段名，第一列也为数据
def csvDictReader2(path,fieldnames):
    with open(path) as wf:
        reader = csv.DictReader(wf,fieldnames=fieldnames)
        items = list(reader)
    return items
 
def csvDictWriter(path,data,fieldnames):
    with open(path,"w",newline="") as wf:
        writer = csv.DictWriter(wf,fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(data)

def readcsvColumn(fileName,col):
    with open(fileName,'rb') as csvfile:
        reader = csv.DictReader(csvfile)
        column = [row[col] for row in reader]
        print(column)

if __name__ == "__main__":

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
    print("=============== start run one =================")
    MergeTxt(root,'/MainLogs.txt')
    # save = data_latency_dtp.csv
    writeData(root,'/MainLogs.txt',save)
    # 生成需要特殊处理的csv(txt->csv)
    fileFather = './data/'
    # frame
    writeData(root,'/start_CatchFrame.txt',fileFather+'data_start_CatchFrame.csv')
    writeData(root,'/Net_Consume.txt',fileFather+'data_Net_Consume.csv')
    writeData(root,'/buffer_read.txt',fileFather+'data_buffer_read.csv')
    writeData(root,'/pJitter_Push.txt',fileFather+'data_pJitter_Push.csv')
    # frame
    writeData(root,'/pJitter_Pop.txt',fileFather+'data_pJitter_Pop.csv')
    writeData(root,'/pRGB_Get.txt',fileFather+'data_pRGB_Get.csv')
    writeData(root,'/pYUV_Get.txt',fileFather+'data_pYUV_Get.csv')
    writeData(root,'/SDL_RenderPresent.txt',fileFather+'data_SDL_RenderPresent.csv')

    all_data_frames = pd.DataFrame()
    datas1 = pd.read_csv(fileFather+'data_Net_Consume.csv',usecols=['AlgoTime','Detail','Which','TimeType'])
    datas2 = pd.read_csv(fileFather+'data_buffer_read.csv',usecols=['AlgoTime','Detail'])
    datas3 = pd.read_csv(fileFather+'data_pJitter_Push.csv',usecols=['AlgoTime','Detail'])
    datas4 = pd.read_csv(fileFather+'data_pJitter_Pop.csv',usecols=['AlgoTime','Detail','Which','TimeType'])
    datas5 = pd.read_csv(fileFather+'data_pRGB_Get.csv',usecols=['AlgoTime','Detail'])
    datas6 = pd.read_csv(fileFather+'data_pYUV_Get.csv',usecols=['AlgoTime','Detail'])
    datas7 = pd.read_csv(fileFather+'data_SDL_RenderPresent.csv',usecols=['AlgoTime','Detail'])

    all_data_frames.insert(all_data_frames.shape[1], 'data_Net_Consume_t', datas1['AlgoTime'])
    all_data_frames.insert(all_data_frames.shape[1], 'data_Net_Consume_n', datas1['Detail'])
    all_data_frames.insert(all_data_frames.shape[1], 'data_Net_Consume_f', datas1['Which'])
    all_data_frames.insert(all_data_frames.shape[1], 'data_Net_Consume_s', datas1['TimeType'])

    all_data_frames.insert(all_data_frames.shape[1], 'buffer_read_t', datas2['AlgoTime'])
    all_data_frames.insert(all_data_frames.shape[1], 'buffer_read_n', datas2['Detail'])

    all_data_frames.insert(all_data_frames.shape[1], 'pJitter_Push_t', datas3['AlgoTime'])
    all_data_frames.insert(all_data_frames.shape[1], 'pJitter_Push_n', datas3['Detail'])

    all_data_frames.insert(all_data_frames.shape[1], 'pJitter_Pop_t', datas4['AlgoTime'])
    all_data_frames.insert(all_data_frames.shape[1], 'pJitter_Pop_n', datas4['Detail'])
    all_data_frames.insert(all_data_frames.shape[1], 'pJitter_Pop_f', datas4['Which'])
    all_data_frames.insert(all_data_frames.shape[1], 'pJitter_Pop_s', datas4['TimeType'])

    all_data_frames.insert(all_data_frames.shape[1], 'pRGB_Get_t', datas5['AlgoTime'])
    all_data_frames.insert(all_data_frames.shape[1], 'pRGB_Get_n', datas5['Detail'])

    all_data_frames.insert(all_data_frames.shape[1], 'pYUV_Get_t', datas6['AlgoTime'])
    all_data_frames.insert(all_data_frames.shape[1], 'pYUV_Get_n', datas6['Detail'])

    all_data_frames.insert(all_data_frames.shape[1], 'SDL_RenderPresent_t', datas7['AlgoTime'])
    all_data_frames.insert(all_data_frames.shape[1], 'SDL_RenderPresent_n', datas7['Detail'])

    all_data_frames.to_csv(fileFather+"clean_frame_data.csv", index=False)
    print(all_data_frames)
    # 读csv为dic
    # fieldnames = ['LogType', 'Which', 'AlgoTime', 'TimeType', 'Detail']
    # data_buffer_read = csvDictReader1(fileFather+"data_buffer_read.csv")
    
    # 写“干净”数据入csv
    # csvDictWriter(fileFather+"clean_frame_data.csv",dict_country,fieldnames)


    
    # 去除播放器“丢掉”的包，得到“干净”的包组合
    
    print("--done!--")