#ifndef P_SODTP_DECODER_H
#define P_SODTP_DECODER_H


extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "SDL2/SDL.h"
}

#include <sodtp_block.h>

class SodtpJitter;


class Decoder
{
public:
    AVCodecContext          *pVCodecCtx;
    struct SwsContext       *pSwsCtx;
    AVFrame                 *pFrame;
    AVFrame                 *pFrameRGB;
    AVFrame                 *pFrameYUV;
    AVFrame                 *pFrameShow;
    AVPacket                *pPacket;
    SodtpJitter             *pJitter;
    SodtpBlockPtr           pBlock;
    // iStream should be pJitter->stream_id.
    int                     iStream;
    // iFrame should be pBlock->block_id + 1.
    int                     iFrame;

    // Number of received blocks, and iBlock should be the same with the
    // number of decoded pictures when each block can be parsed into a
    // picture without flushing the codec.
    int                     iBlock;

    int                     iStart;

    // path for saving decoded picture.
    const char              *path;

    // mutex for pFrameShow
    std::mutex              mutex;



    // SDL_Rect                *pRect;
    SDL_Texture             *pTexture;

public:
    Decoder() {
        iStart = 0;
        pTexture = NULL;
    }
};

#endif // P_SODTP_DECODER_H