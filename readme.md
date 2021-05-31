# Readme
opencv version 3.4.0
ffmpeg version 4.1.3
SDL version 2.0

// 播放主函数
p_decode_video.h 1344

// 发送codecpar
bounded_buffer.h 329

// 读取SodtpBlock的read函数
sodtp_block.cxx 79

// 发送前内存组织
lhs_dtp_server.h 256

//SodtpStreamHeader结构体
sodtp_block.h 55

//SodtpBlock结构体
sodtp_block.h 81

## docker image 制作方法

基于实验室的 cpu-base

1. apt install ffmpeg, build-essential, cmake
2. git clone https://github.com/opencv/opencv.git
3. git fetch --tags
4. git checkout 3.4.0 -b 3.4.0
5. mkdir build
6. cd build && cmake ..
7. make -j64
8. apt install libev -y
9. 安装 Rust and Go 编译 DTP 项目
10. apt-get install libsdl2-2.0 # 安装 SDL2
11. apt-get install libsdl2-dev

## 修改

1. 修改了 livexTransport 中的两个 av_err2str macro 的用法。目前的用法会使得编译器报错。
2. 修改了 Makefile 中 pkg-config 命令运行的位置，调整至最后