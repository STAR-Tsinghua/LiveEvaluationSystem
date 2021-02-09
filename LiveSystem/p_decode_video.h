#ifndef P_DECODE_VIDEO_H
#define P_DECODE_VIDEO_H

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <stdio.h>
#include <ev.h>
#include <sys/stat.h>
#include <util_url_file.h>
#include <sodtp_config.h>
#include <p_sodtp_jitter.h>
#include <p_sdl_play.h>
#include <p_sodtp_decoder.h>


#undef av_err2str
#define av_err2str(errnum) av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)

void SaveFrame(AVFrame *pFrame, int width, int height, int iStream, int iFrame) {
    FILE *pFile;
    char path[128];

    // // Open file
    // sprintf(dir, "/Users/yuming/Movies/decode/%d", iStream);
    // sprintf(path, "%s/frame%d.ppm", dir, iFrame);
    // // sprintf(szFilename, "/Users/yuming/Movies/decode/%d/frame%d.ppm", iStream, iFrame);
    // if (access(dir, 0) == -1)
    // {
    //     mkdir(dir, 0775);
    // }

    sprintf(path, "/Users/yuming/Movies/decode/%d", iStream);
    if (access(path, 0) == -1)
    {
        mkdir(path, 0775);
    }
    sprintf(path, "%s/frame%d.ppm", path, iFrame);

    pFile=fopen(path, "wb");
    if(pFile==NULL)
        return;

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for(int y = 0; y < height; y++)
        fwrite(pFrame->data[0] + y*pFrame->linesize[0], 1, width*3, pFile);

    // Close file
    fclose(pFile);
}

void SaveFrame(Decoder *decoder) {
    FILE            *pFile;
    AVCodecContext  *ctx    = decoder->pVCodecCtx;
    AVFrame         *pFrame = decoder->pFrameRGB;
    char path[128];

    if (access(decoder->path, 0) == -1)
    {
        if (mkdir(decoder->path, 0775) < 0) {
            fprintf(stderr, "mkdir: fail to create %s\n", decoder->path);
            return;
        }
    }

    sprintf(path, "%s/%d", decoder->path, decoder->iStream);
    if (access(path, 0) == -1)
    {
        if (mkdir(path, 0775) < 0) {
            fprintf(stderr, "mkdir: fail to create %s\n", path);
            return;
        }
    }

    sprintf(path, "%s/frame%d.ppm", path, decoder->iFrame);
    pFile = fopen(path, "wb");
    if(pFile == NULL) {
        return;
    }

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", ctx->width, ctx->height);

    // Write pixel data
    for(int y = 0; y < ctx->height; y++)
        fwrite(pFrame->data[0] + y*pFrame->linesize[0], 1, ctx->width*3, pFile);

    // Close file
    fclose(pFile);
}

void SavePGM(unsigned char *buf, int wrap, int xsize, int ysize,
                     char *filename)
{
    FILE *file;
    int i;

    file = fopen(filename,"w");
    printf("%s\n", filename);
    if (!file) {
        fprintf(stderr, "Fail to open file!\n");
    }

    fprintf(file, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, file);
    fclose(file);
}

// Warning! We do not flush avcodec to get the remaining frames.
// So streamer needs to make sure each frame can be decoded directly without
// pushing a empty packet into codec.
void DecodePacket(AVCodecContext    *pVCodecCtx,
            struct SwsContext       *pSwsCtx,
            AVFrame                 *pFrame,
            AVFrame                 *pFrameRGB,
            AVPacket                *pPacket,
            int32_t                 iStream,
            int32_t                 iFrame)
{
    int ret;

    ret = avcodec_send_packet(pVCodecCtx, pPacket);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(pVCodecCtx, pFrame);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
            // printf("imcomplete picture: stream %d,\t frame %d\n", iStream, iFrame);
            return;
        }
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }

        printf("saving: stream %d,\t frame %d,\t codec frame %d\n",
            iStream, iFrame, pVCodecCtx->frame_number);
        fflush(stdout);


        // Convert the image from its native format to RGB
        sws_scale(pSwsCtx, (uint8_t const * const *)pFrame->data,
                pFrame->linesize, 0, pVCodecCtx->height,
                pFrameRGB->data, pFrameRGB->linesize);
        if (iFrame < 2000) {
            SaveFrame(pFrameRGB, pVCodecCtx->width, pVCodecCtx->height, iStream, iFrame);
        }
    }
}

// Warning! We do not flush avcodec to get the remaining frames.
// So streamer needs to make sure each frame can be decoded directly without
// pushing a empty packet.
void DecodePacket(Decoder *decoder)
{
    AVCodecContext      *pVCodecCtx = decoder->pVCodecCtx;
    struct SwsContext   *pSwsCtx    = decoder->pSwsCtx;
    AVFrame             *pFrame     = decoder->pFrame;
    AVFrame             *pFrameRGB  = decoder->pFrameRGB;
    AVPacket            *pPacket    = decoder->pPacket;
    int32_t             iStream     = decoder->iStream;
    int32_t             iFrame      = decoder->iFrame;


    int ret = avcodec_send_packet(pVCodecCtx, pPacket);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(pVCodecCtx, pFrame);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
            // printf("decoding once over : stream %d,\t frame %d\n", iStream, iFrame);
            return;
        }
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }

        printf("saving: stream %d,\t frame %d,\t codec frame %d\n",
            iStream, iFrame, pVCodecCtx->frame_number);
        fflush(stdout);


        if (!decoder->path) {
            // do not save the picture unless we need to.
            continue;
        }

        // Convert the image from its native format to RGB
        sws_scale(pSwsCtx, (uint8_t const * const *)pFrame->data,
                pFrame->linesize, 0, pVCodecCtx->height,
                pFrameRGB->data, pFrameRGB->linesize);
        if (iFrame < 2000) {
            SaveFrame(decoder);
        }
    }
}

void DecodePacketPlay(Decoder *decoder)
{
    AVCodecContext      *pVCodecCtx = decoder->pVCodecCtx;
    struct SwsContext   *pSwsCtx    = decoder->pSwsCtx;
    AVFrame             *pFrame     = decoder->pFrame;
    AVFrame             *pFrameYUV  = decoder->pFrameYUV;
    AVPacket            *pPacket    = decoder->pPacket;
    int32_t             iStream     = decoder->iStream;
    int32_t             iFrame      = decoder->iFrame;


    int ret = avcodec_send_packet(pVCodecCtx, pPacket);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(pVCodecCtx, pFrame);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // fprintf(stderr, "debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
            // printf("decoding once over : stream %d,\t frame %d\n", iStream, iFrame);
            return;
        }
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }

        printf("saving: stream %d,\t frame %d,\t codec frame %d\n",
            iStream, iFrame, pVCodecCtx->frame_number);
        fflush(stdout);

        // Convert the image from its native format to YUV
        sws_scale(pSwsCtx, (uint8_t const * const *)pFrame->data,
                pFrame->linesize, 0, pVCodecCtx->height,
                pFrameYUV->data, pFrameYUV->linesize);

        {
            scoped_lock lock(decoder->mutex);
            decoder->iStart = 1;
            AVFrame *tmp = decoder->pFrameYUV;
            decoder->pFrameYUV = decoder->pFrameShow;
            decoder->pFrameShow = tmp;
        }
    }
}

// Warning!!!
// Buffer size should be added AV_INPUT_BUFFER_PADDING_SIZE.
// Warning!!!
int SodtpReadPacket(
    SodtpJitterPtr          pJitter,
    // AVCodecParserContext    *pVCodecParserCtx,
    // AVCodecContext          *pVCodecCtx,
    AVPacket                *pPacket) {


    int ret;
    SodtpBlockPtr pBlock;
    ret = pJitter->pop(pBlock);

    // Warning!!!
    // Block buffer size should be size + AV_INPUT_BUFFER_PADDING_SIZE
    // Warning!!!
    //
    //
    // if (pBlock) {
    if (ret == SodtpJitter::STATE_NORMAL) {
        av_packet_from_data(pPacket, pBlock->data, pBlock->size);
    }
    //
    //
    //
    return ret;
}

int SodtpReadPacket(
    SodtpJitterPtr          pJitter,
    AVPacket                *pPacket,
    SodtpBlockPtr           &pBlock) {


    int ret;
    ret = pJitter->pop(pBlock);
    if (!pBlock) {
        pPacket->data = NULL;
        pPacket->size = 0;
        return ret;
    }

    // Warning!!!
    // Block buffer size should be size + AV_INPUT_BUFFER_PADDING_SIZE
    // Warning!!!
    //
    //
    // if (pBlock) {
    if (ret == SodtpJitter::STATE_NORMAL) {
        av_packet_from_data(pPacket, pBlock->data, pBlock->size);
    } else {
        fprintf(stderr, "jitter is buffering now.\n");
    }

    return ret;
}

int SodtpReadPacket(
    SodtpJitter             *pJitter,
    AVPacket                *pPacket,
    SodtpBlockPtr           &pBlock) {

    // if(pPacket->flags & AV_PKT_FLAG_KEY){
    //     Print2File("key frame===========");
    // }else{
    //     Print2File("Not key frame============");//跑这里
    // }


    int ret;
    ret = pJitter->pop(pBlock);
    if (!pBlock) {
        pPacket->data = NULL;
        pPacket->size = 0;
        return ret;
    }

    // Warning!!!
    // Block buffer size should be size + AV_INPUT_BUFFER_PADDING_SIZE
    // Warning!!!
    //
    //
    // if (pBlock) {
    if (ret == SodtpJitter::STATE_NORMAL) {
        av_packet_from_data(pPacket, pBlock->data, pBlock->size);
    }
    // else {
    //     fprintf(stderr, "jitter is buffering now.\n");
    // }

    return ret;
}


// int video_peer(int argc, char *argv[]) {
int video_viewer1(SodtpJitterPtr pJitter) {

    printf("video viewer once!\n");

    // Initalizing these to NULL prevents segfaults!
    AVCodecContext          *pVCodecCtx = NULL;
    AVCodec                 *pVCodec = NULL;
    AVFrame                 *pFrame = NULL;
    AVFrame                 *pFrameRGB = NULL;
    AVPacket                packet;
    int                     i, ret;
    int                     numBytes;
    uint8_t                 *buffer = NULL;
    struct SwsContext       *pSwsCtx = NULL;

    // Register all formats and codecs
    // av_register_all();

    pVCodec = avcodec_find_decoder(AV_CODEC_ID_H265);
    if (!pVCodec) {
        fprintf(stderr, "Codec not found\n");
        return -1;
    }

    pVCodecCtx = avcodec_alloc_context3(pVCodec);
    if (!pVCodecCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return -1;
    }
    pVCodecCtx->thread_count = 1;

    // Open Codec
    if (avcodec_open2(pVCodecCtx, pVCodec, NULL) < 0) {
        fprintf(stderr, "Fail to open codec!\n");
        return -1;
    }

    // Allocate video frame
    pFrame = av_frame_alloc();
    // Allocate an AVFrame structure
    pFrameRGB = av_frame_alloc();
    if (pFrame == NULL || pFrameRGB == NULL)
        return -1;

    // Determine required buffer size and allocate buffer
    printf("hahaha1\n");
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pVCodecCtx->width, pVCodecCtx->height, 1);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    printf("hahaha2\n");

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer,
        AV_PIX_FMT_RGB24, pVCodecCtx->width, pVCodecCtx->height, 1);
    printf("hahaha3\n");

    // initialize SWS context for software scaling
    pSwsCtx = sws_getContext(pVCodecCtx->width,
                            pVCodecCtx->height,
                            pVCodecCtx->pix_fmt,
                            pVCodecCtx->width,
                            pVCodecCtx->height,
                            AV_PIX_FMT_RGB24,
                            SWS_BILINEAR,
                            NULL,
                            NULL,
                            NULL
                            );
    printf("hahaha4\n");
    av_init_packet(&packet);

    // Warning!!!
    // SodtpReadPacket() might be wrongly implemented.
    // Need to double check.
    // Warning!!!
    // This should also be blocked!
    i = 0;
    while (true) {
        ret = SodtpReadPacket(pJitter, &packet);
        printf("hahaha\n");
        if (ret == SodtpJitter::STATE_NORMAL) {
            printf("packet size %d\n", packet.size);
            DecodePacket(pVCodecCtx, pSwsCtx, pFrame, pFrameRGB, &packet, 0, ++i);
        }
        if (pJitter->state == SodtpJitter::STATE_CLOSE) {
            // Stream is closed.
            // Thread will be closed by breaking this loop.
            fprintf(stderr, "Stream is closed!\n");
            break;
        }
    }
    ///
    ///


    // Free the RGB image
    av_free(buffer);
    av_frame_free(&pFrameRGB);

    // Free the YUV frame
    av_frame_free(&pFrame);

    // Close the codecs
    avcodec_close(pVCodecCtx);

    // Notification for the jitter clearing.
    sem_post(pJitter->_sem);
    ev_feed_signal(SIGUSR1);

    return 0;
}

struct buffer_data {
    uint8_t *ptr;
    size_t size; ///< size left in the buffer
};

int read_buffer(void *opaque, uint8_t *buf, int buf_size)
{
    struct buffer_data *bd = (struct buffer_data *)opaque;
    buf_size = FFMIN(buf_size, bd->size);

    if (!buf_size)
        return AVERROR_EOF;
    fprintf(stderr, "ptr:%p size:%zu\n", bd->ptr, bd->size);

    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr  += buf_size;
    bd->size -= buf_size;

    return buf_size;
}

AVFormatContext* sniff_format2(uint8_t *data, size_t size) {
    Print2File("AVFormatContext* sniff_format2(uint8_t *data, size_t size) !!!!!!!!!!!!!!!!!!!!!!!!!!!");
    AVFormatContext *fmt_ctx = NULL;
    AVIOContext *avio_ctx = NULL;
    AVInputFormat* input_fmt = NULL;
    uint8_t *avio_ctx_buffer = NULL;
    size_t avio_ctx_buffer_size = 2000000;
    char *input_filename = NULL;
    int ret = 0;
    struct buffer_data bd = { 0 };


    /* fill opaque structure used by the AVIOContext read callback */
    bd.ptr  = data;
    bd.size = size;

    if (!(fmt_ctx = avformat_alloc_context())) {
        Print2File("if (!(fmt_ctx = avformat_alloc_context()))");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
    if (!avio_ctx_buffer) {
        Print2File("if (!avio_ctx_buffer");
        ret = AVERROR(ENOMEM);
        goto end;
    }
    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
                                  0, &bd, &read_buffer, NULL, NULL);
    if (!avio_ctx) {
        Print2File("if (!avio_ctx) ");
        ret = AVERROR(ENOMEM);
        goto end;
    }
    // fmt_ctx->probesize = 200* 8;//修改探针大小
    fmt_ctx->pb = avio_ctx;
    // input_fmt = av_find_input_format("mpegts");//修改的加速不用判断格式(Debug)
    //  出问题的函数在这里！！！！！！！！！！！！！！！！！
    ret = avformat_open_input(&fmt_ctx, NULL, input_fmt, NULL);
    Print2File("avformat_open_input ret : "+std::to_string(ret));
    // ret = lhs_avformat_open_input(&fmt_ctx, NULL, NULL, NULL);
    if (ret < 0) {
        Print2File("Could not open input");
        fprintf(stderr, "Could not open input\n");
        goto end;
    }

    ret = avformat_find_stream_info(fmt_ctx, NULL);//End of file 错误
    if (ret < 0) {
        Print2File("Could not find stream information======= : "+std::to_string(ret));
        Print2File(av_err2str(ret));
        Print2File("Could not find stream information============");
        fprintf(stderr, "Could not find stream information\n");
        goto end;
    }

    av_dump_format(fmt_ctx, 0, input_filename, 0);

end:
    // avformat_close_input(&fmt_ctx);

    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
    // if (avio_ctx)
    //     av_freep(&avio_ctx->buffer);
    // avio_context_free(&avio_ctx);

    if (ret < 0) {
        Print2File("Error occurred:");
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return NULL;
    }

    return fmt_ctx;
}

void worker_cb(EV_P_ ev_timer *w, int revents) {
    // AVPacket packet;
    Decoder *decoder = (Decoder *)w->data;

    int ret = SodtpReadPacket(decoder->pJitter, decoder->pPacket, decoder->pBlock);


    if (decoder->pJitter->state == SodtpJitter::STATE_CLOSE) {
        // Stream is closed.
        // Thread will be closed by breaking event loop.
        ev_timer_stop(loop, w);
        fprintf(stderr, "Stream %d is closed!\n", decoder->iStream);
        return;
    }

    if (ret == SodtpJitter::STATE_NORMAL) {
        // Receive one more block.
        decoder->iBlock++;
        // printf("decoding: stream %d,\t block %d,\t size %d,\t received block count %d\n",
        //     decoder->pBlock->stream_id, decoder->pBlock->block_id,
        //     decoder->pPacket->size, decoder->iBlock);
        printf("decoding: stream %d,\t block %d,\t size %d,\t delay %d\n",
            decoder->pBlock->stream_id, decoder->pBlock->block_id,
            decoder->pPacket->size, (int)(current_mtime() - decoder->pBlock->block_ts));

        decoder->iFrame = decoder->pBlock->block_id + 1;
        // DecodePacket(
        //             decoder->pVCodecCtx,
        //             decoder->pSwsCtx,
        //             decoder->pFrame,
        //             decoder->pFrameRGB,
        //             decoder->pPacket,
        //             decoder->pJitter->stream_id,
        //             decoder->pBlock->block_id + 1);
        DecodePacket(decoder);
    }
    else if (ret == SodtpJitter::STATE_BUFFERING) {
        printf("decoding: buffering stream %d\n", decoder->iStream);
    }
    else if (ret == SodtpJitter::STATE_SKIP) {
        //计算卡顿的地方？
        printf("decoding: skip one block of stream %d\n", decoder->iStream);
    }
    else {
        printf("decoding: warning! unknown state of stream %d!\n", decoder->iStream);
    }
    // Free the packet that was allocated by av_read_frame
    // av_free_packet(&packet);


    // normal or skip : 40ms
    // buffering: 200ms;
    // printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
}

void set_codec_context(AVCodecContext *ctx, int width, int height) {
    ctx->codec_id   = AV_CODEC_ID_H265;
    ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    ctx->width      = width;
    ctx->height     = height;
    ctx->pix_fmt    = AV_PIX_FMT_YUV420P;

    // ctx->time_base.num = 1;
    // ctx->time_base.den = 1200000;
    // ctx->gop_size   = 25;
}

int video_viewer3(SodtpJitterPtr pJitter, const char *path) {

    printf("viewer: viewing stream %u!\n", pJitter->stream_id);

    // Initalizing these to NULL prevents segfaults!
    AVFormatContext         *pVFormatCtx = NULL;
    AVCodecContext          *pVCodecCtx = NULL;
    AVCodec                 *pVCodec = NULL;
    AVFrame                 *pFrame = NULL;
    AVFrame                 *pFrameRGB = NULL;
    AVPacket                packet;
    int                     iFrame;
    int                     ret;
    int                     numBytes;
    uint8_t                 *buffer = NULL;
    struct SwsContext       *pSwsCtx = NULL;

    SodtpBlockPtr           pBlock = NULL;

    // Register all formats and codecs
    // av_register_all();

    av_init_packet(&packet);

    pVCodecCtx = avcodec_alloc_context3(NULL);
    if (!pVCodecCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return -1;
    }
    pVCodecCtx->thread_count = 1;

    int i = 0;
    int WAITING_UTIME = 20000;
    int WAITING_ROUND = 500;
    int SKIPPING_ROUND = 70;
    // while (true) {
    //     ret = pJitter->front(pBlock);

    //     if (ret == SodtpJitter::STATE_NORMAL && pBlock->block_id == 0) {
    //         printf("sniffing: stream %d,\t block %d,\t size %d\n",
    //                 pBlock->stream_id, pBlock->block_id, pBlock->size);
    //         pVFormatCtx = sniff_format2(pBlock->data, pBlock->size);
    //         break;
    //     }
    //     printf("viewer: waiting for stream %u block 0! sleep round %d!\n", pJitter->stream_id, i);

    //     if (++i > WAITING_ROUND) {
    //         printf("viewer: fail to read the first block of stream %u.\n", pJitter->stream_id);
    //         break;
    //     }
    //     usleep(WAITING_UTIME);
    // }

    // // succeed to sniff the format.
    // if (pVFormatCtx) {
    //     avcodec_parameters_to_context(pVCodecCtx, pVFormatCtx->streams[0]->codecpar);
    // }
    // // fail to sniff the format, then try to use the meta data.
    // else {
    //     printf("viewer: check the context meta data for stream %u.\n", pJitter->stream_id);
    //     SodtpMetaData meta;
    //     pJitter->get_meta_data(&meta);
    //     if (meta.width && meta.height) {
    //         set_codec_context(pVCodecCtx, meta.width, meta.height);
    //     }
    //     else {
    //         fprintf(stderr, "fail to parse the codec context, exit stream %u.\n", pJitter->stream_id);
    //         return -1;
    //     }
    // }

    while (true) {
        ret = pJitter->front(pBlock);

        if (ret == SodtpJitter::STATE_NORMAL && pBlock->key_block) {
            Print2File("video_viewer3 sniffing: stream ");
            fprintf(stdout, "sniffing: stream %d,\t block %d,\t size %d\n",
                    pBlock->stream_id, pBlock->block_id, pBlock->size);
            pVFormatCtx = sniff_format2(pBlock->data, pBlock->size);
            break;
        }
        fprintf(stderr, "decoding: waiting for the key block of stream %u. sleep round %d!\n", pJitter->stream_id, i);

        if (++i > WAITING_ROUND) {
            fprintf(stderr, "decoding: fail to read the key block of stream %u.\n", pJitter->stream_id);
            break;
        }
        usleep(WAITING_UTIME);
    }

    // // skip some stale blocks to find a key frame.
    // if (!pVFormatCtx) {
    //     i = 0;
    //     while (true) {
    //         ret = pJitter->front(pBlock);
    //         if (ret == SodtpJitter::STATE_NORMAL && pBlock->key_block) {
    //             fprintf(stdout, "sniffing: stream %d,\t block %d,\t size %d\n",
    //                     pBlock->stream_id, pBlock->block_id, pBlock->size);
    //             pVFormatCtx = sniff_format2(pBlock->data, pBlock->size);
    //             break;
    //         }
    //         pJitter->pop(pBlock);

    //         if (++i > SKIPPING_ROUND) {
    //             fprintf(stderr, "viewer: quit stream %u.\n", pJitter->stream_id);
    //             return -1;
    //         }
    //         usleep(WAITING_UTIME);
    //     }
    // }

    if (!pVFormatCtx) {
        fprintf(stderr, "viewer: quit stream %u.\n", pJitter->stream_id);
        return -1;
    }
    avcodec_parameters_to_context(pVCodecCtx, pVFormatCtx->streams[0]->codecpar);

    pVCodec = avcodec_find_decoder(pVCodecCtx->codec_id);
    if (!pVCodec) {
        fprintf(stderr, "Codec not found\n");
        return -1;
    }

    // Open Codec
    if (avcodec_open2(pVCodecCtx, pVCodec, NULL) < 0) {
        fprintf(stderr, "Fail to open codec!\n");
        return -1;
    }

    // Allocate video frame
    pFrame = av_frame_alloc();
    // Allocate an AVFrame structure
    pFrameRGB = av_frame_alloc();
    if (pFrame == NULL || pFrameRGB == NULL)
        return -1;

    // Determine required buffer size and allocate buffer
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pVCodecCtx->width, pVCodecCtx->height, 1);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer,
        AV_PIX_FMT_RGB24, pVCodecCtx->width, pVCodecCtx->height, 1);

    // initialize SWS context for software scaling
    pSwsCtx = sws_getContext(pVCodecCtx->width,
                            pVCodecCtx->height,
                            pVCodecCtx->pix_fmt,
                            pVCodecCtx->width,
                            pVCodecCtx->height,
                            AV_PIX_FMT_RGB24,
                            SWS_BILINEAR,
                            NULL,
                            NULL,
                            NULL
                            );

    // SaveConfig scon;
    // if (!path) {
    //     scon.parse("./config/save.conf");
    //     path = scon.path.c_str();
    // }

    Decoder decoder;
    decoder.pVCodecCtx  = pVCodecCtx;
    decoder.pSwsCtx     = pSwsCtx;
    decoder.pFrame      = pFrame;
    decoder.pFrameRGB   = pFrameRGB;
    decoder.pPacket     = &packet;
    decoder.pJitter     = pJitter.get();
    decoder.pBlock      = NULL;
    decoder.iStream     = pJitter->stream_id;
    decoder.iFrame      = 0;
    decoder.iBlock      = 0;
    decoder.path        = path;

    ev_timer worker;
    struct ev_loop *loop = ev_loop_new(EVFLAG_AUTO);

    double nominal = (double)pJitter->get_nominal_depth() / 1000.0;
    double interval = 0.040;    // 40ms, i.e. 25fps

    ev_timer_init(&worker, worker_cb, nominal, interval);
    ev_timer_start(loop, &worker);
    worker.data = &decoder;

    ev_loop(loop, 0);


    // Free the RGB image
    av_free(buffer);
    av_frame_free(&pFrameRGB);

    // Free the YUV frame
    av_frame_free(&pFrame);

    // Close the codecs
    avcodec_close(pVCodecCtx);

    // Close the video file
    avformat_close_input(&pVFormatCtx);

    // Notification for clearing the jitter.
    sem_post(pJitter->_sem);
    ev_feed_signal(SIGUSR1);

    return 0;
}


void worker_cb4(EV_P_ ev_timer *w, int revents) {
    // Print2File("========worker_cb4===============");//这里跑的
    // AVPacket packet;
    Decoder *decoder = (Decoder *)w->data;
    int ret = SodtpReadPacket(decoder->pJitter, decoder->pPacket, decoder->pBlock);
    if (decoder->pJitter->state == SodtpJitter::STATE_CLOSE) {
        // Stream is closed.
        // Thread will be closed by breaking event loop.
        ev_timer_stop(loop, w);
        fprintf(stderr, "Stream %d is closed!\n", decoder->iStream);
        return;
    }
    if (ret == SodtpJitter::STATE_NORMAL) {
        // Receive one more block.
        decoder->iBlock++;
        // Print2File("ret == SodtpJitter::STATE_NORMAL");//这里跑的
        // printf("decoding: stream %d,\t block %d,\t size %d,\t received block count %d\n",
        //     decoder->pBlock->stream_id, decoder->pBlock->block_id,
        //     decoder->pPacket->size, decoder->iBlock);
        printf("decoding: stream %d,\t block %d,\t size %d,\t delay %d\n",
            decoder->pBlock->stream_id, decoder->pBlock->block_id,
            decoder->pPacket->size, (int)(current_mtime() - decoder->pBlock->block_ts));

        decoder->iFrame = decoder->pBlock->block_id + 1;
        if(decoder->pPacket->flags & AV_PKT_FLAG_KEY){
            // Print2File("iskey frame===========");
        }else{
            // Print2File("Not key frame============");//跑这里
        }
        DecodePacketPlay(decoder);
        // Print2File("DecodePacketPlay(decoder);");//这里断了！！！！！！！！！！！！！
    }
    else if (ret == SodtpJitter::STATE_BUFFERING) {
        printf("decoding: buffering stream %d\n", decoder->iStream);
    }
    else if (ret == SodtpJitter::STATE_SKIP) {
        printf("decoding: skip one block of stream %d\n", decoder->iStream);
    }
    else {
        printf("decoding: warning! unknown state of stream %d!\n", decoder->iStream);
    }
    // Free the packet that was allocated by av_read_frame
    // av_free_packet(&packet);


    // normal or skip : 40ms
    // buffering: 200ms;
    // printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
}
// decode and display the pictures.
int video_viewer4(SodtpJitterPtr pJitter, SDLPlay *splay, const char *path) {
    // printf("viewer: viewing stream %u!\n", pJitter->stream_id);
    // Initalizing these to NULL prevents segfaults!
    AVFormatContext         *pVFormatCtx = NULL;
    AVCodecContext          *pVCodecCtx = NULL;
    AVCodec                 *pVCodec = NULL;
    AVFrame                 *pFrame = NULL;
    AVFrame                 *pFrameYUV = NULL;
    AVFrame                 *pFrameShow = NULL;
    AVPacket                packet;
    int                     iFrame;
    int                     ret;
    int                     numBytes;
    uint8_t                 *buffer = NULL;
    struct SwsContext       *pSwsCtx = NULL;

    SodtpBlockPtr           pBlock = NULL;
    SDL_Texture             *pTexture = NULL;

    CodecParWithoutExtraData* myCodecPar = NULL;
    // Register all formats and codecs
    // av_register_all(); // 原本就是注释掉的
	//初始化网络库 （可以打开rtsp rtmp http 协议的流媒体视频）
	// avformat_network_init();// 原本就是注释掉的

    av_init_packet(&packet);

    pVCodecCtx = avcodec_alloc_context3(NULL);
    if (!pVCodecCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return -1;
    }
    pVCodecCtx->thread_count = 1;

    int i = 0;
    int WAITING_UTIME = 20000;
    int WAITING_ROUND = 500;
    int SKIPPING_ROUND = 70;

    while (true) {
        ret = pJitter->front(pBlock);

        if ((ret == SodtpJitter::STATE_NORMAL) && pBlock->key_block) {
            fprintf(stdout, "sniffing: stream %d,\t block %d,\t size %d\n",
                    pBlock->stream_id, pBlock->block_id, pBlock->size);

            // Print2File("==========================改的接口==========================");
            // pVFormatCtx = sniff_format2(pBlock->data, pBlock->size); //原本！！！！
            if(pBlock->haveFormatContext){
                // Print2File("myCodecPar = pBlock->codecParPtr;");
                // myCodecPar 除了extraData的数据
                myCodecPar = pBlock->codecParPtr;
                // Print2File("*myCodecPar->extradata = *pBlock->codecParExtradata");
                myCodecPar->extradata = pBlock->codecParExtradata;
                // Print2File("*myCodecPar->extradata = *pBlock->codecParExtradata break");
                break;
            }else{
                Print2File("pBlock->haveFormatContext false : continue");
                continue;
            }
            break;
        }
        // 弱网发生
        // Print2File("decoding: waiting for the key block of stream %u. sleep round");
        fprintf(stderr, "decoding: waiting for the key block of stream %u. sleep round %d!\n", pJitter->stream_id, i);

        if (++i > WAITING_ROUND) {
            // 弱网发生
            // Print2File("decoding: fail to read the key block of stream");
            fprintf(stderr, "decoding: fail to read the key block of stream %u.\n", pJitter->stream_id);
            break;
        }
        usleep(WAITING_UTIME);
    }
    timeMainPlayer.evalTime("p","video_viewer4_while_pass");
    
    // Print2File("if (!pVFormatCtx)");
    // if (!pVFormatCtx) {
    //     Print2File("viewer: quit stream");
    //     fprintf(stderr, "viewer: quit stream %u.\n", pJitter->stream_id);
    //     return -1;
    // }
    // Print2File("beafore if (!myCodecPar)");
    if (!myCodecPar) {
        //弱网发生
        // Print2File("viewer: quit stream");
        fprintf(stderr, "viewer: quit stream %u.\n", pJitter->stream_id);
        return -1;
    }
    // Print2File("after if (!myCodecPar)");
    // 源码修改成功，待改第二个变量
    // CodecParWithoutExtraData codecpar;
    // myCodecPar = &codecpar;
    // int ret3 = lhs_copy_parameters_to_myParameters(myCodecPar, pVFormatCtx->streams[0]->codecpar);
    // Print2File("int ret2 = lhs_copy_parameters_to_context ==========================改的接口==========================");
    int ret2 = lhs_copy_parameters_to_context(pVCodecCtx, myCodecPar);
    // int ret2 = avcodec_parameters_to_context(pVCodecCtx, pVFormatCtx->streams[0]->codecpar);
    if(ret2<0){
        Print2File("avcodec_parameters_to_context():"+std::to_string(ret2));
    }
    
    // Print2File("pVCodec = avcodec_find_decoder(pVCodecCtx->codec_id);");
    pVCodec = avcodec_find_decoder(pVCodecCtx->codec_id);
    if (!pVCodec) {
        Print2File("Codec not found");
        fprintf(stderr, "Codec not found\n");
        return -1;
    }
    // Print2File("if (avcodec_open2(pVCodecCtx, pVCodec, NULL) < 0) {");
    // Open Codec
    if (avcodec_open2(pVCodecCtx, pVCodec, NULL) < 0) {
        Print2File("Fail to open codec!");
        fprintf(stderr, "Fail to open codec!\n");
        return -1;
    }
    // Print2File("pFrame = av_frame_alloc();");
    // Allocate video frame
    pFrame = av_frame_alloc();
    // Allocate an AVFrame structure
    // Print2File("pFrameYUV = av_frame_alloc();");
    pFrameYUV = av_frame_alloc();
    // Print2File("pFrameShow = av_frame_alloc();");
    pFrameShow = av_frame_alloc();
    if (pFrame == NULL || pFrameYUV == NULL || pFrameShow == NULL) {
        Print2File("fail to allocate frame!");
        printf("fail to allocate frame!\n");
        return -1;
    }
    // Determine required buffer size and allocate buffer
    timeMainPlayer.evalTime("p","av_image_get_buffer_size");
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pVCodecCtx->width, pVCodecCtx->height, 1);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    // Print2File("av_image_fill_arrays");
    // Assign appropriate parts of buffer to image planes in pFrameYUV
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, buffer,
        AV_PIX_FMT_YUV420P, pVCodecCtx->width, pVCodecCtx->height, 1);
    av_image_fill_arrays(pFrameShow->data, pFrameShow->linesize, buffer,
        AV_PIX_FMT_YUV420P, pVCodecCtx->width, pVCodecCtx->height, 1);
    // Print2File("sws_getContext");
    // initialize SWS context for software scaling
    pSwsCtx = sws_getContext(pVCodecCtx->width,
                            pVCodecCtx->height,
                            pVCodecCtx->pix_fmt,
                            pVCodecCtx->width,
                            pVCodecCtx->height,
                            AV_PIX_FMT_YUV420P,
                            SWS_BILINEAR,
                            NULL,
                            NULL,
                            NULL
                            );
    // Print2File("SDL_CreateTexture");
    pTexture = SDL_CreateTexture(splay->renderer,
                            SDL_PIXELFORMAT_IYUV,
                            SDL_TEXTUREACCESS_STREAMING,
                            pVCodecCtx->width,
                            pVCodecCtx->height
                            );
    // SaveConfig scon;
    // if (!path) {
    //     scon.parse("./config/save.conf");
    //     path = scon.path.c_str();
    // }

    Decoder &decoder    = pJitter->decoder;
    decoder.pVCodecCtx  = pVCodecCtx;
    decoder.pSwsCtx     = pSwsCtx;
    decoder.pFrame      = pFrame;
    decoder.pFrameYUV   = pFrameYUV;
    decoder.pFrameRGB   = NULL;
    decoder.pFrameShow  = pFrameShow;
    decoder.pPacket     = &packet;
    decoder.pJitter     = pJitter.get();
    decoder.pBlock      = NULL;
    decoder.iStream     = pJitter->stream_id;
    decoder.iFrame      = 0;
    decoder.iBlock      = 0;
    decoder.iStart      = 0;
    // Set the path to be NULL, which means we do not
    // save picture in SDL display mode.
    decoder.path        = NULL;
    decoder.pTexture    = pTexture;


    ev_timer worker;
    struct ev_loop *loop = ev_loop_new(EVFLAG_AUTO);

    double nominal = (double)pJitter->get_nominal_depth() / 1000.0;
    double interval = 0.040;    // 40ms, i.e. 25fps

    ev_timer_init(&worker, worker_cb4, nominal, interval);
    ev_timer_start(loop, &worker);
    worker.data = &decoder;

    ev_loop(loop, 0);


    // Free the YUV image
    av_free(buffer);
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrameShow);

    // Free the original frame
    av_frame_free(&pFrame);

    // Close the codecs
    avcodec_close(pVCodecCtx);

    // Close the video file
    avformat_close_input(&pVFormatCtx);

    // Notification for clearing the jitter.
    sem_post(pJitter->_sem);
    timeMainPlayer.evalTime("p","ev_feed_signal(SIGUSR1);");
    ev_feed_signal(SIGUSR1);
    
    return 0;
}


// =======================lhs改的=======================
// int lhs_avformat_open_input(AVFormatContext **ps, const char *filename,
//                         AVInputFormat *fmt, AVDictionary **options)
// {
//     AVFormatContext *s = *ps;
//     int ret = 0;
//     AVDictionary *tmp = NULL;
//     ID3v2ExtraMeta *id3v2_extra_meta = NULL;
 
//     if (!s && !(s = avformat_alloc_context()))
//         return AVERROR(ENOMEM);
//     if (!s->av_class) {
//         av_log(NULL, AV_LOG_ERROR, "Input context has not been properly allocated by avformat_alloc_context() and is not NULL either\n");
//         return AVERROR(EINVAL);
//     }
//     if (fmt)
//         s->iformat = fmt;
 
//     if (options)
//         av_dict_copy(&tmp, *options, 0);
 
//     if ((ret = av_opt_set_dict(s, &tmp)) < 0)
//         goto fail;
 
//     if ((ret = init_input(s, filename, &tmp)) < 0)
//         goto fail;
//     s->probe_score = ret;
 
//     if (s->format_whitelist && av_match_list(s->iformat->name, s->format_whitelist, ',') <= 0) {
//         av_log(s, AV_LOG_ERROR, "Format not on whitelist\n");
//         ret = AVERROR(EINVAL);
//         goto fail;
//     }
 
//     avio_skip(s->pb, s->skip_initial_bytes);
 
//     /* Check filename in case an image number is expected. */
//     if (s->iformat->flags & AVFMT_NEEDNUMBER) {
//         if (!av_filename_number_test(filename)) {
//             ret = AVERROR(EINVAL);
//             goto fail;
//         }
//     }
 
//     s->duration = s->start_time = AV_NOPTS_VALUE;
//     av_strlcpy(s->filename, filename ? filename : "", sizeof(s->filename));
 
//     /* Allocate private data. */
//     if (s->iformat->priv_data_size > 0) {
//         if (!(s->priv_data = av_mallocz(s->iformat->priv_data_size))) {
//             ret = AVERROR(ENOMEM);
//             goto fail;
//         }
//         if (s->iformat->priv_class) {
//             *(const AVClass **) s->priv_data = s->iformat->priv_class;
//             av_opt_set_defaults(s->priv_data);
//             if ((ret = av_opt_set_dict(s->priv_data, &tmp)) < 0)
//                 goto fail;
//         }
//     }
 
//     /* e.g. AVFMT_NOFILE formats will not have a AVIOContext */
//     if (s->pb)
//         ff_id3v2_read(s, ID3v2_DEFAULT_MAGIC, &id3v2_extra_meta, 0);
 
//     if (!(s->flags&AVFMT_FLAG_PRIV_OPT) && s->iformat->read_header)
//         if ((ret = s->iformat->read_header(s)) < 0)
//             goto fail;
 
//     if (id3v2_extra_meta) {
//         if (!strcmp(s->iformat->name, "mp3") || !strcmp(s->iformat->name, "aac") ||
//             !strcmp(s->iformat->name, "tta")) {
//             if ((ret = ff_id3v2_parse_apic(s, &id3v2_extra_meta)) < 0)
//                 goto fail;
//         } else
//             av_log(s, AV_LOG_DEBUG, "demuxer does not support additional id3 data, skipping\n");
//     }
//     ff_id3v2_free_extra_meta(&id3v2_extra_meta);
 
//     if ((ret = avformat_queue_attached_pictures(s)) < 0)
//         goto fail;
 
//     if (!(s->flags&AVFMT_FLAG_PRIV_OPT) && s->pb && !s->data_offset)
//         s->data_offset = avio_tell(s->pb);
 
//     s->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;
 
//     if (options) {
//         av_dict_free(options);
//         *options = tmp;
//     }
//     *ps = s;
//     return 0;
 
// fail:
//     ff_id3v2_free_extra_meta(&id3v2_extra_meta);
//     av_dict_free(&tmp);
//     if (s->pb && !(s->flags & AVFMT_FLAG_CUSTOM_IO))
//         avio_close(s->pb);
//     avformat_free_context(s);
//     *ps = NULL;
//     return ret;
// }

// int lhs_avformat_open_input(AVFormatContext **ps, const char *filename,
//                         AVInputFormat *fmt, AVDictionary **options)
// {
//     AVFormatContext *s = *ps;
//     int i, ret = 0;
//     AVDictionary *tmp = NULL;
//     ID3v2ExtraMeta *id3v2_extra_meta = NULL;

//     if (!s && !(s = avformat_alloc_context())) //如果s没有初始化，先初始化s
//         return AVERROR(ENOMEM);
//     if (!s->av_class) {
//         av_log(NULL, AV_LOG_ERROR, "Input context has not been properly allocated by avformat_alloc_context() and is not NULL either\n");
//         return AVERROR(EINVAL);
//     }
//     if (fmt)
//         s->iformat = fmt;

//     if (options)
//         av_dict_copy(&tmp, *options, 0);

//     if (s->pb) // must be before any goto fail
//         s->flags |= AVFMT_FLAG_CUSTOM_IO;

//     if ((ret = av_opt_set_dict(s, &tmp)) < 0)
//         goto fail;

//     if (!(s->url = av_strdup(filename ? filename : ""))) {
//         ret = AVERROR(ENOMEM);
//         goto fail;
//     }

// #if FF_API_FORMAT_FILENAME
// FF_DISABLE_DEPRECATION_WARNINGS
//     av_strlcpy(s->filename, filename ? filename : "", sizeof(s->filename));
// FF_ENABLE_DEPRECATION_WARNINGS
// #endif
// 	//	1. 重点在看这里：
//     if ((ret = init_input(s, filename, &tmp)) < 0)
//         goto fail;
//     s->probe_score = ret;

//     if (!s->protocol_whitelist && s->pb && s->pb->protocol_whitelist) {
//         s->protocol_whitelist = av_strdup(s->pb->protocol_whitelist);
//         if (!s->protocol_whitelist) {
//             ret = AVERROR(ENOMEM);
//             goto fail;
//         }
//     }

//     if (!s->protocol_blacklist && s->pb && s->pb->protocol_blacklist) {
//         s->protocol_blacklist = av_strdup(s->pb->protocol_blacklist);
//         if (!s->protocol_blacklist) {
//             ret = AVERROR(ENOMEM);
//             goto fail;
//         }
//     }

//     if (s->format_whitelist && av_match_list(s->iformat->name, s->format_whitelist, ',') <= 0) {
//         av_log(s, AV_LOG_ERROR, "Format not on whitelist \'%s\'\n", s->format_whitelist);
//         ret = AVERROR(EINVAL);
//         goto fail;
//     }

//     avio_skip(s->pb, s->skip_initial_bytes);

//     /* Check filename in case an image number is expected. */
//     if (s->iformat->flags & AVFMT_NEEDNUMBER) {
//         if (!av_filename_number_test(filename)) {
//             ret = AVERROR(EINVAL);
//             goto fail;
//         }
//     }

//     s->duration = s->start_time = AV_NOPTS_VALUE;

//     /* Allocate private data. */
//     if (s->iformat->priv_data_size > 0) {
//         if (!(s->priv_data = av_mallocz(s->iformat->priv_data_size))) {
//             ret = AVERROR(ENOMEM);
//             goto fail;
//         }
//         if (s->iformat->priv_class) {
//             *(const AVClass **) s->priv_data = s->iformat->priv_class;
//             av_opt_set_defaults(s->priv_data);
//             if ((ret = av_opt_set_dict(s->priv_data, &tmp)) < 0)
//                 goto fail;
//         }
//     }

//     /* e.g. AVFMT_NOFILE formats will not have a AVIOContext */
//     if (s->pb)
//         ff_id3v2_read_dict(s->pb, &s->internal->id3v2_meta, ID3v2_DEFAULT_MAGIC, &id3v2_extra_meta);


//     if (!(s->flags&AVFMT_FLAG_PRIV_OPT) && s->iformat->read_header)
//     //重点2
//         if ((ret = s->iformat->read_header(s)) < 0)
//             goto fail;

//     if (!s->metadata) {
//         s->metadata = s->internal->id3v2_meta;
//         s->internal->id3v2_meta = NULL;
//     } else if (s->internal->id3v2_meta) {
//         int level = AV_LOG_WARNING;
//         if (s->error_recognition & AV_EF_COMPLIANT)
//             level = AV_LOG_ERROR;
//         av_log(s, level, "Discarding ID3 tags because more suitable tags were found.\n");
//         av_dict_free(&s->internal->id3v2_meta);
//         if (s->error_recognition & AV_EF_EXPLODE)
//             return AVERROR_INVALIDDATA;
//     }

//     if (id3v2_extra_meta) {
//         if (!strcmp(s->iformat->name, "mp3") || !strcmp(s->iformat->name, "aac") ||
//             !strcmp(s->iformat->name, "tta")) {
//             if ((ret = ff_id3v2_parse_apic(s, &id3v2_extra_meta)) < 0)
//                 goto fail;
//             if ((ret = ff_id3v2_parse_chapters(s, &id3v2_extra_meta)) < 0)
//                 goto fail;
//             if ((ret = ff_id3v2_parse_priv(s, &id3v2_extra_meta)) < 0)
//                 goto fail;
//         } else
//             av_log(s, AV_LOG_DEBUG, "demuxer does not support additional id3 data, skipping\n");
//     }
//     ff_id3v2_free_extra_meta(&id3v2_extra_meta);

//     if ((ret = avformat_queue_attached_pictures(s)) < 0)
//         goto fail;

//     if (!(s->flags&AVFMT_FLAG_PRIV_OPT) && s->pb && !s->internal->data_offset)
//         s->internal->data_offset = avio_tell(s->pb);

//     s->internal->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;

//     update_stream_avctx(s);

//     for (i = 0; i < s->nb_streams; i++)
//         s->streams[i]->internal->orig_codec_id = s->streams[i]->codecpar->codec_id;

//     if (options) {
//         av_dict_free(options);
//         *options = tmp;
//     }
//     *ps = s;
//     return 0;

// fail:
//     ff_id3v2_free_extra_meta(&id3v2_extra_meta);
//     av_dict_free(&tmp);
//     if (s->pb && !(s->flags & AVFMT_FLAG_CUSTOM_IO))
//         avio_closep(&s->pb);
//     avformat_free_context(s);
//     *ps = NULL;
//     return ret;
// }

#endif // DECODE_VIDEO_H