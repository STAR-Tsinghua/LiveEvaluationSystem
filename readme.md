# LiveStream System

This project implements a simple live stream system based on DTP/quiche/UDP

## Download advice

This repo uses git submodule to handle addtional depenencies for this repo. While in Mainland China, the download speed of some library may be desperate. It is recommend to clone this repo first and then handle DTP dependency manually.

```sh
$ git clone https://github.com/STAR-Tsinghua/LiveEvaluationSystem.git -b ubuntu18.04
$ git submodule init
$ git submodule update
```

DTP needs boringssl library, whose downloading may be frustrating in Mainland China. You may clone DTP repo first and then download boringssl src code from github by .zip file.

## Environment settings

The system is tested on Ubuntu18.04 and Ubuntu20.04.

### Toolchain

- gcc: 9.4.0
- cmake: 3.16.3
- rustc: 1.53.0
- go: 1.17.3

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

You can refer to this blog for further information about installing FFmpeg from src code https://blog.csdn.net/wangyjfrecky/article/details/80998303 .

You may have to install libx264 and/or libx265, just use `sudo apt install libx264-dev libx265-dev` to do so.

### Opencv (3.4.0 for Ubuntu 18.04/4.9.0 for Ubuntu20.04)

It is similar to install Opencv as it is for FFmpeg. Use tag 3.4.0 for example:

Get Opencv from github https://github.com/opencv/opencv.git . You need `cmake` to build Opencv.

You can refer to this blog for some advice https://blog.csdn.net/m0_38076563/article/details/79709862

```sh
$ git clone https://github.com/opencv/opencv.git
$ cd opencv
$ git fetch --tags
$ git checkout 3.4.0 -b 3.4.0
$ mkdir build
$ cd build
$ cmake ..
$ make -j8
$ sudo make install
```

You need to check that after running `cmake ..`, options of FFMPEG should all be ON. If you cannot find ffmpeg components, you may refer to the following answer: https://stackoverflow.com/questions/5492919/cmake-cant-find-ffmpeg-in-custom-install-path .

You may have to configure link libraries for opencv (and/or ffmpeg). You can refer to the last several parts of https://blog.csdn.net/wangyjfrecky/article/details/80998303 for help of linking libraries.

In Ubuntu 20.04, you may encounter compiling error when the process reaches near 99% using opencv 3.4.0. The current solution is updating the opencv version to 4.9.0. Refer to the following blog for more information: https://blog.csdn.net/shengliz/article/details/116739660.

### SDL 2.0

Install this library by `sudo apt install libsdl2-dev`

### uthash.h

Install this lib by `sudo apt install uthash-dev`

### Build essentials and CMake

`sudo apt install build-essential cmake`

## Compilation process and toolchains

### DTP

You need to compile DTP first, which needs both Rust and Go language supports.

#### Rust dependency

Rust may be fetched and installed like this:

```sh
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

Then use `rustup toolchain install 1.53.0` to install the specific version of toolchain.

#### Go dependency

Go need to download src file and compile. Please refer to the golang official documentation for more information https://golang.google.cn/doc/install .

#### Change source of Rust and Go

In Mainland China, it is better to configure source for both Rust and Go. The following websites may help.

* https://mirrors.tuna.tsinghua.edu.cn/help/crates.io-index.git/
* https://blog.csdn.net/Kevin_lady/article/details/108700915

#### Compile DTP

Compile DTP first to get library in release mode.

```sh
$ cd submodule/DTP
$ cargo build --release
```

Please note the version of rustc should be 1.53.0 (`rustc -V`). If not, you can use `rustup toolchain` to install it. It is found out that the latest rustc version may cause library linking problem.

### Apply patches

Run `apply_patch.sh` at the root directory of this project. This patch adds Cargo.lock file to both submodules to ensure correct depency version.

### Compile executable

Go to `LiveSystem` and then use the commands

```sh 
$ sudo make rb # r(run)b(binary(maybe)) Compile the executable, and create log directory.
$ sudo make rd # r(run)d(dtp). Run dtp server and client locally
$ sudo make k # k(kill) . Kill both server and client
$ sudo make ru # (run udp). Run udp server and client locally
$ sudo make k
$ sudo make rq # run quiche server and client
$ sudo make kq # you can kill dtp or quiche c/s by command like this
```

You need to set up configuration in `LiveSystem/config/[dtp|quiche].conf` to choose the camera and resolution before running the code. The format like `0 360` meaning using device 0 and resolution 360p.

After running `make rd`, the system should be running with a pop-up window displaying live stream video from the camera.

Please remember to kill both sides after test, or it would block the next test.

## Log files

You may find some useful logs in log files generated in both `LiveSystem` and `LiveSystem/logs`

## Commands

The `command` directory contains some shell commands to run the server and client (for both Linux and Mac OS). You should check the `Makefile` to make sure you understand how to run the server and client.

The following code shows a simple example to run all the live system targets (some features are only available in IPv6):

```bash
$ LD_LIBRARY_PATH=. ./r_dtp_server SERVER_IP SERVER_PORT [CONFIG_FILE(default: ./config/dtp.conf)]
$ LD_LIBRARY_PATH=. ./r_dtp_play SERVER_IP SERVER_PORT CLIENT_IP CLIENT_PORT [CONFIG_FILE(default: ./config/dtp.conf)]
$ LD_LIBRARY_PATH=. ./r_quiche_server SERVER_IP SERVER_PORT [CONFIG_FILE(default: ./config/quiche.conf)]
$ LD_LIBRARY_PATH=. ./r_quiche_play SERVER_IP SERVER_PORT [CONFIG_FILE(default: ./config/quiche.conf)]
$ LD_LIBRARY_PATH=. ./r_udp_server SERVER_IP SERVER_PORT [CONFIG_FILE(default: ./config/udp.conf)]
$ LD_LIBRARY_PATH=. ./r_udp_play SERVER_IP SERVER_PORT [CONFIG_FILE(default: ./config/udp.conf)]
```

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
