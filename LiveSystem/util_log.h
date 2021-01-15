#ifndef UTIL_LOG_H
#define UTIL_LOG_H
extern "C"
{
// 这里一下子加入很多，需要简化include
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>
#include <libavutil/timestamp.h>
}
#include <fstream>
#include <strstream>
#include <sstream>
#include <string>

#define msleep(n) usleep(n*1000)
#define LEARN_LOG "logs/learnLog.txt" // 文件名称
#define DEBUG_LOG "logs/debugLog.txt" // 文件名称

struct buffer_data_write {
	uint8_t *buf;
    size_t size;
    uint8_t *ptr;
    size_t room; ///< size left in the buffer
};

// 此处为同步日志
// TODO 异步日志参考如下网址:
// https://github.com/LeechanX/Ring-Log
static void Print2File(std::string inputStr)
{
    std::fstream f;
	//追加写入,在原来基础上加了ios::app 
	f.open(LEARN_LOG,std::ios::out|std::ios::app);
	//输入你想写入的内容 
	f<<inputStr<<std::endl;
	f.close();
	return;
}

static void Print2FileDebug(std::string inputStr)
{
    std::fstream f;
	//追加写入,在原来基础上加了ios::app 
	f.open(DEBUG_LOG,std::ios::out|std::ios::app);
	//输入你想写入的内容 
	f<<inputStr<<std::endl;
	f.close();
	return;
}

//获取当前时间戳（微秒）
static long long GetCurTime()
{
	return av_gettime();
}


static std::string IntToString(int n)
{
    std::string result;
    std::strstream ss;
    ss <<  n;
    ss >> result;
    return result;
}
 
static std::string lltoString(long long t)
{
    std::string result;
    std::strstream ss;
    ss <<  t;
    ss >> result;
    return result;
}
static std::wstring IntToWstring(unsigned int i)
{
	std::wstringstream ss;
	ss << i;
	return ss.str();
}
static int lhs_write_packet(void *opaque, uint8_t *buf, int buf_size)
{
	struct buffer_data_write *bd = (struct buffer_data_write *)opaque;
	while (buf_size > bd->room) {
		int64_t offset = bd->ptr - bd->buf;
		// uint8_t *bufferPtr = static_cast<uint8_t *>(malloc( sizeof(uint8_t)))
		void* tmpPtr = av_realloc_f(bd->buf, 2, bd->size);
		bd->buf = static_cast<uint8_t *>(malloc( sizeof(tmpPtr)));
		if (!bd->buf)
			return AVERROR(ENOMEM);
		bd->size *= 2;
		bd->ptr = bd->buf + offset;
		bd->room = bd->size - offset;
	}
	/* copy buffer data to buffer_data buffer */
	memcpy(bd->ptr, buf, buf_size);
	bd->ptr  += buf_size;
	bd->room -= buf_size;
	return buf_size;
}
// static double r2d(AVRational r)
// {
// 	return r.den == 0 ? 0:(double)r.num / (double)r.den;
// }


#endif // UTIL_LOG_H