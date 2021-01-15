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
#pragma comment(lib, "avformat.lib")

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
			av_register_all();
			avformat_network_init();
			isFirst = false;
		}
	};
	XTransport(const XTransport&){};
	XTransport& operator=(const XTransport&);
	void Close()
	{
		if (ic)
		{
			avformat_close_input(&ic);
			vs = NULL;
		}
		vc = NULL;
	}


	int AddStream(const AVCodecContext *c) //2
	{
		Print2File("AddStream");
		if (!c)return -1;
		Print2File("after AddStream");
		//b 添加视频流 
		AVStream *st = avformat_new_stream(ic, NULL);
		Print2File("after avformat_new_stream");
		if (!st)
		{
			Print2File("avformat_new_stream err");
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
			Print2File("if (c->codec_type == AVMEDIA_TYPE_VIDEO)");
			// Print2File("st->time_base.num : "+std::to_string(st->time_base.num));
			// Print2File("st->time_base.num : "+std::to_string(st->time_base.num));
			// Print2File("vs->time_base.num : "+std::to_string(vs->time_base.num));
        	// Print2File("vs->time_base.den : "+std::to_string(vs->time_base.den));
		}
		else if(c->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			ac = c;
			as = st;
		}
		Print2File("st->index : "+std::to_string(st->index));
		return st->index;
	}

	bool MemoryOutByFIFO(){
		uint8_t *avio_ctx_buffer = NULL;
		size_t avio_ctx_buffer_size = 4096;
		const size_t bd_buf_size = 1024;
		char *input_filename = NULL;
		int ret = 0;
		struct buffer_data_write bd = { 0 };
		/* fill opaque structure used by the AVIOContext write callback */
 		bd.ptr  = bd.buf = (uint8_t *)av_malloc(bd_buf_size);
		if (!bd.buf) {
			ret = AVERROR(ENOMEM);
			Print2File(av_err2str(ret));
			Print2File(" !bd.buf ret = AVERROR(ENOMEM) ERR : "+std::to_string(ret));
			return false;
		}
		bd.size = bd.room = bd_buf_size;
		// av_register_all();
		// 打开一个FIFO文件的写端
		// 由bd代替
		// fd = open_fifo_for_write("/tmp/test_fifo");
		// 1. 分配缓冲区
		Print2File("av_malloc");
		avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
		if (!avio_ctx_buffer) {
			ret = AVERROR(ENOMEM);
			Print2File("!avio_ctx_buffer");
			Print2File("ret = AVERROR(ENOMEM) ERR : "+std::to_string(ret));
			return false;
		}
		// obuf = av_malloc(obuf_size);
		// 2. 分配AVIOContext，第三个参数write_flag为1
		Print2File("avio_alloc_context");
		AVIOContext *avio_out = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 1, &bd, NULL, &lhs_write_packet, NULL); 
		if (!avio_out) {
			ret = AVERROR(ENOMEM);
			Print2File("!avio_out");
			Print2File("ret = AVERROR(ENOMEM) ERR : "+std::to_string(ret));
			return false;
		} 
		// 3. 分配AVFormatContext，并指定AVFormatContext.pb字段。必须在调用avformat_write_header()之前完成
		Print2File("avformat_alloc_output_context2");
		const char *outPutName = "flv";
		AVOutputFormat* outPutFormatPtr = av_guess_format(outPutName, outPutName, NULL);
		ret = avformat_alloc_output_context2(&ic, outPutFormatPtr, NULL, NULL);
		if(ret<0){
			Print2File(av_err2str(ret));
			Print2File("Could not create output context ret: "+std::to_string(ret));
			return false;
		}
		Print2File("ic->pb=avio_out;");
		ic->pb=avio_out;

		// Print2File("before if (!(ic->oformat->flags & AVFMT_NOFILE))");
		// if (!(ic->oformat->flags & AVFMT_NOFILE)) {
		// 	Print2File("if (!(ic->oformat->flags & AVFMT_NOFILE)) ");
		// 	if (avio_open(&ic->pb, NULL, AVIO_FLAG_WRITE) < 0) {
		// 		Print2File("avio_open fail");
		// 		return false;
		// 	}
		// 	Print2File("if (!(ic->oformat->flags & AVFMT_NOFILE)) end");
		// }
		Print2File("ic->pb=avio_out; return true");
		return true;
	}

	bool WriteHeader(){
		// Print2File("WriteHeader start");
		// // int ret = avio_open(&ic->pb, "flv", AVIO_FLAG_WRITE);
		// // if (ret < 0) {
		// // 	Print2File("avio_open ret < 0");
		// // 	Print2File(av_err2str(ret));
		// // 	return false;
		// // }
		// // Print2File("WriteHeader");

		// int ret;
		// av_dump_format(ic, 0, NULL, 1);
		// // 打开解封装的上下文
        // Print2File("av_dump_format(ic, 0, NULL, 1);");
		// // 写入头信息
		// /** 遇到问题：avformat_write_header()崩溃
		//  *  分析原因：没有调用avio_open()函数
		//  *  解决方案：写成 !(ouFmtCtx->flags & AVFMT_NOFILE)即可
		//  */
		
		// // // 4. 将文件头写入输出文件
		// // Print2File("avformat_write_header");
		// // AVDictionary *opt = NULL;
		// // av_dict_set(&opt, argv[2]+1, argv[3], 0);
		// if ((ret = avformat_write_header(ic, NULL)) < 0) {
		// 	Print2File("avformat_write_header fail ret : "+std::to_string(ret));
		// 	Print2File(av_err2str(ret));
		// 	return false;
		// }
		// Print2File("if ((ret = avformat_write_header(ouFmtCtx, NULL)) < 0)");
		
		// Print2File("WriteHeader End");
		return true;
	}

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

		// bool Init(const char *url) //1
	// {
	// 	///5 Êä³ö·â×°Æ÷ºÍÊÓÆµÁ÷ÅäÖÃ
	// 	//a ´´½¨Êä³ö·â×°Æ÷ÉÏÏÂÎÄ
	// 	int ret = avformat_alloc_output_context2(&ic, 0, "flv", NULL);
	// 	if (ret != 0)
	// 	{
	// 		char buf[1024] = { 0 };
	// 		av_strerror(ret, buf, sizeof(buf) - 1);
	// 		Print2File("avformat_alloc_output_context2 err");
	// 		return false;
	// 	}
	// 	Print2File("avformat_alloc_output_context2 true return");
	// 	return true;
	// }

		// bool SendHead() // 3
	// {
	// 	///打开rtmp 的网络输出IO
	// 	Print2File("打开rtmp 的网络输出IO 跳过（改）！！！！！！！！！");
	// 	// 下面一行待删除
	// 	// int ret = avformat_open_input(&ic->pb, NULL,NULL, AVIO_FLAG_WRITE);
	// 	int ret = avio_open(&ic->pb, NULL, AVIO_FLAG_WRITE);
	// 	if (ret != 0)
	// 	{
	// 		char buf[1024] = { 0 };
	// 		av_strerror(ret, buf, sizeof(buf) - 1);
	// 		Print2File("avio_open err : ");
	// 		Print2File(buf);
	// 		// cout << buf << endl;
	// 		return false;
	// 	}

	// 	//写入封装头
	// 	Print2File("写入封装头 跳过（改）11111111111");
		
	// 	int ret2 = avformat_write_header(ic, NULL);
	// 	if (ret2 != 0)
	// 	{
	// 		char buf[1024] = { 0 };
	// 		av_strerror(ret2, buf, sizeof(buf) - 1);
	// 		Print2File("avformat_write_header err : ");
	// 		Print2File(buf);
	// 		// cout << buf << endl;
	// 		return false;
	// 	}
	// 	// Print2File("SendHead return ture");
	// 	Print2File("写入封装头 跳过（改）2222222222");
	// 	return true;
	// }

private:

};
