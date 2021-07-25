#pragma once
#include <mutex>
#include <thread>
#include <list>
#include <live_xData.h>
class XDataThread
{
public:
	//（缓冲列表大小）列表最大值，超出删除最旧的数据
	int maxList = 6;
	XDataThread(){};
	~XDataThread(){};
	//在列表结尾插入
	void Push(XData d)
	{
		// Print2File("XDataThread:void Push(XData d)");
		mutex.lock();
		//size大可能是坑
		if (datas.size() > maxList)
      {
        datas.front().Drop(); // 第一个元素删除data和size数据
        datas.pop_front(); // 删除list第一个元素,STL的api
      }
		datas.push_back(d);
		mutex.unlock();
		// Print2File("XDataThread:void Push(XData d) Finished");
	}
	//读取列表中最早的数据，返回数据需要调用XData.Drop清理
	XData Pop()
	{
		mutex.lock();
		if (datas.empty())
      {
        mutex.unlock();
        return XData();
      }
		XData d = datas.front();
		datas.pop_front();
		mutex.unlock();
		return d;
	}
  
	void Clear()
	{
		mutex.lock();
		while (!datas.empty())
      {
        datas.front().Drop();
        datas.pop_front();
      }
		mutex.unlock();
	}
  
	//启动线程，如果要移殖Qt用
	bool Start()
	{
		isExit = false;
		// 这里没想到好方法启动一个线程替代
		// QThread::start(); 如果要移殖Qt用
		return true;
	}
  
	//退出线程并且等待线程退出（阻塞），如果要移殖Qt用
	void Stop()
	{
		//先把线程退出
		isExit = true;
		// wait();
	}
  

protected:
	//存放交互数据
	std::list<XData> datas;
	//交互数据列表大小
	int dataCout = 0;
	//互斥访问 datas
	std::mutex mutex;
	//处理线程退出
	bool isExit = false;
  
private:
  
};
