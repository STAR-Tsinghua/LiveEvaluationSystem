#ifndef SODTP_JITTER_H
#define SODTP_JITTER_H

#include <fcntl.h>
#include <unistd.h>

#include <deque>
#include <vector>
#include <thread>
#include <semaphore.h>

#include <sodtp_util.h>
#include <sodtp_block.h>
#include <p_sodtp_decoder.h>



typedef struct JitterSample {
    uint32_t        ooo_count;		// the count of out-of-order packets
    uint32_t        empty_count;	// the times that the buffer is empty
    uint32_t        overflow_count; // the times that the buffer overflows

    double          jitter;
    double          max_jitter;

    uint64_t        prev_snd_ts;
    uint64_t        prev_rcv_ts;
    uint32_t        prev_interval_delta;

} JitterSample;


class SodtpJitter {
public:
    // enum STATE {
    //     INIT,       // for state only.
  
    //     NORMAL,     // for state and ret value.
    //     EMPTY,      // for state and ret value.

    //     CLOSING,    // for state only.
    //     CLOSED,     // for state only.
    // };
    static const uint32_t STATE_INIT        = 0x0001;   // state value for initing thread.
    static const uint32_t STATE_CLOSE       = 0x0002;   // state value for closing thread.

    static const uint32_t STATE_NORMAL      = 0x0100;   // ret value in push and pop.
    static const uint32_t STATE_SUCCESS     = 0x0100;   // ret value in push and pop.
    static const uint32_t STATE_BUFFERING   = 0x0200;   // ret value in pop.
    static const uint32_t STATE_OVERFLOW    = 0x0400;   // ret value in push.
    static const uint32_t STATE_EMPTY       = 0x0800;   // warning! not used!

    static const uint32_t STATE_SKIP        = 0x1000;   // ret value in pop. A skipped block.

    // Disorder limit for the first rebuffered block.
    // When rebuffering, the interval between the first block and last popped one might be too large.
    // In this case, we set (block_id - limit) as the new last_pop, thus any block before the last_pop
    // will be ignored.
    static const int32_t REBUFFER_DISORDER_LIMIT    = 10;

    static const int32_t REAL_TIME_LIMIT    = 300;      // default limit for real-time delay (ms).

    uint32_t            state;

    bool                real_time;              // flag of real-time mode.
    int32_t             _real_time_limit;       // limit for real-time delay.

    // currently not used.
    SodtpMetaData       meta_data;              // meta data for decoding.
    

    JitterSample        _sample;
    // uint32_t            receiver;            // use _thd instead.
    uint32_t            stream_id;

    Decoder             decoder;                // decoder of this stream.


    // Currently _nominal_depth is not really used.
    // And _buffer_max_depth > _nominal_depth.
    int32_t             _nominal_depth;         // nominal jitter depth (ms).
    int32_t             _buffer_max_depth;      // max jitter depth (ms).
    int32_t             _buffer_depth;          // current buffer depth of jitter (ms).
    int32_t             _buffering;             // flag for buffering blocks without pop.
    uint64_t            _buffering_ts;          // timestamp of buffering.

    uint32_t            _first_buf_seq;
    uint32_t            _last_buf_seq;
    uint32_t            _last_pop_seq;          // the id of last popped block.


    std::thread                     *_thd;

    /// We do not use semaphore here.
    /// @deprecated.
    sem_t                           *_sem;
    std::mutex                      _mutex;
    std::deque<SodtpBlockPtr>       _buffer;

    SodtpJitter(int32_t nominal_depth, sem_t *sem);
    SodtpJitter(int32_t nominal_depth, sem_t *sem, std::thread *thd);
    SodtpJitter(int32_t nominal_depth, sem_t *sem, uint32_t stream_id);
    SodtpJitter(int32_t nominal_depth, int32_t max_depth, sem_t *sem, uint32_t stream_id);
    ~SodtpJitter();

    int front(SodtpBlockPtr &block);

    int pop(SodtpBlockPtr &block);
    int push(SodtpBlockPtr block);
    void reset();
    void set_state(uint32_t state);
    void set_work_thread(std::thread *thd);
    void set_depth(int32_t nominal_depth, int32_t max_depth);
    void set_max_depth(int32_t max_depth);
    void set_meta_data(SodtpMetaData *meta);

    std::thread* get_work_thread() {return _thd;}
    int get_nominal_depth();
    void get_meta_data(SodtpMetaData *meta);



    // No mutex in private func().
    // You need to add mutex if needed when calling them.

    void _init(int32_t depth, int32_t max_depth);

    void _reset_sample();

    // update the jitter.
    // However, the block_ts is wrongly assigned. And we cannot calculate
    // the right jitter.
    void _update_jitter(SodtpBlockPtr &block);

    void _set_depth(int32_t nominal_depth, int32_t max_depth);

};

typedef std::shared_ptr<SodtpJitter>  SodtpJitterPtr;

class JitterBuffer
{
public:

    static const int32_t DEPTH_NOMINAL      = 30;  // ms 原来160
    static const int32_t DEPTH_MAXIMUM      = 50;  // ms 原来800


    std::vector<SodtpJitterPtr>     jptrs;

    // Modifying on jptrs should be locked by mutex.
    // Including write and read. Note that:
    // Write() is called by its inner-class functions.
    // Read() is called by outside functions (creating thread).
    std::mutex                      mtx;


    // Notification for creating a decoding thread.
    // We DO NOT need sem for closing the thread, instead, the 'FIN'
    // flag in block for the stream will notify the thread to stop.
    sem_t                           *sem;

    void push_back(SodtpStreamHeader *head, SodtpBlockPtr block);

    // It can only be called after the thread of jptrs[i] is stopped.
    void erase(int i);
    void erase(std::vector<SodtpJitterPtr>::iterator it);

    JitterBuffer() {
        fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
        sem_unlink("jitter_buffer");
        sem = sem_open("jitter_buffer", O_CREAT|O_EXCL, S_IRWXU, 0);
        if (!sem) {
            perror("fail to open semaphore.");
        }
        fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
        // sem_init(&sem, 0, 0);
    }

    ~JitterBuffer(){
        // for (auto ptr : jptrs) {
        //     delete ptr;
        // }
        jptrs.clear();
        fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
        sem_close(sem);
        fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
        sem_unlink("jitter_buffer");
        fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
    }
};

#endif // SODTP_JITTER_H
