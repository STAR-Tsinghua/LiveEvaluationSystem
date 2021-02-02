# coding=UTF-8<code>
import matplotlib.pyplot as plt
#绘制水平条形图
rect=plt.barh(bottom=(0,1,2),height=0.35,width=(40,30,50),align="center")
#显示汉字，前面加u，代表使用unicode
#y轴标记
plt.ylabel('function')

#x轴标记
plt.xlabel('time')

#设置x轴可读显示
plt.yticks((0,1,2),('func1','func2','func3'))

#显示柱状图信息
plt.legend((rect,),("time ms",))
#显示绘制图片
plt.show()
