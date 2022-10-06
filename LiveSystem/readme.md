# LiveStream

The main workspace of the project. It includes source code of the LiveStream system and logs will be stored in this directory.

## Naming conventions

* `live_`: Data structures for live video capture, data storage and data transport.
* `p_`: Stands for "player". The codes to create a live stream video player.
* `r_`: Stands for "run". Executables.
* `s_`: Stands for "server". The codes to create a live stream server.
* `_dtp_`,`_quiche_`, `_udp_`: Meaning the protocol to transfer data.
* `_client`: A client that can receive data from the server with the same transport protocol. But it cannot display the video frames.
* `_play`: It can display the video frames in a window compared to the `_client`.

## Compile and Run

1. Run `make libdtp.so` or `make libquiche.so` to get the required library.
2. Compile with `sudo make rb`
3. Run with `sudo make rd`, `sudo make ru` or `sudo make rq` to test different protocols.
4. Kill the processes with `sudo make k` after each test.

Check Makefile to know more commands.

