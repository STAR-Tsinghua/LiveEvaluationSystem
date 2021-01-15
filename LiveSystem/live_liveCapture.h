#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <live_xData.h>
#include <util_log.h>
#include <live_xDataThread.h>
using namespace cv;

class LiveCapture: public XDataThread
{
public:	
	VideoCapture cam;
	int width = 0;
	int height = 0;
	int fps = 0;
	bool isExit = false;
	~LiveCapture(){};
	LiveCapture(){};
	LiveCapture(const LiveCapture&){};
	LiveCapture& operator=(const LiveCapture&);
	// 原本设计单例，改
	// static LiveCapture& getInstance(){
	// 	Print2File("LiveCapture& getInstance()");
	// 	static LiveCapture lc;
	// 	return lc;
	// }
	
	//线程调用只能调用静态函数
	void run()
	{
		// Print2File("LiveCapture void run()");
		Mat frame;
		while (!isExit)
		{
			// Print2File("while (!isExit)");
			if (!cam.read(frame))
			{
				Print2File("!cam.read(frame)");
				msleep(1);
				continue;
			}
			if (frame.empty())
			{
				Print2File("frame.empty()");
				msleep(1);
				continue;
			}
			XData d((char*)frame.data, frame.cols*frame.rows*frame.elemSize(),GetCurTime());
			Push(d);
		}
	}

	bool Init(int camIndex = 0)
	{
		// Print2File("bool Init() cam.open(0);");
		cam.open(camIndex);
		if (!cam.isOpened())
		{
			// Print2File("bool Init() inside : cam open failed!");
			return false;
		}
		// Print2File("bool Init() inside : cam open success");
		width = cam.get(CAP_PROP_FRAME_WIDTH);
		height = cam.get(CAP_PROP_FRAME_HEIGHT);
		fps = cam.get(CAP_PROP_FPS);
		Print2File("Init cam.get(CAP_PROP_FRAME_WIDTH): "+std::to_string(width));
		Print2File("Init cam.get(CAP_PROP_FRAME_HEIGHT): "+std::to_string(height));
		Print2File("Init cam.get(CAP_PROP_FPS): "+std::to_string(fps)); //log显示mac电脑为12帧
		if (fps == 0) fps = 25;
		return true;
	}
	bool Init(const char *url)
	{
		cam.open(url);
		if (!cam.isOpened())
		{
			// cout << "cam open failed!" << endl;
			return false;
		}
		// cout << url << " cam open success" << endl;
		width = cam.get(CAP_PROP_FRAME_WIDTH);
		height = cam.get(CAP_PROP_FRAME_HEIGHT);
		fps = cam.get(CAP_PROP_FPS);
		if (fps == 0) fps = 25;
		return true;
	}
	void Stop()
	{
		if (cam.isOpened())
		{
			cam.release();
		}
	}
private:

};