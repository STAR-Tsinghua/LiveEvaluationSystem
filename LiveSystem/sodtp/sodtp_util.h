#ifndef SODTP_UTIL_H
#define SODTP_UTIL_H

#include <chrono>
#include <mutex>
#include <sys/time.h>

namespace   clocks = std::chrono;
typedef clocks::steady_clock                    stdclock;
typedef clocks::steady_clock::time_point        timepoint;

typedef std::lock_guard<std::mutex>             scoped_lock;
typedef std::lock_guard<std::recursive_mutex>   rscoped_lock;



static uint64_t current_mtime() {
    struct  timeval tv;
    gettimeofday(&tv, NULL);
    // return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return (tv.tv_sec * 1000) + ((tv.tv_usec + 999) / 1000);
}

static inline bool before(uint32_t seq1, uint32_t seq2)
{
    return (int32_t)(seq1 - seq2) < 0;
}

static inline bool before(uint64_t seq1, uint64_t seq2) {
    return (int64_t)(seq1 - seq2) < 0;
}

#define after(seq2, seq1)   before(seq1, seq2)


#endif // SODTP_UTIL_H