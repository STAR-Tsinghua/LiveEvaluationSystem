#ifndef BOUNDED_BUFFER_H
#define BOUNDED_BUFFER_H

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include <util_url_file.h>
#include <sodtp_block.h>
#include <unistd.h>
#include <util_log.h>
#include <live_liveCapture.h>
#include <live_xTransport.h>
#include <util_mediaEncoder.h>
#define MAX_BOUNDED_BUFFER_SIZE  3

// class BoundedBuffer {
// public:
//     BoundedBuffer(const BoundedBuffer& rhs) = delete;
//     BoundedBuffer& operator=(const BoundedBuffer& rhs) = delete;

//     BoundedBuffer(std::size_t size) : begin_(0), end_(0), buffered_(0), circular_buffer_(size) {
//         //
//     }

//     void produce(void *ptr) {
//         {
//           std::unique_lock<std::mutex> lock(mutex_);
//           // 等待缓冲不为满。
//           not_full_cv_.wait(lock, [=] { return buffered_ < circular_buffer_.size(); });

//           // 插入新的元素，更新下标。
//           circular_buffer_[end_] = ptr;
//           end_ = (end_ + 1) % circular_buffer_.size();

//           ++buffered_;
//         }  // 通知前，自动解锁。

//         // 通知消费者。
//         not_empty_cv_.notify_one();
//     }

//     void* consume() {
//         std::unique_lock<std::mutex> lock(mutex_);
//         // 等待缓冲不为空。
//         not_empty_cv_.wait(lock, [=] { return buffered_ > 0; });

//         // 移除一个元素。
//         auto var = circular_buffer_[begin_];
//         begin_ = (begin_ + 1) % circular_buffer_.size();

//         --buffered_;

//         // 通知前，手动解锁。
//         lock.unlock();
//         // 通知生产者。
//         not_full_cv_.notify_one();
//         return var;
//     }

// private:
//     std::size_t begin_;
//     std::size_t end_;
//     std::size_t buffered_;
//     std::vector<void *> circular_buffer_;
//     std::condition_variable not_full_cv_;
//     std::condition_variable not_empty_cv_;
//     std::mutex mutex_;
// };

// 传输用的
class StreamPacket {
public:
    SodtpStreamHeader   header;
    AVPacket            packet;
    uint8_t     *codecParExtradata;

public:
    ~StreamPacket() {
        av_packet_unref(&packet);
    }
};

typedef std::shared_ptr<StreamPacket>  StreamPktPtr;
typedef std::shared_ptr<std::vector<StreamPktPtr>> StreamPktVecPtr;


template <typename T>
class BoundedBuffer {
public:
    BoundedBuffer(const BoundedBuffer& rhs) = delete;
    BoundedBuffer& operator=(const BoundedBuffer& rhs) = delete;

    BoundedBuffer() : begin_(0), end_(0), buffered_(0), circular_buffer_(0) {}

    BoundedBuffer(std::size_t size) : begin_(0), end_(0), buffered_(0), circular_buffer_(size, NULL) {
        //
    }

    void reset(std::size_t size) {
        begin_ = 0;
        end_ = 0;
        buffered_ = 0;
        circular_buffer_.resize(size, NULL);
    }

    // ~BoundedBuffer() {
    //     for (int i = 0; i < circular_buffer_.size(); i++) {
    //         if (circular_buffer_[i]) {
    //             delete circular_buffer_[i];
    //         }
    //     }
    //     circular_buffer_.clear();
    // }

    // write data into the buffer, and return the stale pointer.
    T produce(T val) {
        // Print2File("T produce(T val) ");
        std::unique_lock<std::mutex> lock(mutex_);
        // wait for a spare place in the buffer.
        not_full_cv_.wait(lock, [=] { return buffered_ < circular_buffer_.size(); });

        // insert a new element, and update the index.
        auto ret = circular_buffer_[end_];
        circular_buffer_[end_] = val;
        end_ = (end_ + 1) % circular_buffer_.size();

        ++buffered_;
        // Print2File("++buffered_");
        // 通知前，手动解锁。
        lock.unlock();
        // 通知消费者。
        not_empty_cv_.notify_one();
        // Print2File("T produce(T val) End");
        return ret;
    }


    T consume() {
        // Print2File("T consume()");
        std::unique_lock<std::mutex> lock(mutex_);
        // Print2File("std::unique_lock<std::mutex> lock(mutex_);");
        // waiting for an non-empty buffer.
        not_empty_cv_.wait(lock, [=] { return buffered_ > 0; });
        // Print2File("not_empty_cv_.wait(lock, [=] { return buffered_ > 0; });");
        // return one element.
        auto ret = circular_buffer_[begin_];
        begin_ = (begin_ + 1) % circular_buffer_.size();

        --buffered_;
        // Print2File("--buffered_;");
        // unlock manually.
        lock.unlock();
        // notify producer.
        not_full_cv_.notify_one();
        // Print2File("T consume() End");
        return ret;
    }



private:
    std::size_t begin_;
    std::size_t end_;
    std::size_t buffered_;
    std::vector<T> circular_buffer_;
    std::condition_variable not_full_cv_;
    std::condition_variable not_empty_cv_;
    std::mutex mutex_;
};

bool liveConsumeThread(LiveCapture *lc, MediaEncoder *xe, XTransport *xt, std::vector<StreamCtxPtr> *vStmCtx, BoundedBuffer<StreamPktVecPtr> *pBuffer){
    // camera 直播输入源
    int i = 0;      // packet index
    int ret = 0;
    int stream_num = 0; // 先暂时设定死
    int block_id = 0;
    StreamPktVecPtr pStmPktVec = NULL;
    // (*ptr) already exists, then we can just resize it.
    // 摄像头消费xData线程
    long long beginTime = GetCurTime();
    for (;;){
        XData vd = lc->Pop();
        if (vd.size <= 0){
            msleep(1);
            continue;
        }
        //处理视频
        if (vd.size > 0){
            vd.pts = vd.pts - beginTime;
            XData yuv = xe->RGBToYUV(vd);
            vd.Drop();
            XData dpkt = xe->EncodeVideo(yuv);
            if (dpkt.size > 0){
                // timeMain.evalTime("dpkt.size > 0");
                if (!dpkt.data || dpkt.size <= 0){
                    Print2File("!pkt.data || pkt.size <= 0");
                    continue;
                }
                stream_num = vStmCtx->size();
                if (!pStmPktVec) {
                    pStmPktVec = std::make_shared<std::vector<StreamPktPtr>>(stream_num);
                    for (auto &item : *pStmPktVec) {
                        item = std::make_shared<StreamPacket>();
                    }
                }
                // (*ptr) already exists, then we can just resize it.
                else {
                    if (pStmPktVec->size() != stream_num) {
                        pStmPktVec->resize(stream_num);
                    }
                }
                i = 0;
                for (auto it = vStmCtx->begin(); it != vStmCtx->end(); i++) {
                    // 这里注意 ret=1 设置为直播1
                    ret = 1;
                    // bool SendFrame(XData d, int streamIndex)
                    AVPacket *pack = (AVPacket *)dpkt.data;
                    //这句十分重要
                    av_packet_ref(&(*pStmPktVec)[i]->packet,pack);
                    int streamIndex = 0;//强制设置为视频流为0
                    pack->stream_index = streamIndex;
                    AVRational stime;
                    AVRational dtime;
                    //判断是音频还是视频
                    if (xt->vs && xt->vc && pack->stream_index == xt->vs->index)
                    {
                        stime = xt->vc->time_base;
                        dtime = xt->vs->time_base;
                    }
                    else if(xt->as && xt->ac &&pack->stream_index == xt->as->index)
                    {
                        stime = xt->ac->time_base;
                        dtime = xt->as->time_base;
                    }
                    //推流
                    pack->pts = av_rescale_q(pack->pts, stime, dtime);
                    pack->dts = av_rescale_q(pack->dts, stime, dtime);
                    pack->duration = av_rescale_q(pack->duration, stime, dtime);//duration计算错误，改成拉流端控制
                    (*pStmPktVec)[i]->header.duration = (*pStmPktVec)[i]->packet.duration * 1000;
                    (*pStmPktVec)[i]->header.duration *= (*it)->pFmtCtx->streams[0]->time_base.num;

                    //原设置相关代码段
                    if (ret < 0) {
                        (*pStmPktVec)[i]->header.flag = HEADER_FLAG_FIN;  // end of stream
                    } else {
                        (*pStmPktVec)[i]->header.flag = HEADER_FLAG_NULL; // normal state
                    }
                    
                    if ((*pStmPktVec)[i]->packet.flags & AV_PKT_FLAG_KEY) {
                        (*pStmPktVec)[i]->header.flag |= HEADER_FLAG_KEY;
                        (*pStmPktVec)[i]->header.haveFormatContext = true;
                        // 拷贝 xt->ic->streams[0]->codecpar 信息
                        CodecParWithoutExtraData* codeparPtr = &((*pStmPktVec)[i]->header.codecPar);
                        // 拷贝codecpar其他变量
                        int ret3 = lhs_copy_parameters_to_myParameters(codeparPtr,xt->ic->streams[0]->codecpar);
                        (*pStmPktVec)[i]->codecParExtradata = xt->ic->streams[0]->codecpar->extradata;
                    }

                    (*pStmPktVec)[i]->header.stream_id = (*it)->stream_id;
                    (*pStmPktVec)[i]->header.block_id = block_id;
                    
                    if (ret < 0) {
                        it = vStmCtx->erase(it);
                    }
                    else {
                        it++;
                    }
                }
                block_id ++;
                // write a vec pointer of packet ptrs into the buffer, and get a stale vec pointer.
                pStmPktVec = pBuffer->produce(pStmPktVec);
                pStmPktVec = NULL;
                if (vStmCtx->empty()) {
                    pBuffer->produce(NULL);
                    break;
                }
            }
        }
    }
    return true;
}

bool prepareLiveMedia(LiveCapture *lc, MediaEncoder *xe, XTransport *xt) {
    timeMainServer.evalTime("s","prepareLiveMedia");
    //  1.打开摄像机
	if (!lc->Init(0)) {
        Print2File("1. 打开摄像机 : 失败！！");
		return false;
	}
	lc->Start();
    timeMainServer.evalTime("s","OpenCamera");

    //  音视频编码类
	//  2 初始化格式转换上下文
	xe->inWidth = lc->width;
	xe->inHeight = lc->height;
	xe->outWidth = lc->width;
	xe->outHeight = lc->height;

	if (!xe->InitScale()){
        Print2File("2. 初始化视频像素转换上下文 : 失败！！");
		return false;
	}
    timeMainServer.evalTime("s","InitScale");

	if (!xe->InitVideoCodec()){
        Print2File("3. 初始化视频编码器 : 失败！！");
		return false;
	}
    timeMainServer.evalTime("s","InitVieoCodec");

    // time.start();
    if(!xt->MemoryOutByFIFO()){
        Print2File("4. MemoryOutByFIFO : 失败！！");
		return false;
    }
    timeMainServer.evalTime("s","MemoryOutByFIFO");
    
    // time.start();
	int vindex = 0;
	vindex = xt->AddStream(xe->vc);
	if (vindex<0){
        Print2File("5. 添加视频流 : 失败！！");
		return false;
	}
    timeMainServer.evalTime("s","AddStream");

    // Tools::Time::AlgoTimeMs time1;
    // time1.start();
    // Print2File("[============Info========)");
    // time1.evalTime("Print2File");

    return true;
}

void live_produce(BoundedBuffer<StreamPktVecPtr> *pBuffer, const char *conf){
    int i = 0;      // packet index
    int ret = 0;
    int stream_num = 0;
    int block_id = 0;
    // 准备摄像头的一对生产者消费者流RGB与YUV
    LiveCapture *lc = new LiveCapture();
    MediaEncoder *mc = new MediaEncoder();
    XTransport *xt = new XTransport();
    timeMainServer.evalTime("s","BeforePrepareLiveMedia");
    if(prepareLiveMedia(&(*lc),&(*mc),&(*xt))){
        StreamPktVecPtr pStmPktVec = NULL;
        std::vector<StreamCtxPtr> vStmCtx;
        if(!set_StmCtxPtrsAndId(&vStmCtx,xt->ic)){
            Print2File("init_live_resource()==false");
            return;
        }
        timeMainServer.evalTime("s","Before2Thread");
        std::thread cameraCaptureProduceThread(&LiveCapture::run,lc);
        std::thread cameraCaptureConsumeThread(liveConsumeThread,&(*lc),&(*mc),&(*xt),&vStmCtx,&(*pBuffer));
        //这里执行并行的程序
        cameraCaptureProduceThread.join();
        cameraCaptureConsumeThread.join();
    }
    return;
}

// read packets from files, and flush them into the buffer.
void produce(BoundedBuffer<StreamPktVecPtr> *pBuffer, const char *conf) {
    Print2File("void produce(BoundedBuffer<StreamPktVecPtr> *pBuffer, const char *conf) ");
    StreamPktVecPtr pStmPktVec = NULL;
    //StreamPktVecPtr 下是 StreamPacket
    //StreamPacket 下是
    //SodtpStreamHeader   header;
    //AVPacket保存的是解码前的数据，也就是压缩后的数据。该结构本身不直接包含数据，其有一个指向数据域的指针，FFmpeg中很多的数据结构都使用这种方法来管理数据。
    //AVPacket            packet;

    std::vector<StreamCtxPtr> vStmCtx;

    int i = 0;      // packet index
    int ret = 0;
    int stream_num = 0;
    int block_id = 0;
    
    init_resource(&vStmCtx, conf);

    while (true) {
        stream_num = vStmCtx.size();
        Print2File("stream_num : "+std::to_string(stream_num));
        // fprintf(stderr, "stream num %d\n", stream_num);
        if (!pStmPktVec) {
            pStmPktVec = std::make_shared<std::vector<StreamPktPtr>>(stream_num);
            for (auto &item : *pStmPktVec) {
                item = std::make_shared<StreamPacket>();
            }
        }
        // (*ptr) already exists, then we can just resize it.
        else {
            if (pStmPktVec->size() != stream_num) {
                pStmPktVec->resize(stream_num);
            }
        }
        // fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

        // Assert all packets have already been av_packet_unref(), which
        // should be done after reading data from the bounded buffer. 

        i = 0;  // packet index
        Print2File("i = 0 : "+std::to_string(i));
        for (auto it = vStmCtx.begin(); it != vStmCtx.end(); i++) {
            Print2File("stream_num := vStmCtx->end(); i++) : "+std::to_string(i));
            // fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
            // av_init_packet(&(*pStmPktVec)[i]->packet);
            // int64_t *debugDts = &(*pStmPktVec)[i]->packet.dts;
            // int64_t *debugPts = &(*pStmPktVec)[i]->packet.pts;
            // Print2File("&(*pStmPktVec)[i]->packet.pts : " + std::to_string(*debugDts));
            // Print2File("&(*pStmPktVec)[i]->packet.dts : " + std::to_string(*debugPts));
            // 上面验证了&(*pStmPktVec)[i]->packet)中packet为空，只是提供了一个 AVPacket *pkt = av_packet_alloc();
            // int * DebugSize = &(*pStmPktVec)[i]->packet.size;
            // Print2File("Before read : "+std::to_string(*DebugSize));
            // Print2File("Before read : "+std::to_string((*pStmPktVec)[i]->packet.size));

            // static inline int file_read_packet2(
            //     AVFormatContext         *pFormatCtx,// 解封装上下文
            //     AVPacket                *pPacket) {// AVPacket *pkt = av_packet_alloc();

            //     return av_read_frame(pFormatCtx, pPacket);
            // }

            ret = file_read_packet2((*it)->pFmtCtx, &(*pStmPktVec)[i]->packet);
            // fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

            if (ret < 0) {
                Print2File("(*pStmPktVec)[i]->header.flag = HEADER_FLAG_FIN;");
                (*pStmPktVec)[i]->header.flag = HEADER_FLAG_FIN;  // end of stream
            } else {
                Print2File("(*pStmPktVec)[i]->header.flag = HEADER_FLAG_NULL;");
                (*pStmPktVec)[i]->header.flag = HEADER_FLAG_NULL; // normal state
            }

            Print2File("(*pStmPktVec)[i]->packet.pts) : "+std::to_string((*pStmPktVec)[i]->packet.pts));
            Print2File("(*pStmPktVec)[i]->packet.dts) : "+std::to_string((*pStmPktVec)[i]->packet.dts));
            Print2File("(*pStmPktVec)[i]->packet.dts) : "+std::to_string((*pStmPktVec)[i]->packet.duration));
            (*pStmPktVec)[i]->header.stream_id = (*it)->stream_id;
            Print2File("(*it)->stream_id : "+std::to_string((*pStmPktVec)[i]->header.stream_id));
            (*pStmPktVec)[i]->header.block_id = block_id;
            Print2File("(*it)->stream_id : "+std::to_string((*pStmPktVec)[i]->header.block_id));


            //==========================原来======================================

            // We do not set the block timestamp since the block is pre-fetched from files.
            // However, we need to set the timestamp when forwarding them. In fact, it
            // should be set by the streamer.
            // (*pStmPktVec)[i]->header.block_ts = packet.dts;
            // (*pStmPktVec)[i]->header.block_ts = current_mtime();
            
            // duration in milli-seconds
            Print2File("(*pStmPktVec)[i]->header.duration 1 : "+std::to_string((*pStmPktVec)[i]->header.duration));
            Print2File("(*pStmPktVec)[i]->packet.duration 1 : "+std::to_string((*pStmPktVec)[i]->packet.duration));
            //  上面是有值的
            (*pStmPktVec)[i]->header.duration = (*pStmPktVec)[i]->packet.duration * 1000;
            Print2File("(*pStmPktVec)[i]->header.duration 2 : "+std::to_string((*pStmPktVec)[i]->header.duration));
            Print2File("(*it)->pFmtCtx->streams[0]->time_base.num 2 : "+std::to_string((*it)->pFmtCtx->streams[0]->time_base.num));
            (*pStmPktVec)[i]->header.duration *= (*it)->pFmtCtx->streams[0]->time_base.num;
            Print2File("(*pStmPktVec)[i]->header.duration 3 : "+std::to_string((*pStmPktVec)[i]->header.duration));
            Print2File("(*it)->pFmtCtx->streams[0]->time_base.den 3 : "+std::to_string((*it)->pFmtCtx->streams[0]->time_base.den));
            (*pStmPktVec)[i]->header.duration /= (*it)->pFmtCtx->streams[0]->time_base.den;
            Print2File("(*pStmPktVec)[i]->header.duration End : "+std::to_string((*pStmPktVec)[i]->header.duration));

            //==========================原来======================================

            // fprintf(stderr, "duration %dms\n", (*pStmPktVec)[i]->header.duration);


            // printf("frame duration %lld, time base: num %d den %d\n", packet.duration,
            //         conn_io->vFmtCtxPtrs[i]->streams[0]->time_base.num,
            //         conn_io->vFmtCtxPtrs[i]->streams[0]->time_base.den);

            if ((*pStmPktVec)[i]->packet.flags & AV_PKT_FLAG_KEY) {
                Print2File("AV_PKT_FLAG_KEY  ");
                (*pStmPktVec)[i]->header.flag |= HEADER_FLAG_KEY;
            }
            // Print2File("stream_id : "+std::to_string((*it)->stream_id));
            // Print2File("block_id : "+std::to_string(block_id));
            // Print2File("After read : "+std::to_string((*pStmPktVec)[i]->packet.size));
            fprintf(stderr, "produce: stream %d,\tblock %d,\tsize %d\n",
                (*it)->stream_id, block_id, (*pStmPktVec)[i]->packet.size);

            if (ret < 0) {
                fprintf(stderr, "remove a format context\n");
                it = vStmCtx.erase(it);
            }
            else {
                it++;
            }
            Print2File("For End=========================================================  ");
        }

        block_id ++;
        // fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

        // write a vec pointer of packet ptrs into the buffer, and get a stale vec pointer.
        pStmPktVec = pBuffer->produce(pStmPktVec);
        pStmPktVec = NULL;

        if (vStmCtx.empty()) {
            fprintf(stderr, "produce: streaming stops now\n");
            fprintf(stderr, "produce: insert a NULL data.\n");
            pBuffer->produce(NULL);
            break; // while(true) 中断的地方
        }
        // usleep(5000);
        // fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
    }
}

// 这里的consume只是用来作Buffer测试，真正的Buffer消耗在dtp_server.h的sender_cb中
// 发送数据用这个quiche_conn_stream_send_full
// read packets from the buffer, to which packets in files are written.
void consume(BoundedBuffer<StreamPktVecPtr> *pBuffer) {
    StreamPktVecPtr pStmPktVec = NULL;
    // fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

    while (true) {
        Print2File("pStmPktVec = pBuffer->consume();");
        pStmPktVec = pBuffer->consume();
        if (!pStmPktVec) {
            fprintf(stderr, "consume: consuming stops now\n");
            Print2File("consume: consuming stops now");
            break;
        }
        // fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
        for (auto &ptr : *pStmPktVec) {
            fprintf(stderr, "consume: stream %d,\tblock %d,\tsize %d\n",
                ptr->header.stream_id, ptr->header.block_id,
                ptr->packet.size);
            av_packet_unref(&ptr->packet);
        }
        // fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
        // if ((*pStmPktVec)[0]->header.flag & HEADER_FLAG_FIN) {
        //     fprintf(stderr, "consume: consuming stops now\n");
        //     break;
        // }
        usleep(40000);
    }
}

#endif  // BOUNDED_BUFFER_H