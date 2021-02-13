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

//����Ƶ����ӿ���
class MediaEncoder
{
public:
	//�������
	int inWidth = 1280;
	int inHeight = 720;
	int inPixSize = 3;
	int channels = 2;
	int sampleRate = 44100;
	XSampleFMT inSampleFmt = X_S16;
	//�������
	int outWidth = 1280;
	int outHeight = 720;
	int bitrate = 4000000;//ѹ����ÿһ����Ƶ��bitλ��С50KB
	int fps = 60;//֡��
	int nbSample = 1024;
	XSampleFMT outSampleFmt = X_FLAPT;
	long long lasta = -1;
	MediaEncoder(){};
	~MediaEncoder(){};
	MediaEncoder(const MediaEncoder&){};
	MediaEncoder& operator=(const MediaEncoder&);
	//��������������
	// static XMediaEncode *Get(unsigned char index = 0);
	//����ֵ�������������
	XData RGBToYUV(XData d)
	{
		XData r;
		r.pts = d.pts;
		//rgb to yuv
		//��������ݽṹ usb��������
		uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
		//indata[0] bgrbgrbgr
		//plane indata[0] bbbbb indata[1] ggggg indata[2] rrrrr
		indata[0] = (uint8_t*)d.data;
		int insize[AV_NUM_DATA_POINTERS] = { 0 };
		//һ�У������ݵ��ֽ���
		insize[0] = inWidth * inPixSize;

		int h = sws_scale(vsc, indata, insize, 0, inHeight,//Դ����
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

	//��ʼ�����ظ�ʽת���������ĳ�ʼ��
	bool InitScale()
	{
		//2 ��ʼ����ʽת��������
		vsc = sws_getCachedContext(vsc,
			inWidth, inHeight,//Դ���
			AV_PIX_FMT_BGR24,
			outWidth, outHeight,
			AV_PIX_FMT_YUV420P,
			SWS_BICUBIC,//�ߴ�仯ʹ���㷨
			0, 0, 0
		);

		if (!vsc)
		{
			Print2File("sws_getCachedContext failed!");
			return false;
		}
		//3 ��ʼ����������ݽṹ
		yuv = av_frame_alloc();
		yuv->format = AV_PIX_FMT_YUV420P;
		yuv->width = inWidth;
		yuv->height = inHeight;
		yuv->pts = 0;
		//����yuv�ռ� usb������
		av_frame_get_buffer(yuv, 32);
		//��bug��ʱע��
		//int ret = av_frame_get_buffer(yuv, 32);
		//if (ret != 0);
		//{
		//	char buf[1024] = { 0 };
		//	av_strerror(ret, buf, sizeof(buf) - 1);
		//	throw exception(buf);
		//}
		return true;
	}

	//��Ƶ�ز��������ĳ�ʼ��(��ʱע�͵�)
	bool InitResample()
	{
		// //2 ��Ƶ�ز��� �����ĳ�ʼ��
		// asc = NULL;
		// asc = swr_alloc_set_opts(asc,
		// 	av_get_default_channel_layout(channels), (AVSampleFormat)outSampleFmt, sampleRate,//�����ʽ
		// 	av_get_default_channel_layout(channels), (AVSampleFormat)inSampleFmt, sampleRate, 0, 0//�����ʽ
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

		// // cout << "��Ƶ�ز��� �����ĳ�ʼ�� success" << endl;

		// ///3 ��Ƶ�ز�������ռ����
		// pcm = av_frame_alloc();
		// pcm->format = outSampleFmt;
		// pcm->channels = channels;
		// pcm->channel_layout = av_get_default_channel_layout(channels);
		// pcm->nb_samples = nbSample;//һ֡��Ƶһͨ���Ĳ�������
		// ret = av_frame_get_buffer(pcm, 0);//��pcm�����ڴ�ռ�
		// if (ret != 0) {
		// 	char err[1024] = { 0 };
		// 	av_strerror(ret, err, sizeof(err) - 1);
		// 	Print2File("err");
		// 	return false;
		// }
		return true;
	}

	//����ֵ�������������
	XData Resample(XData d)
	{
		XData r;
		//�Ѿ���һ֡Դ����
		//�ز���Դ����
		const uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
		indata[0] = (uint8_t *)d.data;
		int len = swr_convert(asc, pcm->data, pcm->nb_samples,//�������������洢��ַ����������
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



	//��������ʼ��
	bool InitVideoCodec() {
		//4 ��ʼ������������
		//a �ҵ�������
		if (!(vc = CreateCodec(AV_CODEC_ID_H264)))
		{
			return false;
		}
		vc->bit_rate = 50 * 1024 * 8;//ѹ����ÿһ����Ƶ��bitλ��С50KB
		vc->width = outWidth;
		vc->height = outHeight;
		//vc->time_base = { 1,fps };
		vc->framerate = { fps,1 };

		//������Ĵ�С������֡һ���ؼ�֡
		vc->gop_size = 50;//�Ĺ�
		vc->max_b_frames = 0;
		vc->pix_fmt = AV_PIX_FMT_YUV420P;
		return OpenCodec(&vc);
	}

	//��Ƶ�����ʼ��
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

	//��Ƶ���� ����ֵ�����û�����
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
		//h264����
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

	//��Ƶ���� ����ֵ�����û�����
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
			//С��1000���ܻᶪ��������΢����㣬���������㣬���ٴ���1000
			p->pts += 1200;
		}
		lasta = p->pts;
		//pts ����
		//nb_sample/sample_rate = һ֡��Ƶ������sec
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
	AVCodecContext *vc = 0;//������������
	AVCodecContext *ac = 0;//��Ƶ������������

protected:
	

private:
	bool OpenCodec(AVCodecContext **c)
	{
		//����Ƶ������
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
		//��Ƶ������������
		AVCodecContext* c = avcodec_alloc_context3(codec);
		if (!c)
		{
			Print2File("avcodec_alloc_context3  failed");
			return NULL;
		}

		c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		// c->thread_count = XGetCpuNum();
		// TODO ����Ĺ���Ϊ�˼���Ϊ4��ʵ��mac��CPU�����������Կ�ƽ̨��CPU���ж���
		c->thread_count = 4;
		c->time_base = { 1,1000000 };
		return c;
	}
	SwsContext *vsc = NULL;//���ظ�ʽת��������
	SwrContext *asc = NULL;//��Ƶ�ز���������
	AVFrame *yuv = NULL;//�����YUV
	AVFrame *pcm = NULL;//�ز��������PCM
	AVPacket vpack = {0};//��Ƶ֡
	AVPacket apack = {0};//��Ƶ֡
	int vpts = 0;
	int apts = 0;
};

#endif  // UTIL_MEDIAENCODER_H