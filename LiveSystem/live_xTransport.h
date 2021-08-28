#ifndef LIVE_XTRANSPORT_H
#define LIVE_XTRANSPORT_H
#include <iostream>
#include <string>
extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>
}
#include "util/util_log.h"
#include "live_xData.h"
#pragma comment(lib, "avformat.lib")

// Write all the data from buf to opaque
//
// The type of opaque should be a struct buffer_data_write
//
// The data written in uint8 FIFO
static int lhs_write_packet(void *opaque, uint8_t *buf, int buf_size) {
  struct buffer_data_write *bd = (struct buffer_data_write *)opaque;
  while (buf_size > bd->room) {
    int64_t offset = bd->ptr - bd->buf;
    // uint8_t *bufferPtr = static_cast<uint8_t *>(malloc( sizeof(uint8_t)))
    void *tmpPtr = av_realloc_f(bd->buf, 2, bd->size);
    bd->buf = static_cast<uint8_t *>(malloc(sizeof(tmpPtr)));
    if (!bd->buf)
      return AVERROR(ENOMEM);
    bd->size *= 2;
    bd->ptr = bd->buf + offset;
    bd->room = bd->size - offset;
  }
  /* copy buffer data to buffer_data buffer */
  memcpy(bd->ptr, buf, buf_size);
  bd->ptr += buf_size;
  bd->room -= buf_size;
  return buf_size;
}

//! A encapsulized Format Context of a media file.
//!
//! Each object has a compulsory video stream and an optional audio stream.
//!
//! Member functions help to create the FormatContext of both video and audio
//! stream
//!
//! Warning!: Potential memory leak
class XTransport
{
public:
	//自定义封装器
	AVFormatContext *ic = NULL;

	// AVIOContext *avio_out = NULL;
	//视频编码器流
	const AVCodecContext *vc = NULL;
	//视频编码器流
	const AVCodecContext *ac = NULL;
	//视频流
	AVStream *vs = NULL;
	//音频流
	AVStream *as = NULL;

	// String url = "";
	~XTransport(){};
	XTransport(){
		static bool isFirst = true;
		if (isFirst)
		{
			// av_register_all(); // deprecated
			avformat_network_init();
			isFirst = false;
		}
	};
	XTransport(const XTransport&){};
	XTransport& operator=(const XTransport&);
	void close()
	{
		if (ic)
		{
			avformat_close_input(&ic);
			vs = NULL;
		}
		vc = NULL;
	}

  /**
   Add Stream to this file, described in *ic

   This function creates a new stream according to the given
   AVCodecContext and then it stores the new stream into the coresponding pointer that is either video or audio stream

   @return: the stream number of the added stream. -1 for error

    TODO: check the number of video and audio streams
   */
  int addStream(const AVCodecContext *c)
	{
		if (!c) return -1;

		// Add a media stream
    // p.s.: the stream doesn't actually hold data in the Live System
		AVStream *st = avformat_new_stream(ic, NULL);
		if (!st)
		{
			return -1;
		}
		st->codecpar->codec_tag = 0;
		//从编码器复制参数
		avcodec_parameters_from_context(st->codecpar, c);
		av_dump_format(ic, 0, NULL, 1);

		if (c->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			vc = c;
			vs = st;
		}
		else if(c->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			ac = c;
			as = st;
		}
		return st->index;
	}

	bool MemoryOutByFIFO(){
		uint8_t *avio_ctx_buffer = NULL;
		size_t avio_ctx_buffer_size = 4096;//原来4096
		const size_t bd_buf_size = 1024;//原来1024
		char *input_filename = NULL;
		int ret = 0;
		struct buffer_data_write bd = { 0 };
		char err_buf[512];
		int err_buf_size = 512;
		/* fill opaque structure used by the AVIOContext write callback */
 		bd.ptr  = bd.buf = (uint8_t *)av_malloc(bd_buf_size);
		if (!bd.buf) {
			ret = AVERROR(ENOMEM);
			av_make_error_string(err_buf, err_buf_size, ret);
			Print2File(err_buf);
			Print2File(" !bd.buf ret = AVERROR(ENOMEM) ERR : "+std::to_string(ret));
			return false;
		}
		bd.size = bd.room = bd_buf_size;
		// 1. 分配缓冲区
		avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
		if (!avio_ctx_buffer) {
			ret = AVERROR(ENOMEM);
			Print2File("!avio_ctx_buffer");
			Print2File("ret = AVERROR(ENOMEM) ERR : "+std::to_string(ret));
			return false;
		}
		// 2. 分配AVIOContext，第三个参数write_flag为1
		AVIOContext *avio_out = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 1, &bd, NULL, &lhs_write_packet, NULL); 
		if (!avio_out) {
			ret = AVERROR(ENOMEM);
			Print2File("!avio_out");
			Print2File("ret = AVERROR(ENOMEM) ERR : "+std::to_string(ret));
			return false;
		}
		// 3. 分配AVFormatContext，并指定AVFormatContext.pb字段。必须在调用avformat_write_header()之前完成
		const char *outPutName = "flv";
		AVOutputFormat* outPutFormatPtr = av_guess_format(outPutName, outPutName, NULL);
		ret = avformat_alloc_output_context2(&ic, outPutFormatPtr, NULL, NULL);
		if(ret<0){
			av_make_error_string(err_buf, err_buf_size, ret);
			Print2File(err_buf);
			Print2File("Could not create output context ret: "+std::to_string(ret));
			return false;
		}
		ic->pb=avio_out;
		return true;
	}

	bool WriteHeader(){
		return true;
	}

  /**
     Write the data into the media file
     @useless in LiveSystem
   */
  bool SendFrame(XData d, int streamIndex)
	{
		if (!d.data || d.size <= 0)return false;
		AVPacket *pack = (AVPacket *)d.data;
		pack->stream_index = streamIndex;
		AVRational stime;
		AVRational dtime;
		//判断是音频还是视频
		if (vs && vc && pack->stream_index == vs->index)
		{
			stime = vc->time_base;
			dtime = vs->time_base;
		}
		else if(as && ac &&pack->stream_index == as->index)
		{
			stime = ac->time_base;
			dtime = as->time_base;
		}
		else
		{
			return false;
		}
		//推流
		pack->pts = av_rescale_q(pack->pts, stime, dtime);
		pack->dts = av_rescale_q(pack->dts, stime, dtime);
		pack->duration = av_rescale_q(pack->duration, stime, dtime);
		int ret = av_interleaved_write_frame(ic, pack);//其中源码包含 av_packet_unref()
		if (ret == 0)
		{
			// cout << "#" << flush;
			return true;
		}
		return false;
	}

private:

};
#endif // LIVE_XTRANSPORT_H
