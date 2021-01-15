#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <stdio.h>

#include <util_log.h>

int main(int argc, char *argv[]) {
  Print2File("main");
  return 0;
}

// 测试相机代码
// using namespace cv;
 
// int main(int argc, char** argv)
// {   
//   VideoCapture capture(0);
//   while(1){
//     Mat frame;
//     capture >> frame;
//     printf("Camera capture....\n");
//     imshow("读取视频",frame);
//     waitKey(30);
//   }
  
//   return 0;
// }
