#!/usr/bin/env python3
# -*- coding:utf8 -*-
 
import os
import shutil
import sys
 
def copyFiles(root,save):
    if not os.path.exists(save):
        os.makedirs(save)

    if os.path.exists(root):
        # root 所指的是当前正在遍历的这个文件夹的本身的地址
        # dirs 是一个 list，内容是该文件夹中所有的目录的名字(不包括子目录)
        # files 同样是 list, 内容是该文件夹中所有的文件(不包括子目录)
        for root, dirs, files in os.walk(root):
            for file in files:
                src_file = os.path.join(root, file)
                shutil.copy(src_file, save)
                print(src_file)
    
print('copy files finished!')

if __name__ == "__main__":

    argv = sys.argv
    argc = len(argv)

    if argc > 2:
        root1 = argv[1]
        root2 = argv[2]
        save1 = argv[3]
        save2 = argv[4]
    else:
        print("please input root path of logs and csv name.")
        os._exit(0)

    print("root1 path: " + root1)
    print("save1 name: " + save1)
    print("----------\n")
    print("root2 path: " + root2)
    print("save2 name: " + save2)
    print("----------\n")
    print("=============== start run one =================")
    copyFiles(root1, save1)
    copyFiles(root2, save2)
    print("--done!--")