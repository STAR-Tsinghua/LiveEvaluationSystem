# coding=UTF-8<code>
#!/usr/bin/env python3

import os
import re
import sys
import csv

def readerData(root):
	info_list = []   
	with open(root+'/MainLogs.txt',encoding='utf-8') as f:
	    info_record = f.readlines()
	    for record in info_record:
	        info_rec_dic = eval(record)
	        info_list.append(info_rec_dic)

	return info_list
   
def writeData(root,save):
	headers = ['LogType', 'Which', 'AlgoTime', 'TimeType', 'Detail']
	data_list = readerData(root)
	with open(save,'w',newline='',encoding='utf-8') as fv:
	    writer = csv.DictWriter(fv, headers)
	    writer.writeheader()
	    writer.writerows(data_list)

# 合并同一个文件夹下多个txt
def MergeTxt(filepath,outfile):
    if os.path.exists(filepath+outfile):
        os.remove(filepath+outfile)
    k = open(filepath+outfile, 'a+')
    for parent, dirnames, filenames in os.walk(filepath):
        for filepath in filenames:
            txtPath = os.path.join(parent, filepath)  # txtpath就是所有文件夹的路径
            f = open(txtPath)
            # 换行写入
            k.write(f.read())

    k.close()

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
    MergeTxt(root,'/MainLogs.txt')
    writeData(root, save)
    print("--done!--")