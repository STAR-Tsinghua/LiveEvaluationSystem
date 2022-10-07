#ifndef _HELPER_H_
#define _HELPER_H_
#include <stdint.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

uint64_t
getCurrentTime()  //直接调用这个函数就行了，返回值最好是int64_t，uint64_t应该也可以
{
    struct timeval tv;
    gettimeofday(&tv, NULL);  //该函数在sys/time.h头文件中
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

uint64_t
getCurrentTime_mic()  //直接调用这个函数就行了，返回值最好是int64_t，uint64_t应该也可以
{
    struct timeval tv;
    gettimeofday(&tv, NULL);  //该函数在sys/time.h头文件中
    return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}

#define ERROR 0
#define WARN 1
#define INFO 2
#define DEBUG 3
#define TRACE 4

static int LOG_LEVEL = 2;
static int LOG_COLOR = 0;

#define HELPER_LOG stdout
#define HELPER_OUT stdout

// log helper macro, no need to use \n
#define LOG(level, ...)                                                 \
  do                                                                    \
  {                                                                     \
    switch (level)                                                      \
    {                                                                   \
    case ERROR:                                                         \
      LOG_COLOR ? fprintf(HELPER_LOG, "\x1b[31m[ERROR] %s: ", __func__) \
                : fprintf(HELPER_LOG, "[ERROR] %s: ", __func__);        \
      break;                                                            \
    case WARN:                                                          \
      LOG_COLOR ? fprintf(HELPER_LOG, "\x1b[33m[WARN ] %s: ", __func__) \
                : fprintf(HELPER_LOG, "[WARN ] %s: ", __func__);        \
      break;                                                            \
    case INFO:                                                          \
      LOG_COLOR ? fprintf(HELPER_LOG, "\x1b[34m[INFO ] %s: ", __func__) \
                : fprintf(HELPER_LOG, "[INFO ] %s: ", __func__);        \
      break;                                                            \
    case DEBUG:                                                         \
      LOG_COLOR ? fprintf(HELPER_LOG, "\x1b[96m[DEBUG] %s: ", __func__) \
                : fprintf(HELPER_LOG, "[DEBUG] %s: ", __func__);        \
      break;                                                            \
    case TRACE:                                                         \
      LOG_COLOR ? fprintf(HELPER_LOG, "\x1b[90m[TRACE] %s: ", __func__) \
                : fprintf(HELPER_LOG, "[TRACE] %s: ", __func__);        \
      break;                                                            \
    }                                                                   \
    fprintf(HELPER_LOG, __VA_ARGS__);                                   \
    LOG_COLOR ? fprintf(HELPER_LOG, "\x1b[0m\n")                        \
              : fprintf(HELPER_LOG, "\n");                              \
    fflush(NULL);                                                       \
  } while (0)

#define log_error(...)       \
  if (LOG_LEVEL > ERROR)     \
  {                          \
    LOG(ERROR, __VA_ARGS__); \
  }                          \
  else

#define log_warn(...)       \
  if (LOG_LEVEL > WARN)     \
  {                         \
    LOG(WARN, __VA_ARGS__); \
  }                         \
  else

#define log_info(...)       \
  if (LOG_LEVEL > INFO)     \
  {                         \
    LOG(INFO, __VA_ARGS__); \
  }                         \
  else

#define log_debug(...)       \
  if (LOG_LEVEL > DEBUG)     \
  {                          \
    LOG(DEBUG, __VA_ARGS__); \
  }                          \
  else

#define log_trace(...)       \
  if (LOG_LEVEL > TRACE)     \
  {                          \
    LOG(TRACE, __VA_ARGS__); \
  }                          \
  else

// write what we cared into file
#define dump_file(...)                \
  do                                  \
  {                                   \
    fprintf(HELPER_OUT, __VA_ARGS__); \
    fflush(NULL);                     \
  } while (0)

// static void debug_log(const char *line, void *argp)
// {
//   // log_debug("%s", line);
// }
#endif // _HELPER_H_