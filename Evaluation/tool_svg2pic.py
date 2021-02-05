
#conding=utf8  
import os
import sys
import cairosvg

def svg2pic(rootPath,savePath):

    print(os.getcwd()+rootPath.strip('.'))
    # 去除"."是因为传入的是"./data/"
    g = os.walk(os.getcwd()+rootPath.strip('.'))  

    for path,dir_list,file_list in g:  
        for file_name in file_list:  
            # print(os.path.join(path, file_name) )
            print(file_name)
            if os.path.splitext(file_name)[1] == '.svg':
                print(file_name)
                nameTmp = os.path.splitext(file_name)[0]
                # SVG转PNG
                # cairosvg.svg2png(url=file_name, write_to=nameTmp+".png")
                # # SVG转PDF
                cairosvg.svg2pdf(file_obj=open(rootPath+file_name, "rb"), write_to=savePath+nameTmp+".pdf")
                # # SVG转PS
                # cairosvg.svg2ps(bytestring=open(file_name).read().encode('utf-8'))


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
    svg2pic(root, save)
    print("--done!--")