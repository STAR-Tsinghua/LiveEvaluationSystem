#ifndef URL_FILE_H
#define URL_FILE_H


extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <memory>
#include <sodtp_config.h>
#include <util_mediaEncoder.h>

class StreamContext {
public:
    AVFormatContext *pFmtCtx;
    uint32_t stream_id;
    uint32_t flag_meta;

    //lhs 改进版
    AVFormatContext  *ifmt_ctx, *ofmt_ctx;
    AVInputFormat * ifmt;
    AVOutputFormat * ofmt; 

    AVStream *out_stream;
    //TODO Add mediaEncoder
    // MediaEncoder *encoder;

    //lhs 暴改参考版本
    // StreamContext(AVFormatContext *ifmt_ctxPtr, AVFormatContext *ofmt_ctxPtr,int id) {
    //     //  对ifmt_ctx初始化
    //     ifmt = av_find_input_format("h264"); //if (!(vc = CreateCodec(AV_CODEC_ID_H264)))

    //     unsigned char * iobuffer=(unsigned char *)av_malloc(32768);
    //     AVIOContext *avio =avio_alloc_context(iobuffer, 32768,0,pp,HISON_FILL_Buffer,NULL,NULL);
    //     ifmt_ctx->pb=avio;

    //     avformat_open_input(&ifmt_ctx, NULL, ifmt, NULL);

    //     avformat_find_stream_info(ifmt_ctx, 0);

    //     // ofmt_ctx的初始化
    //     avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", g_push_param->DestAddress);// bool Init(const char *url) //1

    //     out_stream = avformat_new_stream(ofmt_ctx, NULL);// int AddStream(const AVCodecContext *c) //2

    //     avcodec_parameters_copy(out_stream->codecpar, in_codecpar)// xPlayer

    //     avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);// bool SendHead() // 3
    // }

    // TODO 改掉它，不然变废代码
    StreamContext(AVFormatContext *ptr, int id) {
        Print2File("StreamContext(AVFormatContext *ptr, int id)");
        if(ptr==NULL){
            Print2File("StreamContext ptr==NULL");
        }else{
            Print2File("StreamContext ptr!=NULL");

        }
        pFmtCtx = ptr;
        stream_id = id;
        flag_meta = false;
        if(pFmtCtx->streams[0]!=NULL){
            Print2File("pFmtCtx->streams[0] != NULL");
        }else{
            Print2File("pFmtCtx->streams[0] == NULL");
        }
        Print2File("ptr->streams->time_base.num : "+std::to_string(pFmtCtx->streams[0]->time_base.num));
        Print2File("ptr->streams->time_base.den : "+std::to_string(pFmtCtx->streams[0]->time_base.den));
        Print2File("End StreamContext(AVFormatContext *ptr, int id) End========");

//         StreamContext ptr!=NULL
// pFmtCtx->streams[0] != NULL
// ptr->streams->time_base.num : 0
// ptr->streams->time_base.den : 0
    }

    ~StreamContext() {
        if (pFmtCtx) {
            avformat_close_input(&pFmtCtx);
        }
    }
};

typedef std::shared_ptr<StreamContext> StreamCtxPtr;

int init_AVFormatContext(
    AVFormatContext         **pFormatCtx,
    const char              *pFilename) {

    // FILE *file;
    // file = fopen(pFilename,"r");
    // if (!file) {
    //     printf("Fail to open file!\n");
    //     printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
    // }

    // Open video file
    if (avformat_open_input(pFormatCtx, pFilename, NULL, NULL) != 0)
        return -1; // Couldn't open file

    // Retrieve stream information
    if (avformat_find_stream_info(*pFormatCtx, NULL) < 0)
        return -1; // Couldn't find stream information

    // Dump information about file onto standard error
    av_dump_format(*pFormatCtx, 0, pFilename, 0);

    return 0;
}



int file_read_packet(
    FILE                    *file,
    AVCodecParserContext    *pVCodecParserCtx,
    AVCodecContext          *pVCodecCtx,
    AVPacket                *pPacket) {

#define BUFFER_SIZE 4096

    static uint8_t      buffer[BUFFER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    static int          initBuffer = 0;
    static size_t       data_size = 0;
    static uint8_t      *data;
    int                 ret;

    // set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams)
    if (!initBuffer) {
        initBuffer = 1;
        memset(buffer + BUFFER_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
    }

    // Flush the parser, or output the left frame in parser.
    // This code flushes the parser in two case:
    // a) when there are left data in parser;
    // b) when EOF, the last frame might not be parsed.
    while (data_size > 0) {
        ret = av_parser_parse2(pVCodecParserCtx, pVCodecCtx, &pPacket->data, &pPacket->size,
                                data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            fprintf(stderr, "Error while parsing\n");
            return -1;
        }
        data      += ret;
        data_size -= ret;
 
        if (pPacket->size) {
            printf("packet size = %d\n", pPacket->size);
            // printf("packet size = %d,\t sum = %d\n", pPacket->size, sum);
            return 1;
        }
    }

    while (!feof(file)) {
        // read raw data from the input file
        data_size = fread(buffer, 1, BUFFER_SIZE, file);
        if (!data_size) {
            printf("debug: %s:%s():%d\n", __FILE__, __FUNCTION__, __LINE__);
            return -2;
        }

        // use the parser to split the data into frames
        data = buffer;
        while (data_size > 0) {
            ret = av_parser_parse2(pVCodecParserCtx, pVCodecCtx, &pPacket->data, &pPacket->size,
                                   data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0) {
                fprintf(stderr, "Error while parsing\n");
                return -1;
            }
            data      += ret;
            data_size -= ret;

            // printf("Packet Seq Num:%d\t", pVCodecParserCtx->output_picture_number);
            // printf("KeyFrame:%d pts:%d, dts:%d, duration:%d\t", pVCodecParserCtx->key_frame, 
            //     pVCodecParserCtx->pts, pVCodecParserCtx->dts, pVCodecParserCtx->duration);

            if (pPacket->size) {
            // if (pPacket->size && pPacket->stream_index == videoStream) {
                printf("packet size = %d\n", pPacket->size);
                // printf("packet size = %d,\t sum = %d\n", pPacket->size, sum);
                return 1;
            }
        }
    }


    // flush the parser.
    if (feof(file)) {
        av_parser_parse2(pVCodecParserCtx, pVCodecCtx, &pPacket->data, &pPacket->size,
                                       NULL, 0, 0, 0, 0);
        if (pPacket->size) {
            printf("flush the parser.\n");
            printf("packet size = %d\n", pPacket->size);
            return 1;
        }

        // End of Stream.
        return -4;
    }

    // Something is wrong.
    // E.g. the file may contain an incomplete frame.
    return -10;
}


static inline int file_read_packet2(
    AVFormatContext         *pFormatCtx,
    AVPacket                *pPacket) {

    return av_read_frame(pFormatCtx, pPacket);
}


// @deprecated
int file_read_packet3(
    const char              *pFilename,
    AVPacket                *pPacket) {


    static AVFormatContext          *pFormatCtx = NULL;
    static AVCodecContext           *pVCodecCtx = NULL;
    static AVCodec                  *pVCodec = NULL;
    static int                      i, videoStream;
    static int                      initContext = 0;

    if (!initContext) {
        initContext = 1;

        // Open video file
        if (avformat_open_input(&pFormatCtx, pFilename, NULL, NULL) != 0)
            return -1; // Couldn't open file

        // Retrieve stream information
        if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
            return -1; // Couldn't find stream information

        // Dump information about file onto standard error
        av_dump_format(pFormatCtx, 0, pFilename, 0);

        // Find the first video stream
        videoStream = -1;
        for (i = 0; i < pFormatCtx->nb_streams; i++) {
            if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStream = i;
                break;
            }
        }
        if (videoStream == -1)
            return -1; // Didn't find a video stream

        // Find the decoder for the video stream
        // pVCodec = avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
        // if (pVCodec == NULL) {
        //     fprintf(stderr, "Unsupported codec!\n");
        //     return -1; // Codec not found
        // }
        // printf("H265 = %d, Codec type = %d\n", AV_CODEC_ID_H265, pVCodec->id);

        pVCodecCtx = avcodec_alloc_context3(pVCodec);
        if (!pVCodecCtx) {
            fprintf(stderr, "Could not allocate video codec context\n");
            return -1;
        }
        pVCodecCtx->thread_count = 1;
        avcodec_parameters_to_context(pVCodecCtx, pFormatCtx->streams[videoStream]->codecpar);

        pVCodec = avcodec_find_decoder(pVCodecCtx->codec_id);
        printf("H265 = %d, Codec type = %d\n", AV_CODEC_ID_H265, pVCodec->id);

        // Open Codec
        if (avcodec_open2(pVCodecCtx, pVCodec, NULL) < 0) {
            printf("fail to open codec!\n");
            return -1;
        }

    }

    // av_packet_unref(&packet);
    return av_read_frame(pFormatCtx, pPacket);
}

// @deprecated
int file_read_packet4(
    const char              *pFilename,
    AVPacket                *pPacket) {


    static AVFormatContext          *pFormatCtx = NULL;
    static int                      initContext = 0;

    if (!initContext) {
        initContext = 1;

        // Open video file
        if (avformat_open_input(&pFormatCtx, pFilename, NULL, NULL) != 0)
            return -1; // Couldn't open file

        // Retrieve stream information
        if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
            return -1; // Couldn't find stream information

        // Dump information about file onto standard error
        av_dump_format(pFormatCtx, 0, pFilename, 0);
    }

    // av_packet_unref(&packet);
    return av_read_frame(pFormatCtx, pPacket);
}


// @deprecated
int sodtp_read_jitter_packet() {

#define BUFFER_SIZE 4096

    static uint8_t      buffer[BUFFER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    static int          initBuffer = 0;
    static size_t       data_size = 0;
    static uint8_t      *data;
    int                 ret;


    return 0;
}



void init_resource(std::vector<AVFormatContext*> *pFmtCtxs) {
    const char files[][100] = {
        "/Users/yuming/Movies/bigbuckbunny/480x272_1.h265",
        "/Users/yuming/Movies/bigbuckbunny/480x272_2.h265",
        "/Users/yuming/Movies/bigbuckbunny/480x272_3.h265",
        // "/Users/lihongsheng/Desktop/About_DTP/movies/sample_0.h264",
        // "/Users/lihongsheng/Desktop/About_DTP/movies/sample_1.h264",
        // "/Users/lihongsheng/Desktop/About_DTP/movies/sample_2.h264",
    };

    for (auto file : files) {
        AVFormatContext *ptr = NULL;

        if (init_AVFormatContext(&ptr, file) < 0) {
            fprintf(stderr, "Could not init format context by file %s\n", file);
            if (ptr) {
                avformat_close_input(&ptr);
            }
        }
        else {
            printf("init resource: %s\n", file);
            pFmtCtxs->push_back(ptr);
        }
    }
}

void init_resource(std::vector<AVFormatContext*> *pFmtCtxs, const char *conf) {
    StreamConfig stc;
    stc.parse(conf);


    printf("stream number: %lu\n", stc.files.size());
    for (auto &file : stc.files) {
        AVFormatContext *ptr = NULL;

        if (init_AVFormatContext(&ptr, file.c_str()) < 0) {
            fprintf(stderr, "Could not init format context by file %s\n", file.c_str());
            if (ptr) {
                avformat_close_input(&ptr);
            }
        }
        else {
            printf("init resource: %s\n", file.c_str());
            pFmtCtxs->push_back(ptr);
        }
    }
}

void init_resource(AVFormatContext **pFmtCtx, const char *filename) {
    if (init_AVFormatContext(pFmtCtx, filename) < 0) {
        fprintf(stderr, "Could not init format context by file %s\n", filename);
        *pFmtCtx = NULL;
    }
    else {
        printf("init resource: %s\n", filename);
    }
}

int init_Live_AVFormatContext(
    AVFormatContext         **pFormatCtx,
    const char              *pFilename) {
    Print2File("init_Live_AVFormatContext");



    // 下面是不成功的案例
    //&ic, 0, "flv", url
    // int ret = avformat_alloc_output_context2(pFormatCtx, NULL, NULL, pFilename);
    // int ret = avformat_alloc_output_context2(pFormatCtx, 0, "flv", pFilename);
    // if (ret != 0)
	// {
    //     Print2File("init_Live_AVFormatContext return -1");
    //     return -1;
	// }
//================= Old ==================
    // // Open video file
    // if (avformat_open_input(pFormatCtx, pFilename, NULL, NULL) != 0)
    //     return -1; // Couldn't open file

    // // Retrieve stream information
    // if (avformat_find_stream_info(*pFormatCtx, NULL) < 0)
    //     return -1; // Couldn't find stream information

    // // Dump information about file onto standard error
    // av_dump_format(*pFormatCtx, 0, pFilename, 0);
//================= Old ==================
    Print2File("init_Live_AVFormatContext return 0");
    return 0;
}

//暂时废除
int addStream(AVFormatContext *fc, const AVCodecContext *vc, AVStream *&vs, const char * pFilename)
{
    //  TODO
    //  这个函数将来可能重构，保存AVStream和AVCodecContext等信息
    if (!vc){
        Print2File("AVCodecContext == NULL");
        return -1;
    }
    if(fc){
        Print2File("fc != NULL");
    }else{
        Print2File("fc == NULL");
    }
    //b 添加视频流 
    vs = avformat_new_stream(fc, NULL);
    if (!vs)
    {
        Print2File("avformat_new_stream failed");
        return -1;
    }
    
    vs->codecpar->codec_tag = 0;
    //从编码器复制参数
    // Print2File("avcodec_parameters_from_context(st->codecpar, vc)");
    avcodec_parameters_from_context(vs->codecpar, vc);
    // Print2File("av_dump_format(fc, 0, pFilename, 1)");
    //  这里注意最后一个参数如果是 0 程序就停住了
    av_dump_format(fc, 0, pFilename, 1);
    // Print2File("st->index : "+std::to_string(st->index));
    //指定视频流
    if (vc->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        Print2File("vc->codec_type == AVMEDIA_TYPE_VIDEO");
    }
    else if(vc->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        Print2File("vc->codec_type == AVMEDIA_TYPE_AUDIO");
    }
    return vs->index;
}

bool set_StmCtxPtrsAndId(std::vector<StreamCtxPtr> *pStmCtxPtrs, AVFormatContext *fc){
    int id = 0;
    Print2File("cptr = std::make_shared<StreamContext>(ptr, id); id: "+std::to_string(id));
    StreamCtxPtr cptr = std::make_shared<StreamContext>(fc, id);
    if(cptr==NULL){
        Print2File("ptr==NULL return false");
        return false;
    }
    pStmCtxPtrs->push_back(cptr);
    id++;
    return true;
}
// lhs修改后的版本1
bool init_live_resource1(std::vector<StreamCtxPtr> *pStmCtxPtrs, AVCodecContext *vc, AVStream *&vs){
    Print2File("init_live_resource");
    int id = 0;
    StreamCtxPtr cptr = NULL;
    AVFormatContext *ptr = NULL;
    //  初始化 AVFormatContext
    const char * lhs_dtp = "lhs_dtp";
    if(init_Live_AVFormatContext(&ptr, lhs_dtp) < 0){
        if (ptr) {
            avformat_close_input(&ptr);
            return false;
        }
        return false;
    }else{
        if(ptr==NULL){
            Print2File("ptr==NULL return false");
            return false;
        }
        Print2File("ptr!=NULL return false");
        Print2File("cptr = std::make_shared<StreamContext>(ptr, id); id: "+std::to_string(id));
        cptr = std::make_shared<StreamContext>(ptr, id);
        // Print2File("init_live_resource : pStmCtxPtrs->push_back(cptr);");
        pStmCtxPtrs->push_back(cptr);
        id++;
    }
    Print2File("添加视频流");
    //  添加视频流
	int vindex = 0;
	vindex = addStream(ptr,vc,vs,lhs_dtp);
	if (vindex<0)
	{
        Print2File("addStream video Fail");
		return false;
	}
    Print2File("vindex>=0");
    Print2File("vindex : "+std::to_string(vindex));
    //Log 出来是 0 需要注意一下
    return true;
}

//真正使用的init_resource
void init_resource(std::vector<StreamCtxPtr> *pStmCtxPtrs, const char *conf) {
    StreamConfig stc;

    stc.parse(conf);
    printf("stream number: %lu\n", stc.files.size());

    int id = 0;
    StreamCtxPtr cptr = NULL;
    AVFormatContext *ptr = NULL;

    for (auto &file : stc.files) {
        // It is necessary to set ptr = NULL, or else we will fail to open format context.
        ptr = NULL;

        Print2File("file.c_str() : "+file);
        if (init_AVFormatContext(&ptr, file.c_str()) < 0) {
            Print2File("if (init_AVFormatContext(&ptr, file.c_str()) < 0) ");
            fprintf(stderr, "Could not init format context by file %s\n", file.c_str());
            if (ptr) {
                Print2File("avformat_close_input(&ptr);");
                avformat_close_input(&ptr);
            }
        }
        else {
            Print2File("if (init_AVFormatContext(&ptr, file.c_str()) < 0) else!!!!!");
            printf("init resource: %s\n", file.c_str());
            cptr = std::make_shared<StreamContext>(ptr, id);
            Print2File("pStmCtxPtrs->push_back(cptr); 111111111111");
            pStmCtxPtrs->push_back(cptr);

            id++;
        }
    }
}

void init_resource(std::vector<StreamContext*> *pStmCtxPtrs, const char *conf) {
    StreamConfig stc;

    stc.parse(conf);
    printf("stream number: %lu\n", stc.files.size());

    int id = 0;
    StreamContext *cptr = NULL;
    AVFormatContext *ptr = NULL;

    for (auto &file : stc.files) {
        // It is necessary to set ptr = NULL, or else we will fail to open format context.
        ptr = NULL;

        if (init_AVFormatContext(&ptr, file.c_str()) < 0) {
            fprintf(stderr, "Could not init format context by file %s\n", file.c_str());
            if (ptr) {
                avformat_close_input(&ptr);
            }
        }
        else {
            printf("init resource: %s\n", file.c_str());
            cptr = new StreamContext(ptr, id);
            Print2File("pStmCtxPtrs->push_back(cptr); 222222222222");
            pStmCtxPtrs->push_back(cptr);

            id++;
        }
    }
}




bool compare_packet(AVPacket *ptr1, AVPacket *ptr2) {
    bool ret = false;
    if (ptr1->size == ptr2->size) {
        ret = (0 == memcmp(ptr1->data, ptr2->data, ptr1->size));
    }

    return ret;
}


#endif  // URL_FILE_H