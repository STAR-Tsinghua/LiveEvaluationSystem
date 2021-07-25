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

CXFLAGS = -std=c++11 -g
OPENCVFLAGS = `pkg-config --cflags --libs opencv`

INCS = -I$(CURDIR) -I$(DTP_INC) -I$(FFMPEG_INC)

LIBS = -L$(FFMPEG_LIB) -L$(FFMPEG_SHARED) -lswscale -lswresample -lavformat \
		-lavdevice -lavcodec -lavutil -lavfilter \
		-lSDL2 \
		-L$(DTP_LIB) -lquiche \
		-lm -lz -lev -ldl -pthread \

TARGETS = r_dtp_server r_dtp_play r_udp_server r_udp_play 
# r_quiche_server r_quiche_play
# 先放着只为实验图的"实验"

LOGS = logs

LOGS_DTP = logs/ev_dtp

LOGS_UDP = logs/ev_udp

all: $(TARGETS)

# ================= dtp =================
r_dtp_server : s_dtp_server.cxx
	$(CX) $(CXFLAGS)  -o $@ $^ $(INCS) $(LIBS) $(OPENCVFLAGS)

r_dtp_play : p_dtp_play.cxx p_sodtp_jitter.cxx sodtp_block.cxx
	$(CX) $(CXFLAGS) -o $@ $^ $(INCS) $(LIBS)

# ================= udp =================
r_udp_server : s_udp_server.cxx
	$(CX) $(CXFLAGS)  -o $@ $^ $(INCS) $(LIBS) $(OPENCVFLAGS)

r_udp_play : p_udp_play.cxx p_sodtp_jitter.cxx sodtp_block.cxx
	$(CX) $(CXFLAGS) -o $@ $^ $(INCS) $(LIBS)

# ================= quiche =================
# r_quiche_server : s_quiche_server.cxx
# 	$(CX) $(CXFLAGS) $(OPENCVFLAGS) -o $@ $^ $(INCS) $(LIBS)

# r_quiche_play : p_quiche_play.cxx p_sodtp_jitter.cxx sodtp_block.cxx
# 	$(CX) $(CXFLAGS) -o $@ $^ $(INCS) $(LIBS)



clean:
	rm -rf $(TARGETS)

cl_all:
	rm -rf $(LOGS)
	mkdir $(LOGS)

cl_dtp:
	rm -rf $(LOGS_DTP)
	mkdir $(LOGS_DTP)

cl_udp:
	rm -rf $(LOGS_UDP)
	mkdir $(LOGS_UDP)

rb:
	rm -rf $(TARGETS)
	make cl_all
	make all

# dtp
rd:
	make cl_dtp
	LD_LIBRARY_PATH=. ./r_dtp_server 127.0.0.1 9093 1>dtp_server.log 2>&1 &
	sleep 2s
	LD_LIBRARY_PATH=. ./r_dtp_play 127.0.0.1 9093 1>dtp_play.log 2>&1 &

# udp
ru:
	make cl_udp
	./r_udp_server 127.0.0.1 9093 1> udp_server.log 2>&1 &
	sleep 2s
	./r_udp_play 127.0.0.1 9093 1> udp_player.log 2>&1 &

# quiche
# rq:
# 	Open ../command/lhs_quiche_startBoth.command

k:
	../command/lhs_killBoth.sh

