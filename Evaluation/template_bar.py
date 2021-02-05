# coding=UTF-8<code>
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
plt.rcParams["font.sans-serif"]=["SimHei"] #正常显示中文
plt.rcParams["axes.unicode_minus"]=False #正常显示负号
 
# d=pd.read_excel("E:Pythonprojectsdatadata100.xlsx",header=None)
# d=d[0]
# d=list(d)
 
ages=range(11)
count=[3,6,7,11,13,18,15,11,7,5,4]
plt.bar(ages,count, label="graph 1")
# params
# x: 条形图x轴
# y：条形图的高度
# width：条形图的宽度 默认是0.8
# bottom：条形底部的y坐标值 默认是0
# align：center / edge 条形图是否以x轴坐标为中心点或者是以x轴坐标为边缘
plt.legend()
plt.xlabel("ages")
plt.ylabel("count")
plt.title(u"测试例子――条形图")
 
for i in range(11):
  plt.text(i,count[i]+0.1,"%s"%count[i],va="center")
 
plt.show()