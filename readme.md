# LiveStream Ubuntu 18.04

## Download advice

This repo uses git submodule to handle addtional depenencies for this repo. While in Mainland China, the download speed of some library may be desperate. It is recommend to clone this repo first and then handle DTP dependency manually.

```sh
$ git clone https://github.com/STAR-Tsinghua/LiveEvaluationSystem.git -b ubuntu18.04
$ git submodule init
$ git submodule update
```

DTP needs boringssl library, whose downloading may be frustrating in Mainland China. You may clone DTP repo first and then download boringssl src code from github by .zip file.

## Environment settings

The system is tested on Ubuntu18.04 virtual machine and it indicates that the system is operating under the following environments.

### FFmpeg version 4.1.3

```sh
$ ffmpeg -version
ffmpeg version n4.1.3 Copyright (c) 2000-2019 the FFmpeg developers
built with gcc 7 (Ubuntu 7.5.0-3ubuntu1~18.04)
configuration: --enable-shared --enable-gpl --enable-libx264 --enable-libx265 --prefix=/usr/local/ffmpeg
libavutil      56. 22.100 / 56. 22.100
libavcodec     58. 35.100 / 58. 35.100
libavformat    58. 20.100 / 58. 20.100
libavdevice    58.  5.100 / 58.  5.100
libavfilter     7. 40.101 /  7. 40.101
libswscale      5.  3.100 /  5.  3.100
libswresample   3.  3.100 /  3.  3.100
libpostproc    55.  3.100 / 55.  3.100
```

It is tested that you should install FFmpeg first.

Install FFmpeg from src code. I got the source code from github with tag n4.1.3.

```sh
$ git clone https://github.com/FFmpeg/FFmpeg.git
$ cd FFmpeg
$ git fetch --tags
$ git checkout n4.1.3 -b n4.1.3
```

Then configure the source code with `./configure`. The following configuration is proved to be OK on my virtual machine.

`./configure --enable-shared --enable-gpl --enable-libx264 --enable-libx265 --prefix=/usr/local/ffmpeg`

You can reference this blog for further information about installing FFmpeg from src code [](https://blog.csdn.net/wangyjfrecky/article/details/80998303).

You may have to install libx264 and/or libx265, just use `sudo apt install libx264-dev libx265-dev` to do so.

### Opencv 3.4.0

It is similar to install Opencv as it is for FFmpeg. Use tag 3.4.0 for Opencv from github [](https://github.com/opencv/opencv.git). You need cmake to build Opencv

You can reference this blog for some advice [](blog.csdn.net/m0_38076563/article/details/79709862)

You don't need to configure other parameters for Opencv.

### SDL 2.0

Install this library by `sudo apt install libsdl2-2.0 libsdl2-dev`

## Compilation process and toolchains

### DTP

You need to compile DTP first, which needs both Rust and Go language supports.

Rust may be fetched and installed like this:

```sh
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

Go need to download src file and compile. Please refer to the golang official documentation for more information [](https://golang.google.cn/doc/install)

In Mainland China, it is better to configure source for both Rust and Go. The following websites may help.

* https://mirrors.tuna.tsinghua.edu.cn/help/crates.io-index.git/
* https://blog.csdn.net/Kevin_lady/article/details/108700915

Compile DTP first to get library in release mode.

```sh
$ cd submodule/DTP
$ cargo build --release
```

### Compile executable

Go to `LiveSystem` and then use the commands

```sh 
$ cp ../submodule/DTP/target/release/libquiche.so ./ # need to copy this file manually
$ make rb # r(run)b(binary(maybe)) Compile the executable, and create log directory. You may need super user authorization for this.
$ make rd # r(run)d(dtp). Run dtp server and client locally
$ make k # k(kill) . Kill both server and client
$ make ru # (run udp). Run udp server and client locally
$ make k
```
After running `make rd`, the system should be running with a pop-up window displaying live stream video from the camera.

Please remember to kill both sides after test, or it would block the next test.

## Log files

You may find some useful logs in log files generated in both `LiveSystem` and `LiveSystem/logs`

## Commands

The `command` directory contains some shell commands to run the server and client (for both Linux and Mac OS). You should check the `Makefile` to make sure you understand how to run the server and client.

## Original readme contents

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

### (WIP) docker image 制作方法 

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
10. apt install libsdl2-2.0 
11. apt install libsdl2-dev


### 修改

1. 修改了 livexTransport 中的两个 av_err2str macro 的用法。目前的用法会使得编译器报错。
2. 修改了 Makefile 中 pkg-config 命令运行的位置，调整至最后
