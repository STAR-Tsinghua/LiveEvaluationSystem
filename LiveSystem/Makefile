
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

TARGETS = dtp_server dplay

LOGS = logs

all: $(TARGETS)

dtp_server : dtp_server.cxx
	$(CX) $(CXFLAGS) $(OPENCVFLAGS) -o $@ $^ $(INCS) $(LIBS)

dplay : p_dplay.cxx p_sodtp_jitter.cxx sodtp_block.cxx
	$(CX) $(CXFLAGS) -o $@ $^ $(INCS) $(LIBS)

clean:
	rm -rf $(TARGETS)

clog:
	rm -rf $(LOGS)
	mkdir $(LOGS)

rb:
	rm -rf $(TARGETS)
	make clog
	make all

r:
	Open ../command/lhs_startBoth.command

rr:
	make clog
	Open ../command/lhs_startBoth.command

k:
	Open ../command/lhs_killBoth.command