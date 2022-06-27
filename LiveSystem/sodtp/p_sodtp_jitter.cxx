#include <stdio.h>
#include <ev.h>
#include <util_log.h>
#include "p_sodtp_jitter.h"



SodtpJitter::SodtpJitter(int32_t nominal_depth, sem_t *sem) {
    _sem = sem;
    _thd = NULL;
    _init(nominal_depth, 0);

    state = STATE_INIT;
    real_time = true;
    _real_time_limit = REAL_TIME_LIMIT;
    memset(&meta_data, 0, sizeof(meta_data));
}

SodtpJitter::SodtpJitter(int32_t nominal_depth, sem_t *sem, std::thread *thd) {
    _sem = sem;
    _thd = thd;
    _init(nominal_depth, 0);

    state = STATE_INIT;
    real_time = true;
    _real_time_limit = REAL_TIME_LIMIT;
    memset(&meta_data, 0, sizeof(meta_data));
}

SodtpJitter::SodtpJitter(int32_t nominal_depth, sem_t *sem, uint32_t stream_id) {
    _sem = sem;
    _thd = NULL;
    _init(nominal_depth, 0);

    this->stream_id = stream_id;

    state = STATE_INIT;
    real_time = true;
    _real_time_limit = REAL_TIME_LIMIT;
    memset(&meta_data, 0, sizeof(meta_data));
}

SodtpJitter::SodtpJitter(int32_t nominal_depth, int32_t max_depth, sem_t *sem, uint32_t stream_id) {
    _sem = sem;
    _thd = NULL;
    _init(nominal_depth, max_depth);

    this->stream_id = stream_id;

    state = STATE_INIT;
    real_time = true;
    _real_time_limit = REAL_TIME_LIMIT;
    memset(&meta_data, 0, sizeof(meta_data));
}

SodtpJitter::~SodtpJitter() {
    scoped_lock lock(_mutex);

    _buffer.clear();
}

void SodtpJitter::_reset_sample() {
    _sample.ooo_count = 0;
    _sample.empty_count = 0;
    _sample.overflow_count = 0;

    _sample.jitter = 0.0;
    _sample.max_jitter = 0.0;

    _sample.prev_snd_ts = 0;
    _sample.prev_rcv_ts = 0;
    _sample.prev_interval_delta = 0;
}

void SodtpJitter::_set_depth(int32_t nominal_depth, int32_t max_depth) {
    _nominal_depth = nominal_depth;
    if (max_depth >= nominal_depth) {
        _buffer_max_depth = max_depth;
    }
    else {
        _buffer_max_depth = nominal_depth * 2;
    }
}

void SodtpJitter::set_depth(int32_t nominal_depth, int32_t max_depth) {
    scoped_lock lock(_mutex);
    _set_depth(nominal_depth, max_depth);
}

void SodtpJitter::set_max_depth(int32_t max_depth) {
    scoped_lock lock(_mutex);
    _buffer_max_depth = max_depth;
}

void SodtpJitter::set_meta_data(SodtpMetaData *meta) {
    scoped_lock lock(_mutex);
    if (meta) {
        memcpy(&meta_data, meta, sizeof(meta_data));
    }
}

void SodtpJitter::set_state(uint32_t state) {
    scoped_lock lock(_mutex);
    this->state = state;
}

void SodtpJitter::set_work_thread(std::thread *thd) {
    // Currently there is no other value assignment.
    // So, we do not use a lock here.
    _thd = thd;
}

int SodtpJitter::get_nominal_depth() {
    scoped_lock lock(_mutex);
    return _nominal_depth;
}

void SodtpJitter::get_meta_data(SodtpMetaData *meta) {
    scoped_lock lock(_mutex);
    if (meta) {
        memcpy(meta, &meta_data, sizeof(meta_data));
    }
}

void SodtpJitter::_init(int32_t nominal_depth, int32_t max_depth) {
    if (!_buffer.empty()) {
        _buffer.clear();
    }

    _buffering = true;
    _buffer_depth = 0;
    _buffering_ts = 0;

    _first_buf_seq = 0;
    _last_buf_seq = 0;
    _last_pop_seq = -1;
    // _last_pop_seq = -20;

    _set_depth(nominal_depth, max_depth);
    _reset_sample();
}

void SodtpJitter::reset() {
    scoped_lock lock(_mutex);
    _init(_nominal_depth, _buffer_max_depth);
}

int SodtpJitter::front(SodtpBlockPtr &block) {
    int ret = STATE_BUFFERING;
    block = NULL;

    scoped_lock lock(_mutex);

    if (!_buffer.empty()) {
        block = _buffer.front();
        ret = STATE_NORMAL;
    }

    return ret;
}

int SodtpJitter::pop(SodtpBlockPtr &block) {
    int ret = STATE_NORMAL;
    block = NULL;


    // Need mutex here.
    scoped_lock lock(_mutex);

    // ---
    // Check the buffering state.
    if (_buffer.empty()) {
        if (!_buffering) {
            _buffering = true;
        }

        // Action on the statistical sample
        _sample.empty_count ++;
    }
    else if (_buffering) {
        int buffer_time = current_mtime() - _buffering_ts;
        if ((buffer_time >= _nominal_depth) || (_buffer_depth  >= _nominal_depth)) {
            _buffering = false;
            _buffering_ts = 0;
        }
    }

    if (_buffering) {
        return STATE_BUFFERING;
    }
    // ---





    // assert(!_buffer.empty());
    if ((_first_buf_seq - _last_pop_seq) == 1) {
        // Pop a block
        block = _buffer.front();
        _buffer.pop_front();
        _buffer_depth -= block->duration;


        _last_pop_seq = block->block_id;


        if (_buffer.empty()) {
            _first_buf_seq = _last_buf_seq = _last_pop_seq + 1;
        }
        else {
            _first_buf_seq = _buffer.front()->block_id;
        }

        ret = STATE_NORMAL;
    }
    else {
        _last_pop_seq ++;
        ret = STATE_SKIP;
        fprintf(stderr, "last_pop %u,\t last_buf %u,\t first_buf %u\n",
            _last_pop_seq, _last_buf_seq, _first_buf_seq);
    }




    // // Pop a block
    // ptr = _buffer.front();
    // _buffer.pop_front();
    // ///
    // ///
    // _last_pop_seq = ptr->block_id;
    // ///
    // ///


    // // if (_buffer.size() < 2) {
    // //     _buffer_depth = 0;
    // // }
    // // else {
    // //     _buffer_depth = _buffer.back()->block_ts - _buffer.front()->block_ts;
    // // }
    // _buffer_depth -= ptr->duration;


    // if last pkt, set state = close;
    if (block && block->last_one) {
        state = STATE_CLOSE;
        fprintf(stderr, "state = close now!\n");
    }

    return ret;
}

int SodtpJitter::push(SodtpBlockPtr block) {

    int ret = 0;

    if (block && block->data) {
        // Need mutex here.
        scoped_lock lock(_mutex);

        // drop stale blocks in real-time mode.
        if (real_time && (block->block_ts + _real_time_limit < current_mtime()) && block->block_id != 0 && block->block_id != 1) {
            fprintf(stderr, "ignore non real-time block, stream %d,\t block %d\n",
                block->stream_id, block->block_id);
            return ret;
        }

        if (_buffer_depth > _buffer_max_depth) {
            SodtpBlockPtr ptr = _buffer.front();
            fprintf(stderr, "pop stale buffer, stream %d,\t block %d\n",
                ptr->stream_id, ptr->block_id);
            _buffer_depth -= ptr->duration;
            _buffer.pop_front();
            _sample.overflow_count++;
            // ptr.reset();
            // printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
        }

        fprintf(stderr, "stream %d,\t block %d,\t size %d\n", block->stream_id, block->block_id, block->size);


        // When buffering starts, write down the timestamp.
        if (_buffering && (_buffering_ts == 0)) {
            _buffering_ts = current_mtime();
        }

        _update_jitter(block);


        // When buffer is popped to empty:
        // 1) _last_pop_seq is the latest block;
        // 2) _first_buf_seq will be set to _last_pop_seq;
        // 3) _last_buf_seq remains the latest block, i.e. _last_pop_seq;
        if (
            // empty buffer and a newer block.
            (_buffer.empty() && after(block->block_id, _last_pop_seq)) ||
            // higher id than buffered blocks
            after(block->block_id, _last_buf_seq)) {

            _buffer.push_back(block);
            _last_buf_seq = block->block_id;
            _buffer_depth += block->duration;

            if (_buffer.size() == 1) {
                _first_buf_seq = _last_buf_seq;

                // Attention!
                // Attention! This is very important!
                // When re-buffered, we reset _last_pop = _first_buf - K.
                // (K-1) is the number of out-of-order packets we can tolerate.
                // Because before re-buffered, there might be many lost packets,
                // we choose to restart the jitter cycling.
                int32_t K = REBUFFER_DISORDER_LIMIT;  // e.g. 1, 2, 10, 20
                // K = 1 means we do not tolerate new ooo blocks compared to the first
                // pushed block when re-buffering.
                if ((int32_t)(_first_buf_seq - _last_pop_seq) > K) {
                    _last_pop_seq = _first_buf_seq - K;
                }
            }
        }
        // out-of-order blocks.
        else {
            _sample.ooo_count ++;

            ///
            /// Warning!!!
            /// Here, we ignore any block that is older than already popped blocks.
            ///
            ///
            if (after(block->block_id, _last_pop_seq)) {
                if (before(block->block_id, _first_buf_seq)) {
                    _buffer.push_front(block);
                    _first_buf_seq = block->block_id;
                    _buffer_depth += block->duration;
                }
                else {
                    std::deque<SodtpBlockPtr>::iterator it;
                    for (it = _buffer.begin(); it != _buffer.end(); ++it) {
                        if (before(block->block_id, (*it)->block_id)) {
                            _buffer.insert(it, block);
                            _buffer_depth += block->duration;
                            break;
                        }
                    }
                }
            }
            // Ignore this stale block.
            else {
                fprintf(stderr, "ignore this stale block in stream %d,\t block %d\n",
                    stream_id, block->block_id);
            }
        }



        /*
        if (_buffer.empty()) {
            _buffer.push_back(block);
            printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
        }
        // Case: out-of-order block
        // Find the right place to insert this block.
        // This method might be not efficient.
        else if (block->block_id < _buffer.back()->block_id) {
            std::deque<SodtpBlockPtr>::iterator it;
            for (it = _buffer.begin(); it != _buffer.end(); ++it) {
                if (block->block_id < (*it)->block_id) {
                    printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
                    _buffer.insert(it, block);
                    printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
                    break;
                }
            }
        }
        // Case: in-order block
        // Insert at the end.
        else {
            _buffer.push_back(block);
            printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
        }

        // Update the buffer depth of jitter
        // _buffer_depth = _buffer.back()->block_ts - _buffer.front()->block_ts;
        _buffer_depth += block->duration;
        // printf("block duration %d,\t total depth %d\n", block->duration, _buffer_depth);
        */

    }

    return ret;
}


void SodtpJitter::_update_jitter(SodtpBlockPtr &block) {
    uint64_t now = current_mtime();

    // Arrival time of the block in the time line of this stream.
    // E.g. a block arrives 1s after previous block with sending time T.
    // Then this block's arrival time T+1, which is usually different from
    // current time.
    uint32_t arrival;

    // The arrival interval (ms) between each block.
    int arrival_interval = now - _sample.prev_rcv_ts;

    // The sending interval (ms) between each block.
    int sending_interval = block->block_ts - _sample.prev_snd_ts;

    // The delta between arrival interval and sending interval.
    // E.g. application sends a block per 100ms, while some blocks arrive
    // with around 100ms interval but not precisely 100ms, such as 102ms.
    // Then for this block, its delta = 102 - 100 = 2ms.
    uint32_t interval_delta;

    // Jitter in current round.
    uint32_t jitter_now;


    interval_delta = arrival_interval - sending_interval;
    jitter_now = interval_delta - _sample.prev_interval_delta;

    if (jitter_now < 0) {
        jitter_now = -jitter_now;
    }

    _sample.prev_rcv_ts = now;
    _sample.prev_snd_ts = block->block_ts;
    _sample.prev_interval_delta = interval_delta;

    _sample.jitter += (1./16.) * ((double)jitter_now - _sample.jitter);

    if (_sample.max_jitter < _sample.jitter) {
        _sample.max_jitter = _sample.jitter;
    }
}

void JitterBuffer::push_back(SodtpStreamHeader *head, SodtpBlockPtr block) {
    // Search existing stream.
    for (auto it : jptrs) {
        if (it->stream_id == head->stream_id) {
            // Print2File("if (it->stream_id == head->stream_id)");
            // if (head->flag & HEADER_FLAG_META) {
            //     it->set_meta_data((SodtpMetaData*)(block->data));
            //     printf("meta data: stream %d,\twidth %d,\theight %d\n",
            //         it->stream_id, it->meta_data.width, it->meta_data.height);
            // }
            // else {
            //     it->push(block);
            // }
            it->push(block);
            
            return;
        }
    }
    // Print2File("SodtpJitterPtr ptr(new SodtpJitter(DEPTH_NOMINAL, DEPTH_MAXIMUM, sem, head->stream_id));");
    // Else this block belongs to a new stream.
    SodtpJitterPtr ptr(new SodtpJitter(DEPTH_NOMINAL, DEPTH_MAXIMUM, sem, head->stream_id));
    // if (head->flag & HEADER_FLAG_META) {
    //     ptr->set_meta_data((SodtpMetaData*)(block->data));
    //     printf("meta data: stream %d,\twidth %d,\theight %d\n",
    //         ptr->stream_id, ptr->meta_data.width, ptr->meta_data.height);
    // }
    // else {
    //     ptr->push(block);
    // }
    ptr->push(block); //int SodtpJitter::push(SodtpBlockPtr block) {
    // Print2File("jptrs.push_back(ptr);!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    scoped_lock lock(mtx);
    jptrs.push_back(ptr); // jptrs 是vector
    fprintf(stderr, "jptrs num %lu\n", jptrs.size());

    // Notification for creating new thread.
    sem_post(sem);
    ev_feed_signal(SIGUSR1);
}

void JitterBuffer::erase(int i) {
    scoped_lock lock(mtx);
    jptrs.erase(jptrs.begin() + i);
    timeFramePlayer.evalTimeStamp("pJitter_erase_1","p","FrameErase");
}

void JitterBuffer::erase(std::vector<SodtpJitterPtr>::iterator it) {
    scoped_lock lock(mtx);
    jptrs.erase(it);
    timeFramePlayer.evalTimeStamp("pJitter_erase_2","p","FrameErase");
}








