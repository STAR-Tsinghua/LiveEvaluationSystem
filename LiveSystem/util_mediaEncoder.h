#ifndef UTIL_MEDIAENCODER_H
#define UTIL_MEDIAENCODER_H
#pragma once
#include <live_xData.h>

extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

class  AVCodecContext;
enum XSampleFMT
{
	X_S16 = 1,
	X_FLAPT = 8
};
//enum AVSampleFormat {
//	AV_SAMPLE_FMT_NONE = -1,
//	AV_SAMPLE_FMT_U8,          ///< unsigned 8 bits
//	AV_SAMPLE_FMT_S16,         ///< signed 16 bits
//	AV_SAMPLE_FMT_S32,         ///< signed 32 bits
//	AV_SAMPLE_FMT_FLT,         ///< float
//	AV_SAMPLE_FMT_DBL,         ///< double
//	AV_SAMPLE_FMT_U8P,         ///< unsigned 8 bits, planar
//	AV_SAMPLE_FMT_S16P,        ///< signed 16 bits, planar
//	AV_SAMPLE_FMT_S32P,        ///< signed 32 bits, planar
//	AV_SAMPLE_FMT_FLTP,        ///< float, planar
//	AV_SAMPLE_FMT_DBLP,        ///< double, planar
//	AV_SAMPLE_FMT_S64,         ///< signed 64 bits
//	AV_SAMPLE_FMT_S64P,        ///< signed 64 bits, planar
//	AV_SAMPLE_FMT_NB           ///< Number of sample formats. DO NOT USE if linking dynamically
//};

//音视频编码接口类
class MediaEncoder
{
public:
	//输入参数
	int inWidth = 1280;
	int inHeight = 720;
	int inPixSize = 3;
	int channels = 2;
	int sampleRate = 44100;
	XSampleFMT inSampleFmt = X_S16;
	//输出参数
	int outWidth = 1280;
	int outHeight = 720;
	int bitrate = 4000000;//压缩后每一秒视频的bit位大小50KB
	int fps = 60;//帧率
	int nbSample = 1024;
	XSampleFMT outSampleFmt = X_FLAPT;
	long long lasta = -1;
	MediaEncoder(){};
	~MediaEncoder(){};
	MediaEncoder(const MediaEncoder&){};
	MediaEncoder& operator=(const MediaEncoder&);
	//工厂的生产方法
	// static XMediaEncode *Get(unsigned char index = 0);
	//返回值无需调用者清理
	XData RGBToYUV(XData d)
	{
		XData r;
		r.pts = d.pts;
		//rgb to yuv
		//输入的数据结构 usb测有问题
		uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
		//indata[0] bgrbgrbgr
		//plane indata[0] bbbbb indata[1] ggggg indata[2] rrrrr
		indata[0] = (uint8_t*)d.data;
		int insize[AV_NUM_DATA_POINTERS] = { 0 };
		//一行（宽）数据的字节数
		insize[0] = inWidth * inPixSize;

		int h = sws_scale(vsc, indata, insize, 0, inHeight,//源数据
			yuv->data, yuv->linesize);
		if (h <= 0)
		{
			return r;
		}
		yuv->pts = d.pts;
		//cout << h << " " << flush;
		r.data = (char*)yuv;
		int *p = yuv->linesize;
		while ((*p))
		{
			r.size += (*p)*outHeight;
			p++;
		}
		return r;
	}

	//初始化像素格式转换的上下文初始化
	bool InitScale()
	{
		//2 初始化格式转换上下文
		vsc = sws_getCachedContext(vsc,
			inWidth, inHeight,//源宽高
			AV_PIX_FMT_BGR24,
			outWidth, outHeight,
			AV_PIX_FMT_YUV420P,
			SWS_BICUBIC,//尺寸变化使用算法
			0, 0, 0
		);

		if (!vsc)
		{
			Print2File("sws_getCachedContext failed!");
			return false;
		}
		//3 初始化输出的数据结构
		yuv = av_frame_alloc();
		yuv->format = AV_PIX_FMT_YUV420P;
		yuv->width = inWidth;
		yuv->height = inHeight;
		yuv->pts = 0;
		//分配yuv空间 usb不可用
		av_frame_get_buffer(yuv, 32);
		//有bug暂时注释
		//int ret = av_frame_get_buffer(yuv, 32);
		//if (ret != 0);
		//{
		//	char buf[1024] = { 0 };
		//	av_strerror(ret, buf, sizeof(buf) - 1);
		//	throw exception(buf);
		//}
		return true;
	}

	//音频重采样上下文初始化(暂时注释掉)
	bool InitResample()
	{
		// //2 音频重采样 上下文初始化
		// asc = NULL;
		// asc = swr_alloc_set_opts(asc,
		// 	av_get_default_channel_layout(channels), (AVSampleFormat)outSampleFmt, sampleRate,//输出格式
		// 	av_get_default_channel_layout(channels), (AVSampleFormat)inSampleFmt, sampleRate, 0, 0//输入格式
		// );
		// if (!asc) {
		// 	Print2File("swr_alloc_set_opts failed!");
		// 	return false;
		// }
		// int ret = swr_init(asc);
		// if (ret != 0) {
		// 	char err[1024] = { 0 };
		// 	av_strerror(ret, err, sizeof(err) - 1);
		// 	Print2File("err");
		// 	return false;
		// }

		// // cout << "音频重采样 上下文初始化 success" << endl;

		// ///3 音频重采样输出空间分配
		// pcm = av_frame_alloc();
		// pcm->format = outSampleFmt;
		// pcm->channels = channels;
		// pcm->channel_layout = av_get_default_channel_layout(channels);
		// pcm->nb_samples = nbSample;//一帧音频一通道的采样数量
		// ret = av_frame_get_buffer(pcm, 0);//给pcm分配内存空间
		// if (ret != 0) {
		// 	char err[1024] = { 0 };
		// 	av_strerror(ret, err, sizeof(err) - 1);
		// 	Print2File("err");
		// 	return false;
		// }
		return true;
	}

	//返回值无需调用者清理
	XData Resample(XData d)
	{
		XData r;
		//已经读一帧源数据
		//重采样源数据
		const uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
		indata[0] = (uint8_t *)d.data;
		int len = swr_convert(asc, pcm->data, pcm->nb_samples,//输出参数，输出存储地址和样本数量
			indata, pcm->nb_samples
		);
		if (len <= 0)
		{
			return r;
		}
		pcm->pts = d.pts;
		r.data = (char*)pcm;
		r.size = pcm->nb_samples*pcm->channels * 2;
		r.pts = d.pts;
		return r;
	}



	//编码器初始化
	bool InitVideoCodec() {
		//4 初始化编码上下文
		//a 找到编码器
		if (!(vc = CreateCodec(AV_CODEC_ID_H264)))
		{
			return false;
		}
		vc->bit_rate = 50 * 1024 * 8;//压缩后每一秒视频的bit位大小50KB
		vc->width = outWidth;
		vc->height = outHeight;
		//vc->time_base = { 1,fps };
		vc->framerate = { fps,1 };

		//画面组的大小。多少帧一个关键帧
		vc->gop_size = 50;//改过
		vc->max_b_frames = 0;
		vc->pix_fmt = AV_PIX_FMT_YUV420P;
		return OpenCodec(&vc);
	}

	//音频编码初始化
	bool InitAudioCodec() {
		if (!(ac = CreateCodec(AV_CODEC_ID_AAC)))
		{
			return false;
		}
		ac->bit_rate = 40000;
		ac->sample_rate = sampleRate;
		ac->sample_fmt = AV_SAMPLE_FMT_FLTP;
		ac->channels = channels;
		ac->channel_layout = av_get_default_channel_layout(channels);
		return OpenCodec(&ac);
	}

	//视频编码 返回值无需用户清理
	XData EncodeVideo(XData frame)
	{
		// Print2File("Before EncodeVideo : !! av_packet_unref(&vpack);");
		av_packet_unref(&vpack);
		// Print2File("After EncodeVideo : !! av_packet_unref(&vpack);");
		XData r;
		if (frame.size <= 0 || frame.data)
		{
			r;
		}
		AVFrame *p = (AVFrame *)frame.data;
		//h264编码
		//frame->pts = vpts;
		//vpts++;
		int ret = avcodec_send_frame(vc, p);
		// Print2File("int ret = avcodec_send_frame(vc, p);");
		if (ret != 0)
			return r;
		ret = avcodec_receive_packet(vc, &vpack);
		// Print2File("ret = avcodec_receive_packet(vc, &vpack);");
		if (ret != 0 || vpack.size<=0)
		{
			return r;
		}
		r.data = (char*)&vpack;
		r.size = vpack.size;
		r.pts = frame.pts;
		// Print2File("r.size : "+std::to_string(r.size));
		// Print2File("frame.pts : "+std::to_string(frame.pts));
		// Print2File("r.pts : "+std::to_string(r.pts));
		return r;
	}

	//音频编码 返回值无需用户清理
	XData EncodeAudio(XData frame)
	{
		Print2File("EncodeAudio");
		XData r;
		if (frame.size <= 0 || !frame.data)
		{
			r;
		}
		AVFrame *p = (AVFrame *)frame.data;
		if (lasta == p->pts)
		{
			//小于1000可能会丢掉，这里微妙计算，那里毫秒计算，至少大于1000
			p->pts += 1200;
		}
		lasta = p->pts;
		//pts 运算
		//nb_sample/sample_rate = 一帧音频的秒数sec
		//timebase pts = sec*timebase.den

		int ret = avcodec_send_frame(ac, p);
		if (ret != 0)return r;

		av_packet_unref(&apack);
		ret = avcodec_receive_packet(ac, &apack);
		if (ret != 0)return r;
		r.data = (char*)&apack;
		r.size = apack.size;
		r.pts = frame.pts;
		return r;
	}
	AVCodecContext *vc = 0;//编码器上下文
	AVCodecContext *ac = 0;//音频编码器上下文

protected:
	

private:
	bool OpenCodec(AVCodecContext **c)
	{
		//打开音频编码器
		int ret = avcodec_open2(*c, 0, 0);
		if (ret != 0) {
			char err[1024] = { 0 };
			av_strerror(ret, err, sizeof(err) - 1);
			Print2File("err");
			avcodec_free_context(c);
			return false;
		}
		return true;
	}
	AVCodecContext* CreateCodec(AVCodecID cid)
	{
		AVCodec *codec = avcodec_find_encoder(cid);
		if (!codec)
		{
			Print2File("avcodec_find_encoder failed");
			return NULL;
		}
		//音频编码器上下文
		AVCodecContext* c = avcodec_alloc_context3(codec);
		if (!c)
		{
			Print2File("avcodec_alloc_context3  failed");
			return NULL;
		}

		c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		// c->thread_count = XGetCpuNum();
		// TODO 这里改过，为了简化设为4，实验mac的CPU核心数，可以跨平台看CPU看有多少
		c->thread_count = 4;
		c->time_base = { 1,1000000 };
		return c;
	}
	SwsContext *vsc = NULL;//像素格式转换上下文
	SwrContext *asc = NULL;//音频重采样上下文
	AVFrame *yuv = NULL;//输出的YUV
	AVFrame *pcm = NULL;//重采样输出的PCM
	AVPacket vpack = {0};//视频帧
	AVPacket apack = {0};//音频帧
	int vpts = 0;
	int apts = 0;
};

#endif  // UTIL_MEDIAENCODER_H