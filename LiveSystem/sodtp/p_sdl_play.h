#ifndef P_SDL_PLAY_H
#define P_SDL_PLAY_H


extern "C"
{
#include "SDL2/SDL.h"
#include <libavformat/avformat.h>
}




class SDLPlay
{
public:
    SDLPlay() {};
    ~SDLPlay() {};

    int init(int w, int h);
    void update(AVFrame *frame, SDL_Texture *texture, SDL_Rect *rect);
    void show();

public:
    int             screen_w;   // window width
    int             screen_h;   // window height
    int             pixel_w;    // texture width
    int             pixel_h;    // texture height
    SDL_Window      *screen;
    SDL_Renderer    *renderer;

    // std::mutex      _mutex;
    // SDL_Texture     *texture;

    SDL_Event       event;
};

int SDLPlay::init(int w, int h) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
        return -1;
    } 
    // SDL 2.0 supports multiple windows.
    screen_w = w;
    screen_h = h;
    screen = SDL_CreateWindow("SoDTP player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                screen_w, screen_h, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    if(!screen) {
        printf("SDL: could not create window - exiting:%s\n", SDL_GetError());  
        return -1;
    }

    renderer = SDL_CreateRenderer(screen, -1, 0);
    if (!renderer) {
        printf("SDL: could not create renderer:%s\n", SDL_GetError());
        return -1;
    }

    // texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, screen_w, screen_h);

    // if (!texture) {
    //     printf("SDL: could not create texture:%s\n", SDL_GetError());
    //     return -1;
    // }
    SDL_RenderClear(renderer);

    return 0;
}
//https://blog.csdn.net/leixiaohua1020/article/details/40895797
void SDLPlay::update(AVFrame *frame, SDL_Texture *texture, SDL_Rect *rect) {
    // scoped_lock lock(_mutex);
    SDL_UpdateTexture(texture, NULL, frame->data[0], frame->linesize[0]);
    // SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, rect);
    
    // SDL_RenderPresent(renderer);
}

void SDLPlay::show() {
    SDL_RenderPresent(renderer);
}






#endif // P_SDL_PLAY_H
