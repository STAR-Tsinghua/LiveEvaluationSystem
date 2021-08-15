# lib sodtp

A simple media transfer library. It sends video in data blocks that fits the [DTP protocol](https://github.com/STAR-Tsinghua/DTP).

## Simple Introduction

### Mainly used in servers

* bounded_buffer: A producer-consumer buffer. It receives stream blocks.

### Mainly used in clients

* p\_decode\_video: A heavily simplified and modified file from sodtp library. It includes functions to decode packets.
* p\_sdl\_play: Use SDL2 to display AVFrame and display videos in a window.
* p\_sodtp\_decoder: A simple decoder class including a lot of decoding contextgs.
* p\_sodtp\_jitter
   1. JitterSample: A structure to store jitter information
   2. SodtpJitter: A class that uses buffer to store SodtpBlocks in order to avoid reordered packets. It works like a queue, with some states
   3. JitterBuffer: A class uses many SodtpJitter as a vector of buffers to store blocks from different streams. Then it offers `push_back` and `erase` operation.
* p\_stream\_worker: A encapsulated asychronous worker to take actions in the ev_loop.
   1. StreamWorker
     1. JitterBuffer jbuffer: Stores and gives buffered blocks
     2. SDLPlay splay: Uses SDL and decoder to show the video frames
     3. SDL\_Rect rect: Describes the window to display video
     4. vector<thread*> thds: Threads
     5. thd\_conn: The network connection thread
   2. stream\_working: a working function
   
### Utils

* sodtp_util: Utilities for timing and compare
* sodtp_config: Parser of sodtp configs
* sodtp_block.[cxx|h]: Block header and some structures to create header and block data
* util_url_file: Helper functions to parse media file into packets. Offset class StreamContext and a lot of init_resource functions.

