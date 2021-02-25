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
	int buffered_RGB;
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
		//设置buffered_RGB大小，因为多线程，在这里设置
		buffered_RGB_MAX = 1;
		timeFrameServer.start();
		Mat frame;
		while (!isExit)
		{
			if(buffered_RGB>buffered_RGB_MAX){
				// 节约性能
				// msleep(1);
				continue;
			}
			timeFrameServer.evalTimeStamp("start_CatchFrame","s","FrameTime");
			if (!cam.read(frame))
			{
				msleep(1);
				continue;
			}
			if (frame.empty())
			{
				Print2File("frame.empty()");
				msleep(1);
				continue;
			}
			++buffered_RGB;
			XData d((char*)frame.data, frame.cols*frame.rows*frame.elemSize(),GetCurTime());
			timeFrameServer.evalTimeStamp("RGB_Buffer_Count","s",std::to_string(buffered_RGB));
			timeFrameServer.evalTimeStamp("RGB_Push","s","FrameTime");
			Push(d);
		}
	}

	bool Init(int camIndex = 0)
	{
		cam.open(camIndex);
		if (!cam.isOpened())
		{
			return false;
		}
		width = cam.get(CAP_PROP_FRAME_WIDTH);
		height = cam.get(CAP_PROP_FRAME_HEIGHT);
		fps = cam.get(CAP_PROP_FPS);
		// Print2FileInfo("Init cam.get(CAP_PROP_FRAME_WIDTH): "+std::to_string(width));
		// Print2FileInfo("Init cam.get(CAP_PROP_FRAME_HEIGHT): "+std::to_string(height));
		Print2FileInfo("Init cam.get(CAP_PROP_FPS): "+std::to_string(fps)); //log显示mac电脑为30帧
		timeMainServer.evalTime("s","cam.get(CAP_PROP_FPS) Before : "+std::to_string(fps));
		if (fps == 0) fps = 25;

		//强制设置帧率,设置30，实际上根据摄像头只有30,待修复提高
		cam.set(CV_CAP_PROP_FPS,25);
		double fps_after = cam.get(CAP_PROP_FPS);
		timeMainServer.evalTime("s","cam.get(CAP_PROP_FPS) After Set : "+std::to_string(fps_after));
		return true;
	}
	// bool Init(const char *url)
	// {
	// 	cam.open(url);
	// 	if (!cam.isOpened())
	// 	{
	// 		return false;
	// 	}
	// 	width = cam.get(CAP_PROP_FRAME_WIDTH);
	// 	height = cam.get(CAP_PROP_FRAME_HEIGHT);
	// 	fps = cam.get(CAP_PROP_FPS);
	// 	if (fps == 0) fps = 25;
	// 	return true;
	// }
	void Stop()
	{
		if (cam.isOpened())
		{
			cam.release();
		}
	}
private:
	int buffered_RGB_MAX;
};