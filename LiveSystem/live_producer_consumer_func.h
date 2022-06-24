#ifndef LIVE_PRODUCER_CONSUMER_FUNC_H
#define LIVE_PRODUCER_CONSUMER_FUNC_H
#include "sodtp/bounded_buffer.h"
#include "live_xTransport.h"
#include "live_liveCapture.h"
#include "live_AudioCapture.h"
#include "live_xData.h"
#include <cassert>

/**
   Read video frame from LiveCapture and send them to multiple targets
   described in vStmCtx. All the targets receives the same video frame each round

   @param: vStmCtx, ptrs of StreamContexts. It stores information of each stream indexed by stream_id
 */
bool liveConsumeThreadFull(LiveCapture *lc,
                           MediaEncoder *xe,
                           XTransport *xt,
                           std::vector<StreamCtxPtr> *vStmCtx,
                           BoundedBuffer<StreamPktVecPtr> *pBuffer){
  // camera 直播输入源
  int             i           = 0; // packet index
  int             ret         = 0;
  int             stream_num  = 0; // 先暂时设定死
  int             block_id    = 0;
  StreamPktVecPtr pStmPktVec  = NULL;
  // 摄像头消费xData线程
  long long       beginTime   = GetCurTime();
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
      --lc->buffered_RGB;
      timeFrameServer.evalTimeStamp("RGB_Pop","s","FrameTime");
      timeFrameServer.evalTimeStamp("RGB_Buffer_Count","s",std::to_string(lc->buffered_RGB));
      XData dpkt = xe->EncodeVideo(yuv);
      if (dpkt.size > 0){
        timeFrameServer.evalTimeStamp("FrameToYUV","s","FrameTime");
        // timeMain.evalTime("dpkt.size > 0");
        if (!dpkt.data || dpkt.size <= 0){
          Print2File("!pkt.data || pkt.size <= 0");
          continue;
        }
        // create next stream
        stream_num = vStmCtx->size();
        // if ptr doesn't exist, then create a vector to store packets in streams
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
        // Begin pushing the stream
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
              assert(false);
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
            timeFrameServer.evalTimeStamp("Net_Produce","I_frame",std::to_string(block_id));
            (*pStmPktVec)[i]->header.flag |= HEADER_FLAG_KEY;
            (*pStmPktVec)[i]->header.haveFormatContext = true;
            // 拷贝 xt->ic->streams[0]->codecpar 信息
            CodecParWithoutExtraData* codeparPtr = &((*pStmPktVec)[i]->header.codecPar);
            // 拷贝codecpar其他变量
            int ret3 = lhs_copy_parameters_to_myParameters(codeparPtr,xt->ic->streams[0]->codecpar);
            (*pStmPktVec)[i]->codecParExtradata = xt->ic->streams[0]->codecpar->extradata;
          }else{
            timeFrameServer.evalTimeStamp("Net_Produce","P_frame",std::to_string(block_id));
          }

          (*pStmPktVec)[i]->header.stream_id = (*it)->stream_id;
          timeFrameServer.evalTimeStamp("Net_Produce_redsult", "stream", std::to_string((*it)->stream_id));
          (*pStmPktVec)[i]->header.block_id = block_id;

          if (ret < 0) {
            it = vStmCtx->erase(it);
          }
          else {
            it++;
          }
        }

        // write a vec pointer of packet ptrs into the buffer, and get a stale vec pointer.
        // 特别注意 block_id 多流改位置！
        block_id += 2;
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
bool liveConsumeThread(LiveCapture *lc,
                       MediaEncoder *xe,
                       XTransport *xt,
                       std::vector<StreamCtxPtr> *vStmCtx,
                       BoundedBuffer<StreamPktVecPtr> *pBuffer){
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
            --lc->buffered_RGB;
            timeFrameServer.evalTimeStamp("RGB_Pop","s","FrameTime");
            timeFrameServer.evalTimeStamp("RGB_Buffer_Count","s",std::to_string(lc->buffered_RGB));
            XData dpkt = xe->EncodeVideo(yuv);
            if (dpkt.size > 0){
                timeFrameServer.evalTimeStamp("FrameToYUV","s","FrameTime");
                // timeMain.evalTime("dpkt.size > 0");
                if (!dpkt.data || dpkt.size <= 0){
                    Print2File("!pkt.data || pkt.size <= 0");
                    continue;
                }
                // create next stream
                stream_num = vStmCtx->size();
                // if ptr doesn't exist, then create a vector to store packets in streams
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
                // Begin pushing the stream
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
                        assert(false);
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
                        timeFrameServer.evalTimeStamp("Net_Produce","I_frame",std::to_string(block_id));
                        (*pStmPktVec)[i]->header.flag |= HEADER_FLAG_KEY;
                        (*pStmPktVec)[i]->header.haveFormatContext = true;
                        // 拷贝 xt->ic->streams[0]->codecpar 信息
                        CodecParWithoutExtraData* codeparPtr = &((*pStmPktVec)[i]->header.codecPar);
                        // 拷贝codecpar其他变量
                        int ret3 = lhs_copy_parameters_to_myParameters(codeparPtr,xt->ic->streams[0]->codecpar);
                        (*pStmPktVec)[i]->codecParExtradata = xt->ic->streams[0]->codecpar->extradata;
                    }else{
                        timeFrameServer.evalTimeStamp("Net_Produce","P_frame",std::to_string(block_id));
                    }

                    (*pStmPktVec)[i]->header.stream_id = (*it)->stream_id;
                    timeFrameServer.evalTimeStamp("Net_Produce_redsult", "stream", std::to_string((*it)->stream_id));
                    (*pStmPktVec)[i]->header.block_id = block_id;

                    if (ret < 0) {
                        it = vStmCtx->erase(it);
                    }
                    else {
                        it++;
                    }
                }

                // write a vec pointer of packet ptrs into the buffer, and get a stale vec pointer.
                // 特别注意 block_id 多流改位置！
                block_id += 2;
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

bool audioConsumeThread(AudioCapture *ac,
                        MediaEncoder *xe,
                        XTransport *xt,
                        std::vector<StreamCtxPtr> *vStmCtx,
                        BoundedBuffer<StreamPktVecPtr> *pBuffer){
  int ret = 0;
  int stream_num = 0;
  int block_id = 1;
  StreamPktVecPtr pStmPktVec = NULL;
  long long beginTime = GetCurTime();
  while(true) {
    XData audio_frame = ac->Pop();
    if(audio_frame.size <= 0){
      msleep(1);
      continue;
    } else {
      // handle audio
      audio_frame.pts -= beginTime;
      XData rsmpl_audio_frame = xe->Resample(audio_frame);
      audio_frame.Drop();
      timeFrameServer.evalTime("Resample", "s");
      XData packet = xe->EncodeAudio(rsmpl_audio_frame);
      if(packet.size > 0) {
        timeFrameServer.evalTimeStamp("AudioEncode", "s", "FrameTime");
        if(!packet.data) {
          Print2File("!pkt.data");
          continue;
        }
        stream_num = vStmCtx->size();
        if (!pStmPktVec) {
          pStmPktVec = std::make_shared<std::vector<StreamPktPtr>>(stream_num);
          for(auto &item : *pStmPktVec) {
            item = std::make_shared<StreamPacket>();
          }
        } else {
          if(pStmPktVec->size() != stream_num) {
            pStmPktVec->resize(stream_num);
          }
        }

        int i = 0;

        for(auto it = vStmCtx->begin(); it != vStmCtx->end(); ++i) {
          ret = 1;
          AVPacket *pack = (AVPacket *) packet.data;
          av_packet_ref(&(*pStmPktVec)[i]->packet, pack);
          int streamIndex = 1; // Suppose audio stream is stream 1
          pack->stream_index = streamIndex;
          AVRational stime;
          AVRational dtime;

          if(xt->vs && xt->vc && pack->stream_index == xt->vs->index) {
            stime = xt->vc->time_base;
            dtime = xt->vs->time_base;
            assert(false); // this should be unreachable
          } else if(xt->as && xt->ac && pack->stream_index == xt->as->index){
            stime = xt->ac->time_base;
            dtime = xt->as->time_base;
          }
          pack->pts = av_rescale_q(pack->pts, stime, dtime);
          pack->dts = av_rescale_q(pack->dts, stime, dtime);
          pack->duration = av_rescale_q(pack->duration, stime, dtime);
          (*pStmPktVec)[i]->header.duration = (*pStmPktVec)[i]->packet.duration * 1000;
          (*pStmPktVec)[i]->header.duration *= (*it)->pFmtCtx->streams[0]->time_base.num;

          if(ret < 0) {
            (*pStmPktVec)[i]->header.flag = HEADER_FLAG_FIN; // end of stream
            assert(false); // live stream ends nowhere
          } else {
            (*pStmPktVec)[i]->header.flag = HEADER_FLAG_NULL; // normal state
          }

          // copy parameters
          if((*pStmPktVec)[i]->packet.flags & AV_PKT_FLAG_KEY) {
            timeFrameServer.evalTimeStamp("Net_Produce", "audio I frame", std::to_string(block_id));
            (*pStmPktVec)[i]->header.flag |= HEADER_FLAG_KEY;
            (*pStmPktVec)[i]->header.haveFormatContext = true;

            // Copy xt->ic->streams[1]->codecpar info
            CodecParWithoutExtraData* codeparPtr = &((*pStmPktVec)[i]->header.codecPar);
            int ret3 = lhs_copy_parameters_to_myParameters(codeparPtr, xt->ic->streams[0]->codecpar);
            (*pStmPktVec)[i]->codecParExtradata = xt->ic->streams[0]->codecpar->extradata;
          } else {
            timeFrameServer.evalTimeStamp("Net_Produce", "Audio P_frame", std::to_string(block_id));
          }

          (*pStmPktVec)[i]->header.stream_id = (*it)->stream_id;
          timeFrameServer.evalTimeStamp("Audio_Produce_result", "stream", std::to_string((*it)->stream_id));
          (*pStmPktVec)[i]->header.block_id = block_id;
          timeFrameServer.evalTimeStamp("Audio_Produce_block_id", "block_id", std::to_string(block_id));

          if (ret < 0) {
            it = vStmCtx->erase(it);
          }
          else {
            it++;
          }
        }
        block_id += 2;
        pStmPktVec = pBuffer->produce(pStmPktVec);
        pStmPktVec = NULL;
        if(vStmCtx->empty()) {
          pBuffer->produce(NULL);
          break;
        }
      }
    }
  }
  return true;
}

bool prepareAudio(AudioCapture *ac, MediaEncoder *xe, XTransport *xt) {
  timeMainServer.evalTime("s", "prepareAudio");
  // 1. 初始化音频捕捉
  if(!ac->init()) {
    Print2File("1. 打开音频设备： 失败！");
    return false;
  }
  ac->Start();
  timeMainServer.evalTime("s", "OpenAudio");

  // 音频编码类
  // 2. 初始化上下文
  // Default Setting

  if(!xe->InitResample()) {
    Print2File("2. 初始化重采样：失败！！");
    return false;
  }
  timeMainServer.evalTime("s", "InitResample");

	if (!xe->InitAudioCodec()){
    Print2File("3. 初始化音频编码器 : 失败！！");
		return false;
	}
  timeMainServer.evalTime("s","InitAudioCodec");

  // time.start();
  if(!xt->MemoryOutByFIFO()){
    Print2File("4. MemoryOutByFIFO : 失败！！");
		return false;
  }
  timeMainServer.evalTime("s","MemoryOutByFIFO");

  // time.start();
	int aindex = 0;
	aindex = xt->addStream(xe->ac);
	if (aindex < 0){
    Print2File("5. 添加音频流 : 失败！！");
		return false;
	}
  timeMainServer.evalTime("s","AddStream(a)");

  // Tools::Time::AlgoTimeMs time1;
  // time1.start();
  // Print2File("[============Info========)");
  // time1.evalTime("Print2File");

  return true;
}

// warning: watch potential memory leak
void audio_produce(BoundedBuffer<StreamPktVecPtr> *pBuffer, const char *conf) {
  int ret = 0;
  int stream_id = 0;
  AudioCapture *ac = new AudioCapture();
  MediaEncoder *mc = new MediaEncoder();
  XTransport *xt = new XTransport();
  timeMainServer.evalTime("s","BeforePrepareAudio");
  if(prepareAudio(&(*ac), &(*mc), &(*xt))) {
    StreamPktVecPtr pStmPktVec = NULL;
    std::vector<StreamCtxPtr> vStmCtx;
    int stream_id = set_StmCtxPtrsAndId(&vStmCtx, xt->ic);
    if(stream_id < 0) {
      Print2File("init audio resource failed");
      return;
    }
    timeMainServer.evalTime("s", "BeforeAudioThread");
    ac->start();
    std::thread audioCaptureConsumeThread(audioConsumeThread,
                                          &(*ac),
                                          &(*mc),
                                          &(*xt),
                                          &vStmCtx,
                                          &(*pBuffer));
    audioCaptureConsumeThread.join();
    ac->drop();
  }
  return;
}

bool prepareLiveMedia(LiveCapture *lc, MediaEncoder *xe, XTransport *xt, int camera = 0, int resolution = 360) {
  timeMainServer.evalTime("s","prepareLiveMedia");
  //  1.打开摄像机
  if (!lc->Init(camera, resolution)) { // 在同一台机器上运行两个视频程序的话，建议使用 360p，更加流畅
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
	vindex = xt->addStream(xe->vc);
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

// warning: watch potential memory leak
// TODO: bad implementation just for demo, need further refinement
void live_produce_full(BoundedBuffer<StreamPktVecPtr> *pBuffer, const char *conf){
    int i = 0;      // packet index
    int ret = 0;
    int stream_id_video = -1;
    int stream_id_audio = -1;

    // Prepare capture, encoder and transporter
    LiveCapture *lc = new LiveCapture();
    AudioCapture *ac = new AudioCapture();
    MediaEncoder *mc = new MediaEncoder();
    XTransport *xt = new XTransport();
    XTransport *axt = new XTransport();
    timeMainServer.evalTime("s","BeforePrepareLiveMedia");
    if(prepareLiveMedia(&(*lc),&(*mc),&(*xt))){
      std::vector<StreamCtxPtr> vStmCtx; // Store contexts of all the streams in a 'file' such as video and audio
      stream_id_video = set_StmCtxPtrsAndId(&vStmCtx,xt->ic); // add a stream context into the vector (video stream_id = 0)
      if(stream_id_video < 0){
        Print2File("init_live_resource()==false");
        return;
      }
      if(prepareAudio(&(*ac), &(*mc), &(*axt))) {
        std::vector<StreamCtxPtr> vStmCtxAudio; //* temporary implementation
        stream_id_audio = set_StmCtxPtrsAndId(&vStmCtxAudio,axt->ic); // add audio context into the vector (audio stream_id = 1)
        if(stream_id_audio < 0){
          Print2File("init_live_resource()==false");
          return;
        }
        timeMainServer.evalTime("s","Before2Thread");
        std::thread cameraCaptureConsumeThread(
            liveConsumeThread, &(*lc), &(*mc), &(*xt), &vStmCtx, &(*pBuffer));
        std::thread cameraCaptureProduceThread(&LiveCapture::run,lc); // start capturing video
        ac->start(); // start capturing audio
        std::thread audioCaptureConsumeThread(audioConsumeThread,
                                              &(*ac),
                                              &(*mc),
                                              &(*axt),
                                              &vStmCtxAudio,
                                              &(*pBuffer));
        timeMainServer.evalTime("s", "BeforeAudioThread");
        //这里执行并行的程序
        cameraCaptureProduceThread.join();
        cameraCaptureConsumeThread.join();
        audioCaptureConsumeThread.join();
        ac->drop();
      }

    }
    return;
}

// warning: watch potentail memory leak
void live_produce(BoundedBuffer<StreamPktVecPtr> *pBuffer, const char *conf){
    int i = 0;      // packet index
    int ret = 0;
    int stream_id = -1;
    // 准备摄像头的一对生产者消费者流RGB与YUV
    LiveCapture *lc = new LiveCapture();
    MediaEncoder *mc = new MediaEncoder();
    XTransport *xt = new XTransport();
    fprintf(stderr, "live_produce, config name: %s\n", conf);
    int camera = 0, resolution = 360;
    FILE* fconf = fopen(conf, "r");
    if(!fconf) {
      fprintf(stderr, "cannot open conf file\n");
    } else {
      fscanf(fconf, "%d %d", &camera, &resolution);
      fprintf(stderr, "load conf: %d %d\n", camera, resolution);
      if(camera < 0 || camera > 4 || resolution <= 0) {
        fprintf(stderr, "error conf parameter\n");
        camera = 0;
        resolution = 360;
      }
      fclose(fconf);
    }
    
    
    timeMainServer.evalTime("s","BeforePrepareLiveMedia");
    if(prepareLiveMedia(&(*lc),&(*mc),&(*xt), camera, resolution)){
        std::vector<StreamCtxPtr> vStmCtx;
        stream_id = set_StmCtxPtrsAndId(&vStmCtx,xt->ic);
        if(stream_id < 0){
            Print2File("init_live_resource()==false");
            return;
        }
        timeMainServer.evalTime("s","Before2Thread");
        std::thread cameraCaptureConsumeThread(liveConsumeThread,&(*lc),&(*mc),&(*xt),&vStmCtx,&(*pBuffer));
        std::thread cameraCaptureProduceThread(&LiveCapture::run,lc);
        timeMainServer.evalTime("s", "BeforeAudioThread");
        //这里执行并行的程序
        cameraCaptureProduceThread.join();
        cameraCaptureConsumeThread.join();
    }
    return;
}

#endif // LIVE_PRODUCER_CONSUMER_FUNC_H
