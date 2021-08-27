#pragma once
#include <stdlib.h>
#include <string.h>
#include "util/util_log.h"
class XData
{
public:
	char *data = 0;
	int size = 0;
	long long pts = 0;
	XData(){};
  ~XData(){};
  
  //创建空间，并且复制data内容
  XData(char *data, int size,long long p=0)
  {
    this->data = new char[size];
    memcpy(this->data, data, size);
    this->size = size;
    this->pts = p;
  }
  
  void Drop()
  {
    // Print2File("void Drop()");
    if (data)
      delete data;
    data = 0;
    size = 0;
  }
};
