
CC = gcc
CX = g++


# DTP_DIR			= ../submodule/quiche
DTP_DIR 		= ../submodule/DTP
DTP_INC			= $(DTP_DIR)/include
DTP_LIB			= $(DTP_DIR)/target/release

CXFLAGS = -std=c++11 -g
OPENCVFLAGS = `pkg-config --cflags --libs opencv`

INCS = -I$(CURDIR) -I$(DTP_INC)

LIBS = 	-lswscale -lswresample -lavformat \
		-lavdevice -lavcodec -lavutil -lavfilter \
		-lSDL2 \
		-L$(DTP_LIB) -lquiche \
		-lm -lz -lev -ldl -pthread \

TARGETS = r_dtp_server r_dtp_play r_udp_server r_udp_play 
# r_quiche_server r_quiche_play
# 先放着只为实验图的"实验"

LOGS = logs

all: $(TARGETS)

# ================= dtp =================
r_dtp_server : s_dtp_server.cxx
	$(CX) $(CXFLAGS) $(OPENCVFLAGS) -o $@ $^ $(INCS) $(LIBS)

r_dtp_play : p_dtp_play.cxx p_sodtp_jitter.cxx sodtp_block.cxx
	$(CX) $(CXFLAGS) -o $@ $^ $(INCS) $(LIBS)

# ================= udp =================
r_udp_server : s_udp_server.cxx
	$(CX) $(CXFLAGS) $(OPENCVFLAGS) -o $@ $^ $(INCS) $(LIBS)

r_udp_play : p_udp_play.cxx p_sodtp_jitter.cxx sodtp_block.cxx
	$(CX) $(CXFLAGS) -o $@ $^ $(INCS) $(LIBS)

# ================= quiche =================
# r_quiche_server : s_quiche_server.cxx
# 	$(CX) $(CXFLAGS) $(OPENCVFLAGS) -o $@ $^ $(INCS) $(LIBS)

# r_quiche_play : p_quiche_play.cxx p_sodtp_jitter.cxx sodtp_block.cxx
# 	$(CX) $(CXFLAGS) -o $@ $^ $(INCS) $(LIBS)



clean:
	rm -rf $(TARGETS)

cl:
	rm -rf $(LOGS)
	mkdir $(LOGS)

rb:
	rm -rf $(TARGETS)
	make cl
	make all

# dtp
rd:
	Open ../command/lhs_dtp_startBoth.command

# udp
ru:
	Open ../command/lhs_udp_startBoth.command

# quiche
# rq:
# 	Open ../command/lhs_quiche_startBoth.command

k:
	Open ../command/lhs_killBoth.command