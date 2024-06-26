CC = gcc
CX = g++

# 注意对应目录的关系
# DTP_DIR			= ../submodule/quiche
DTP_DIR 		= ../submodule/DTP
DTP_INC			= $(DTP_DIR)/include
DTP_LIB			= $(DTP_DIR)/target/release

FFMPEG_ROOT = /usr/local/ffmpeg
FFMPEG_LIB = $(FFMPEG_ROOT)/lib
FFMPEG_INC = $(FFMPEG_ROOT)/include
FFMPEG_SHARED = $(FFMPEG_ROOT)/shared
FFMPEG_BIN = $(FFMPEG_ROOT)/bin

QUICHE_DIR		= ../submodule/quiche
QUICHE_INC		= $(QUICHE_DIR)/include
QUICHE_LIB		= $(QUICHE_DIR)/target/release

CXFLAGS = -std=c++11 # -g
OPENCVFLAGS = `pkg-config --cflags --libs opencv`

INCS = -I$(CURDIR) -I$(CURDIR)/sodtp -I$(CURDIR)/util -I$(FFMPEG_INC)

LIBS_DIR = -L$(CURDIR) -L$(FFMPEG_LIB) -L$(FFMPEG_SHARED) -L$(LIBCRYPTO_DIR) -L$(LIBSSL_DIR)

LIBS =  -lswscale -lswresample -lavformat \
		-lavdevice -lavcodec -lavutil -lavfilter \
		-lSDL2 \
		-lm -lz -lev -ldl -pthread \

TARGETS = r_dtp_server r_dtp_play r_udp_server r_udp_play \
r_quiche_server r_quiche_play
# 先放着只为实验图的"实验"

LOGS = logs

LOGS_DTP = logs/ev_dtp

LOGS_UDP = logs/ev_udp

LOGS_QUICHE = logs/ev_quiche

all: $(TARGETS)

# ================= dtp =================
r_dtp_server : s_dtp_server.cxx libdtp.so
	$(CX) $(CXFLAGS) -o $@ $^ $(INCS) -I$(DTP_INC) -L$(DTP_LIB) $(LIBS_DIR) -ldtp $(LIBS) $(OPENCVFLAGS)

r_dtp_play : p_dtp_play.cxx libsodtp.so libdtp.so
	$(CX) $(CXFLAGS) -o $@ $^ $(INCS) -I$(DTP_INC) -L$(DTP_LIB) $(LIBS_DIR) -ldtp $(LIBS) -lsodtp

# ================= udp =================
r_udp_server : s_udp_server.cxx
	$(CX) $(CXFLAGS)  -o $@ $^ $(INCS) $(LIBS_DIR) $(LIBS) $(OPENCVFLAGS)

r_udp_play : p_udp_play.cxx libsodtp.so
	$(CX) $(CXFLAGS) -o $@ $^ $(INCS) $(LIBS_DIR) $(LIBS) -lsodtp

# ================= quiche =================
r_quiche_server : s_quiche_server.cxx libquiche.so
	$(CX) $(CXFLAGS) -o $@ $^ $(INCS) -I$(QUICHE_INC) -L$(QUICHE_LIB) $(LIBS_DIR) $(LIBS) $(OPENCVFLAGS) -lssl -lquiche 

r_quiche_play : p_quiche_play.cxx libsodtp.so libquiche.so
	$(CX) $(CXFLAGS) -o $@ $^ $(INCS) -I$(QUICHE_INC) -L$(QUICHE_LIB) $(LIBS_DIR) -lquiche $(LIBS) -lsodtp

clean:
	rm -rf $(TARGETS)

cl_all:
	sudo rm -rf $(LOGS)
	mkdir $(LOGS)

cl_dtp:
	sudo rm -rf $(LOGS_DTP)
	mkdir $(LOGS_DTP)

cl_udp:
	sudo rm -rf $(LOGS_UDP)
	mkdir $(LOGS_UDP)

cl_quiche:
	sudo rm -rf $(LOGS_QUICHE)
	mkdir $(LOGS_QUICHE)

rb:
	rm -rf $(TARGETS)
	rm -rf *.so
	make cl_all
	make all

# dtp
rd: libsodtp.so libdtp.so r_dtp_server r_dtp_play 
	make cl_dtp
	LD_LIBRARY_PATH=. ./r_dtp_server 127.0.0.1 9093 1>dtp_server.log 2>&1 &
	sleep 2s
	LD_LIBRARY_PATH=. ./r_dtp_play 127.0.0.1 9093 127.0.0.1 9099 1>dtp_play.log 2>&1 &

rdc: libsodtp.so libdtp.so r_dtp_play 
	LD_LIBRARY_PATH=. ./r_dtp_play ::1 9093 ::1 9099 1>dtp_play.log 2>&1 &

rds: libsodtp.so libdtp.so r_dtp_server 
	make cl_dtp
	LD_LIBRARY_PATH=. ./r_dtp_server ::1 9093 1>dtp_server.log 2>&1 &

# udp
ru: r_udp_server r_udp_play libsodtp.so
	make cl_udp
	LD_LIBRARY_PATH=. ./r_udp_server 127.0.0.1 9095 1> udp_server.log 2>&1 &
	sleep 2s
	LD_LIBRARY_PATH=. ./r_udp_play 127.0.0.1 9095 1> udp_player.log 2>&1 &

ruc: r_udp_server r_udp_play libsodtp.so
	LD_LIBRARY_PATH=. ./r_udp_play ::1 9095 1> udp_player.log 2>&1 &

rus: r_udp_server r_udp_play libsodtp.so
	make cl_udp
	LD_LIBRARY_PATH=. ./r_udp_server ::1 9095 1> udp_server.log 2>&1 &

# quiche
rq: libsodtp.so r_quiche_server r_quiche_play  libquiche.so
	make cl_quiche
	LD_LIBRARY_PATH=. ./r_quiche_server 127.0.0.1 9094 1>quiche_server.log 2>&1 &
	sleep 2s
	LD_LIBRARY_PATH=. ./r_quiche_play 127.0.0.1 9094 1>quiche_play.log 2>&1 &

rqc: libsodtp.so r_quiche_play libquiche.so
	LD_LIBRARY_PATH=. ./r_quiche_play ::1 9094 1> quiche_player.log 2>&1 &

rqs: r_quiche_server r_quiche_play libquiche.so
	make cl_quiche
	LD_LIBRARY_PATH=. ./r_quiche_server ::1 9094 1> quiche_server.log 2>&1 &

k:
	../command/lhs_killBoth.sh

kq:
	../command/lhs_killq.sh

kd:
	../command/lhs_killd.sh

libsodtp.so: sodtp/sodtp_block.cxx sodtp/p_sodtp_jitter.cxx
	$(CX) $(CXFLAGS) -fPIC -shared -o $@ $^ $(INCS) $(LIBS_DIR) $(LIBS)

libdtp.so:
	cd ../submodule/DTP && cargo build --release --features "boringssl-vendored fec"
	cp -f ../submodule/DTP/target/release/libquiche.so ./libdtp.so

libquiche.so:
	cd ../submodule/quiche && cargo build --release
	cp -f ../submodule/quiche/target/release/libquiche.so ./libquiche.so

libdtp.a:
	cd ../submodule/DTP && cargo build --release --features "boringssl-vendored fec"
	mv -f ../submodule/DTP/target/release/libquiche.a ./libdtp.a
	rm -f ../submodule/DTP/target/release/libquiche.so

libquiche.a:
	cd ../submodule/quiche && cargo build --release
	mv -f ../submodule/quiche/target/release/libquiche.a ./libquiche.a
	rm -f ../submodule/quiche/target/release/libquiche.so
