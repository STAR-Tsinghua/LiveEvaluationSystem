#pragma once
#include <stdlib.h>
#include <string.h>
#include <util_log.h>
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
        // Print2File("cxx : XData::XData(char *data, int size,long long p)");
        this->data = new char[size];
        memcpy(this->data, data, size);
        this->size = size;
        this->pts = p;
        // Print2File("XData this->pts:"+std::to_string(this->pts));
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
