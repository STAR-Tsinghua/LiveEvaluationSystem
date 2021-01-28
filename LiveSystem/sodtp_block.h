#ifndef SODTP_BLOCK_H
#define SODTP_BLOCK_H

#include <list>
#include <memory>

#include <sodtp_util.h>
#include <util_log.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

// Max block data size is 2M bytes
#define MAX_BLOCK_DATA_SIZE 2000000

#define HEADER_FLAG_NULL    0x0000
#define HEADER_FLAG_META    0x0001
#define HEADER_FLAG_KEY     0x0002
#define HEADER_FLAG_FIN     0x0010

typedef struct CodecParWithoutExtraData {
    enum AVMediaType codec_type;
    enum AVCodecID   codec_id;
    uint32_t         codec_tag;
    uint8_t *extradata;
    int      extradata_size;
    int format;
    int64_t bit_rate;
    int bits_per_coded_sample;
    int bits_per_raw_sample;
    int profile;
    int level;
    int width;
    int height;
    AVRational sample_aspect_ratio;
    enum AVFieldOrder                  field_order;
    enum AVColorRange                  color_range;
    enum AVColorPrimaries              color_primaries;
    enum AVColorTransferCharacteristic color_trc;
    enum AVColorSpace                  color_space;
    enum AVChromaLocation              chroma_location;
    int video_delay;
    uint64_t channel_layout;
    int      channels;
    int      sample_rate;
    int      block_align;
    int      frame_size;
    int initial_padding;
    int trailing_padding;
    int seek_preroll;
} CodecParWithoutExtraData;

typedef struct SodtpStreamHeader {
    uint32_t    flag;
    uint32_t    stream_id;
    int64_t     block_ts;
    //
    uint32_t    block_id;
    //
    int32_t     duration;       // frame duration (ms)

    bool haveFormatContext = false;
    CodecParWithoutExtraData codecPar;
    
    // TODO 改写拷贝构造
    // SodtpStreamHeader operator=(SodtpStreamHeader& SshTmp)// 深拷贝函数


    // int32_t     group_id;       // ID of the streaming group.
    // int32_t     deadline;       // deadline of this block
} SodtpStreamHeader;

typedef struct SodtpMetaData
{
    int32_t     width;
    int32_t     height;
} SodtpMetaData;

class SodtpBlock {
public:
    bool        last_one;       // whether the last block in its stream.
    bool        key_block;      // whether the key block in its stream.

    uint32_t    block_id;
    uint32_t    stream_id;

    uint8_t     *data;
    int32_t     size;

    bool haveFormatContext = false;
    uint8_t     *codecParExtradata;
    CodecParWithoutExtraData *codecParPtr;
    // Timestamp of the block.
    // Its value should be orignally set by data producer (e.g. sender).
    // block_ts will be used in checking whether a block should be dropped.
    // Warning: this needs the time synchronization.
    int64_t     block_ts;       // block timestamp at the sender side.
    int32_t     duration;       // frame duration (ms)

    ~SodtpBlock() {
        if (data) {
            delete[] data;
            delete[] codecParExtradata;
        }
    }
};

typedef std::shared_ptr<SodtpBlock>  SodtpBlockPtr;

class BlockData
{
public:
    uint8_t     *offset;
    uint8_t     data[MAX_BLOCK_DATA_SIZE];

    uint32_t    id;             // a temp id for collecting complete data.
    uint64_t    expire_ts;

    BlockData(uint32_t id) {
        // expire_ts = current_time() + 200; // deadline 
        offset = data;
        this->id = id;
    }

    int write(uint8_t *src, int size) {
        int ret = size;

        if ((offset - data) + size > MAX_BLOCK_DATA_SIZE) {
            ret = 0;
            fprintf(stderr, "Block data overflows!\n");
        }
        else {
            memcpy(offset, src, size);
            offset += size;
        }

        return ret;
    }
};
typedef std::shared_ptr<BlockData> BlockDataPtr;


class BlockDataBuffer {
public:
    static const int32_t MAX_BLOCK_NUM = 40;

    std::list<BlockDataPtr> buffer;


    // Curently, we use MAX_BLOCK_NUM to pop stale data of buffer.
    // But we should better use expiration time-out as the signal.
    int write(uint32_t id, uint8_t *src, int size);


    // Read after the block is completed.
    SodtpBlockPtr read(uint32_t id, SodtpStreamHeader *head);
};

// AVCodecParameters 要改成 CodecParWithoutExtraData
// 不能复制extradata
static int lhs_copy_parameters_to_context(AVCodecContext *codec,
                                  const CodecParWithoutExtraData *par)
{
    // Print2File("lhs_copy_parameters_to_context in ~~~~~~~~~~~~");
    // 编码器类型-视频？音频？字幕？
    codec->codec_type = par->codec_type;
    // 编码器id，可以找到唯一的编码器
    codec->codec_id   = par->codec_id;
    // 编码器tag，编码器的另外一种表述
    codec->codec_tag  = par->codec_tag;
 
    // 平均码率
    codec->bit_rate              = par->bit_rate;
    // 采样点编码后所占位数
    codec->bits_per_coded_sample = par->bits_per_coded_sample;
    // 原始采样点所占位数
    codec->bits_per_raw_sample   = par->bits_per_raw_sample;
    // 编码profile
    codec->profile               = par->profile;
    // 编码level
    codec->level                 = par->level;
 
    // 针对不同的编码器类型赋值
    switch (par->codec_type) {
    // 视频
    case AVMEDIA_TYPE_VIDEO:
        // 像素格式
        codec->pix_fmt                = (AVPixelFormat)par->format;
        // 视频宽度
        codec->width                  = par->width;
        // 视频高度
        codec->height                 = par->height;
        // 场帧顺序-逐行扫描？顶场先编码先展示，顶场先编码后展示，底场先编码先展示，底场先编码后展示
        codec->field_order            = par->field_order;
        // 颜色相关的几个参数，不太懂
        codec->color_range            = par->color_range;
        codec->color_primaries        = par->color_primaries;
        codec->color_trc              = par->color_trc;
        codec->colorspace             = par->color_space;
        codec->chroma_sample_location = par->chroma_location;
        // SAR，样本的宽高比
        codec->sample_aspect_ratio    = par->sample_aspect_ratio;
        // 是否有b帧，对应par中是否有视频延迟
        codec->has_b_frames           = par->video_delay;
        break;
 
    // 音频
    case AVMEDIA_TYPE_AUDIO:
        // 采样格式
        codec->sample_fmt       = (AVSampleFormat)par->format;
        // 通道布局
        codec->channel_layout   = par->channel_layout;
        // 通道数
        codec->channels         = par->channels;
        // 采样率
        codec->sample_rate      = par->sample_rate;
        // 音频包中的对齐块大小
        codec->block_align      = par->block_align;
        // 一帧音频中单个通道采样个数(Number of samples per channel in an audio fram)
        codec->frame_size       = par->frame_size;
        // 编码延迟，为送入若干采样点之后编码器才能产生合法的输出，此时的采样点个数
        // codec->delay            =
        // 音频编码会在编码数据前加initial_padding个字节，解码后需要丢弃这么多字节，才能得到真正的音频数据
        codec->initial_padding  = par->initial_padding;
        // 音频编码会在编码数据后加trailing_padding个字节，解码后需要丢弃这么多字节，才能得到真正的音频数据
        codec->trailing_padding = par->trailing_padding;
        // 遇到不连续时需要跳过的样本数(编码时使用)
        codec->seek_preroll     = par->seek_preroll;
        break;
 
    // 字幕
    case AVMEDIA_TYPE_SUBTITLE:
        // 宽
        codec->width  = par->width;
        // 高
        codec->height = par->height;
        break;
    }
    Print2File("codec->pix_fmt : "+std::to_string(codec->pix_fmt));
    Print2File("codec->width : "+std::to_string(codec->width));
    Print2File("codec->height : "+std::to_string(codec->height));
    Print2File("codec->bit_rate : "+std::to_string(codec->bit_rate));
    Print2File("codec->bits_per_coded_sample : "+std::to_string(codec->bits_per_coded_sample));
    Print2File("codec->bits_per_raw_sample : "+std::to_string(codec->bits_per_raw_sample));
    Print2File("lhs_copy_parameters_to_context Mid =================");
    // 拷贝extradata，对于h.264视频流来说，sps和pps存在此处
    if (par->extradata) {
        av_freep(&codec->extradata);
        Print2File("par->extradata_size : "+std::to_string(par->extradata_size));
        codec->extradata = (uint8_t*)av_mallocz(par->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!codec->extradata)
            return AVERROR(ENOMEM);
        memcpy(codec->extradata, par->extradata, par->extradata_size);
        codec->extradata_size = par->extradata_size;
    }

    Print2File("lhs_copy_parameters_to_context END =================");
 
    return 0;
}


static int lhs_copy_context_to_parameters(CodecParWithoutExtraData *codec,
                                  const AVCodecContext *par)
{
    Print2File("~~~~~~~~~~~~ lhs_copy_context_to_parameters in ");
    // 编码器类型-视频？音频？字幕？
    codec->codec_type = par->codec_type;
    // 编码器id，可以找到唯一的编码器
    codec->codec_id   = par->codec_id;
    // 编码器tag，编码器的另外一种表述
    codec->codec_tag  = par->codec_tag;
 
    // 平均码率
    codec->bit_rate              = par->bit_rate;
    // 采样点编码后所占位数
    codec->bits_per_coded_sample = par->bits_per_coded_sample;
    // 原始采样点所占位数
    codec->bits_per_raw_sample   = par->bits_per_raw_sample;
    // 编码profile
    codec->profile               = par->profile;
    // 编码level
    codec->level                 = par->level;
 
    // 针对不同的编码器类型赋值
    switch (par->codec_type) {
    // 视频
    case AVMEDIA_TYPE_VIDEO:
        // 像素格式
        codec->format                = (int)par->pix_fmt;
        // 视频宽度
        codec->width                  = par->width;
        // 视频高度
        codec->height                 = par->height;
        // 场帧顺序-逐行扫描？顶场先编码先展示，顶场先编码后展示，底场先编码先展示，底场先编码后展示
        codec->field_order            = par->field_order;
        // 颜色相关的几个参数，不太懂
        codec->color_range            = par->color_range;
        codec->color_primaries        = par->color_primaries;
        codec->color_trc              = par->color_trc;
        codec->color_space            = par->colorspace;
        codec->chroma_location        = par->chroma_sample_location;
        // SAR，样本的宽高比
        codec->sample_aspect_ratio    = par->sample_aspect_ratio;
        // 是否有b帧，对应par中是否有视频延迟
        codec->video_delay            = par->has_b_frames;
        break;
 
    // 音频
    case AVMEDIA_TYPE_AUDIO:
        // 采样格式
        codec->format           = (int)par->sample_fmt;
        // 通道布局
        codec->channel_layout   = par->channel_layout;
        // 通道数
        codec->channels         = par->channels;
        // 采样率
        codec->sample_rate      = par->sample_rate;
        // 音频包中的对齐块大小
        codec->block_align      = par->block_align;
        // 一帧音频中单个通道采样个数(Number of samples per channel in an audio fram)
        codec->frame_size       = par->frame_size;
        // 编码延迟，为送入若干采样点之后编码器才能产生合法的输出，此时的采样点个数
        // codec->delay            =
        // 音频编码会在编码数据前加initial_padding个字节，解码后需要丢弃这么多字节，才能得到真正的音频数据
        codec->initial_padding  = par->initial_padding;
        // 音频编码会在编码数据后加trailing_padding个字节，解码后需要丢弃这么多字节，才能得到真正的音频数据
        codec->trailing_padding = par->trailing_padding;
        // 遇到不连续时需要跳过的样本数(编码时使用)
        codec->seek_preroll     = par->seek_preroll;
        break;
 
    // 字幕
    case AVMEDIA_TYPE_SUBTITLE:
        // 宽
        codec->width  = par->width;
        // 高
        codec->height = par->height;
        break;
    }
 
    // 拷贝extradata，对于h.264视频流来说，sps和pps存在此处
    if (par->extradata) {
        av_freep(&codec->extradata);
        codec->extradata = (uint8_t*)av_mallocz(par->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!codec->extradata)
            return AVERROR(ENOMEM);
        memcpy(codec->extradata, par->extradata, par->extradata_size);
        codec->extradata_size = par->extradata_size;
    }
 
    return 0;
}

static int lhs_copy_parameters_to_myParameters(CodecParWithoutExtraData *codec,
                                  const AVCodecParameters *par)
{
    Print2File("lhs_copy_parameters_to_myParameters in ~~~~~~~~~~~~");
    // 编码器类型-视频？音频？字幕？
    codec->codec_type = par->codec_type;
    // 编码器id，可以找到唯一的编码器
    codec->codec_id   = par->codec_id;
    // 编码器tag，编码器的另外一种表述
    codec->codec_tag  = par->codec_tag;
 
    // 平均码率
    codec->bit_rate              = par->bit_rate;
    // 采样点编码后所占位数
    codec->bits_per_coded_sample = par->bits_per_coded_sample;
    // 原始采样点所占位数
    codec->bits_per_raw_sample   = par->bits_per_raw_sample;
    // 编码profile
    codec->profile               = par->profile;
    // 编码level
    codec->level                 = par->level;
 
    // 针对不同的编码器类型赋值
    switch (par->codec_type) {
    // 视频
    case AVMEDIA_TYPE_VIDEO:
        // 像素格式
        codec->format                = par->format;
        // 视频宽度
        codec->width                  = par->width;
        // 视频高度
        codec->height                 = par->height;
        // 场帧顺序-逐行扫描？顶场先编码先展示，顶场先编码后展示，底场先编码先展示，底场先编码后展示
        codec->field_order            = par->field_order;
        // 颜色相关的几个参数，不太懂
        codec->color_range            = par->color_range;
        codec->color_primaries        = par->color_primaries;
        codec->color_trc              = par->color_trc;
        codec->color_space             = par->color_space;
        codec->chroma_location        = par->chroma_location;
        // SAR，样本的宽高比
        codec->sample_aspect_ratio    = par->sample_aspect_ratio;
        // 是否有b帧，对应par中是否有视频延迟
        codec->video_delay           = par->video_delay;
        break;
 
    // 音频
    case AVMEDIA_TYPE_AUDIO:
        // 采样格式
        codec->format       = par->format;
        // 通道布局
        codec->channel_layout   = par->channel_layout;
        // 通道数
        codec->channels         = par->channels;
        // 采样率
        codec->sample_rate      = par->sample_rate;
        // 音频包中的对齐块大小
        codec->block_align      = par->block_align;
        // 一帧音频中单个通道采样个数(Number of samples per channel in an audio fram)
        codec->frame_size       = par->frame_size;
        // 编码延迟，为送入若干采样点之后编码器才能产生合法的输出，此时的采样点个数
        // codec->delay            =
        // 音频编码会在编码数据前加initial_padding个字节，解码后需要丢弃这么多字节，才能得到真正的音频数据
        codec->initial_padding  = par->initial_padding;
        // 音频编码会在编码数据后加trailing_padding个字节，解码后需要丢弃这么多字节，才能得到真正的音频数据
        codec->trailing_padding = par->trailing_padding;
        // 遇到不连续时需要跳过的样本数(编码时使用)
        codec->seek_preroll     = par->seek_preroll;
        break;
 
    // 字幕
    case AVMEDIA_TYPE_SUBTITLE:
        // 宽
        codec->width  = par->width;
        // 高
        codec->height = par->height;
        break;
    }
 
    // 拷贝extradata，对于h.264视频流来说，sps和pps存在此处
    // 这一段只能在“同一个进程使用”
    if (par->extradata) {
        av_freep(&codec->extradata);
        codec->extradata = (uint8_t*)av_mallocz(par->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!codec->extradata)
            return AVERROR(ENOMEM);
        memcpy(codec->extradata, par->extradata, par->extradata_size);
        codec->extradata_size = par->extradata_size;
    }
    // 这一段只能在“同一个进程使用”
    Print2File("lhs_copy_parameters_to_myParameters End ~~~~~~~~~~~~");
    return 0;
}

// static int lhs_copy_myParameters_to_myParameters(CodecParWithoutExtraData *codec,
//                                   const CodecParWithoutExtraData *par)
// {
//     Print2File("lhs_copy_myParameters_to_myParameters in ~~~~~~~~~~~~");
//     // 编码器类型-视频？音频？字幕？
//     codec->codec_type = par->codec_type;
//     // 编码器id，可以找到唯一的编码器
//     codec->codec_id   = par->codec_id;
//     // 编码器tag，编码器的另外一种表述
//     codec->codec_tag  = par->codec_tag;
//     Print2File("lhs_copy_myParameters_to_myParameters 222222222");
//     // 平均码率
//     codec->bit_rate              = par->bit_rate;
//     // 采样点编码后所占位数
//     codec->bits_per_coded_sample = par->bits_per_coded_sample;
//     // 原始采样点所占位数
//     codec->bits_per_raw_sample   = par->bits_per_raw_sample;
//     // 编码profile
//     codec->profile               = par->profile;
//     // 编码level
//     codec->level                 = par->level;
 
//     // 针对不同的编码器类型赋值
//     switch (par->codec_type) {
//     // 视频
//     case AVMEDIA_TYPE_VIDEO:
//         // 像素格式
//         codec->format                = par->format;
//         // 视频宽度
//         codec->width                  = par->width;
//         // 视频高度
//         codec->height                 = par->height;
//         // 场帧顺序-逐行扫描？顶场先编码先展示，顶场先编码后展示，底场先编码先展示，底场先编码后展示
//         codec->field_order            = par->field_order;
//         // 颜色相关的几个参数，不太懂
//         codec->color_range            = par->color_range;
//         codec->color_primaries        = par->color_primaries;
//         codec->color_trc              = par->color_trc;
//         codec->color_space             = par->color_space;
//         codec->chroma_location        = par->chroma_location;
//         // SAR，样本的宽高比
//         codec->sample_aspect_ratio    = par->sample_aspect_ratio;
//         // 是否有b帧，对应par中是否有视频延迟
//         codec->video_delay           = par->video_delay;
//         break;
 
//     // 音频
//     case AVMEDIA_TYPE_AUDIO:
//         // 采样格式
//         codec->format       = par->format;
//         // 通道布局
//         codec->channel_layout   = par->channel_layout;
//         // 通道数
//         codec->channels         = par->channels;
//         // 采样率
//         codec->sample_rate      = par->sample_rate;
//         // 音频包中的对齐块大小
//         codec->block_align      = par->block_align;
//         // 一帧音频中单个通道采样个数(Number of samples per channel in an audio fram)
//         codec->frame_size       = par->frame_size;
//         // 编码延迟，为送入若干采样点之后编码器才能产生合法的输出，此时的采样点个数
//         // codec->delay            =
//         // 音频编码会在编码数据前加initial_padding个字节，解码后需要丢弃这么多字节，才能得到真正的音频数据
//         codec->initial_padding  = par->initial_padding;
//         // 音频编码会在编码数据后加trailing_padding个字节，解码后需要丢弃这么多字节，才能得到真正的音频数据
//         codec->trailing_padding = par->trailing_padding;
//         // 遇到不连续时需要跳过的样本数(编码时使用)
//         codec->seek_preroll     = par->seek_preroll;
//         break;
 
//     // 字幕
//     case AVMEDIA_TYPE_SUBTITLE:
//         // 宽
//         codec->width  = par->width;
//         // 高
//         codec->height = par->height;
//         break;
//     }
//     Print2File("lhs_copy_myParameters_to_myParameters par->extradata ============");
//     // 拷贝extradata，对于h.264视频流来说，sps和pps存在此处
//     if (par->extradata) {
//         av_freep(&codec->extradata);
//         codec->extradata = (uint8_t*)av_mallocz(par->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
//         if (!codec->extradata)
//             return AVERROR(ENOMEM);
//         memcpy(codec->extradata, par->extradata, par->extradata_size);
//         codec->extradata_size = par->extradata_size;
//     }
//     Print2File("lhs_copy_myParameters_to_myParameters End ============");
//     return 0;
// }

#endif // SODTP_BLOCK_H