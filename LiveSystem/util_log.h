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

#include <chrono>
#include <type_traits>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <unordered_map>

#define msleep(n) usleep(n*1000)
#define LEARN_LOG "logs/learnLog.txt" // 文件名称
#define DEBUG_LOG "logs/debugLog.txt" // 文件名称

static bool showDebugLog = false;

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

static void Print2FileInfo(std::string inputStr)
{
    if(!showDebugLog){
        return;
    }
    Print2File("[........Info........:"+inputStr+"]");
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

//微秒
static inline uint64_t getCurrentMicroseconds(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

//毫秒
static inline uint64_t getCurrentMillisecond(){
    return getCurrentMicroseconds()/1000;
}

namespace Tools {

    namespace Time {

        using ns = std::chrono::nanoseconds;
        using us = std::chrono::microseconds;
        using ms = std::chrono::milliseconds;
        using s = std::chrono::seconds;
        using m = std::chrono::minutes;
        using h = std::chrono::hours;

        template <typename DurationType> class AlgoTime;
        using AlgoTimeNs = AlgoTime<ns>;
        using AlgoTimeUs = AlgoTime<us>;//微秒
        using AlgoTimeMs = AlgoTime<ms>;//毫秒
        using AlgoTimeS = AlgoTime<s>;//秒
        using AlgoTimeM = AlgoTime<m>;
        using AlgoTimeH = AlgoTime<h>;

        /**
         * 用于计算代码运行的时间
         * @tparam DurationType 类似于时间单位，默认以“毫秒”为单位
         */
        template <typename DurationType = ms>
        class AlgoTime {
        public:
            void start() {
                start_  = std::chrono::steady_clock::now();
            }

            void startAndWrite(std::string inStr){
                // std::chrono::system_clock::now().time_since_epoch().count();//功能：获取系统时间戳，单位微秒(microsecond)
                // std::chrono::steady_clock::now().time_since_epoch().count();//功能：获取系统时间戳，单位纳秒(nanosecond)
                // gettimeofday //功能：获取系统时间戳，单位微秒(microsecond)
                // Print2File("[LogType:start:"+ inStr +";*****:"+ std::to_string(getCurrentMicroseconds()) +" ; "+ getTimeType()+"]");
                // 注意下面和上面的时间戳不一样，因为上面只用来对比server和player时间差
                start_  = std::chrono::steady_clock::now();
            }

            typename DurationType::rep elapsed() const {
                auto elapsed = std::chrono::duration_cast<DurationType>(
                        std::chrono::steady_clock::now() - start_);
                return elapsed.count();
            }

            void printElapsed() {
                auto time = elapsed();
                if (std::is_same<DurationType, ns>::value) {
                    std::cout << std::endl << "[AlgoTime: " << time << " ns]" << std::endl;
                } else if (std::is_same<DurationType, us>::value) {
                    std::cout << std::endl << "[AlgoTime: " << time << " us]" << std::endl;
                } else if (std::is_same<DurationType, ms>::value) {
                    std::cout << std::endl << "[AlgoTime: " << time << " ms]" << std::endl;
                } else if (std::is_same<DurationType, s>::value) {
                    std::cout << std::endl << "[AlgoTime: " << time << " s]" << std::endl;
                } else if (std::is_same<DurationType, m>::value) {
                    std::cout << std::endl << "[AlgoTime: " << time << " m]" << std::endl;
                } else if (std::is_same<DurationType, h>::value) {
                    std::cout << std::endl << "[AlgoTime: " << time << " h]" << std::endl;
                }
            }

            void evalTime(std::string which , std::string detail){
                //当前从开始到现在的时间戳
                if(map_str.find(detail) != map_str.end()){
                    return;
                }else{
                    map_str[detail] = true;
                }
                auto time = elapsed();
                int timeShow = time-lastTime;
                lastTime = time;
                Print2File("{'LogType': 'Latency', 'Which': '"+which+"', 'AlgoTime': '"+ std::to_string(timeShow)+"', 'TimeType': '"+getTimeType()+"', 'Detail': '"+detail+"'}");
            }

            void evalTimeStamp(std::string which , std::string detail){
                //当前从开始到现在的时间戳
                Print2File("{'LogType': 'FrameTime', 'Which': '"+which+"', 'AlgoTime': '"+ std::to_string(getCurrentMillisecond())+"', 'TimeType': '"+getTimeType()+"', 'Detail': '"+detail+"'}");
            }

        private:
            std::chrono::steady_clock::time_point start_;
            std::unordered_map<std::string, bool> map_str;
            int lastTime;
            std::string getTimeType(){
                if (std::is_same<DurationType, ns>::value) {
                    return "ns";
                } else if (std::is_same<DurationType, us>::value) {
                    return "us";
                } else if (std::is_same<DurationType, ms>::value) {
                    return "ms";
                } else if (std::is_same<DurationType, s>::value) {
                    return "s";
                } else if (std::is_same<DurationType, m>::value) {
                    return "m";
                } else if (std::is_same<DurationType, h>::value) {
                    return "h";
                }
            }
        };

    } // namespace Time

} // namespace Tools

static Tools::Time::AlgoTimeMs timeMainServer;
static Tools::Time::AlgoTimeMs timeMainPlayer;

static Tools::Time::AlgoTimeMs timeFrameServer;
static Tools::Time::AlgoTimeMs timeFramePlayer;

#endif // UTIL_LOG_H