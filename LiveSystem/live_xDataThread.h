#pragma once
#include <mutex>
#include <thread>
#include <list>
#include <live_xData.h>
class XDataThread
{
public:
	//�������б��С���б����ֵ������ɾ����ɵ�����
	int maxList = 150;
	XDataThread(){};
	~XDataThread(){};
	//���б��β����
	void Push(XData d)
	{
		// Print2File("XDataThread:void Push(XData d)");
		mutex.lock();
		//size������ǿ�
		if (datas.size() > maxList)
		{
			datas.front().Drop(); // ��һ��Ԫ��ɾ��data��size���� 
			datas.pop_front(); // ɾ��list��һ��Ԫ��,STL��api
		}
		datas.push_back(d);
		mutex.unlock();
		// Print2File("XDataThread:void Push(XData d) Finished");
	}
	//��ȡ�б�����������ݣ�����������Ҫ����XData.Drop����
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

	//�����̣߳����Ҫ��ֳQt��
	bool Start()
	{
		isExit = false;
		// ����û�뵽�÷�������һ���߳����
		// QThread::start(); ���Ҫ��ֳQt��
		return true;
	}

	//�˳��̲߳��ҵȴ��߳��˳��������������Ҫ��ֳQt��
	void Stop()
	{
		//�Ȱ��߳��˳�
		isExit = true;
		// wait();
	}


protected:
	//��Ž�������
	std::list<XData> datas;
	//���������б��С
	int dataCout = 0;
	//������� datas
	std::mutex mutex;
	//�����߳��˳�
	bool isExit = false;
	
private:

};

