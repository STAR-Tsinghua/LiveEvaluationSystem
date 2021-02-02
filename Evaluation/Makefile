

LOG_PATH ?= ~/Documents/yuming/result/log
PIC_PATH ?= ~/Documents/yuming/result/pic
CMP_PATH ?= ~/Documents/yuming/decode

all: fig_bw fig_rtt

fig_bw: data_bw.csv

fig_rtt: data_rtt.csv

data.csv: filter.py $(shell find $(LOG_PATH))
	python3 $< $(LOG_PATH) $@

data_stall.csv: filter_stall.py $(shell find $(LOG_PATH))
	python3 $< $(LOG_PATH) $@

data_decode.csv: filter_decode.py $(shell find $(LOG_PATH))
	python3 $< $(LOG_PATH) $@

data_ssim.csv: filter_ssim.py $(shell find $(PIC_PATH)) $(shell find $(CMP_PATH))
	python3 $< $(PIC_PATH) $(CMP_PATH) $@

data_pic.csv: filter_pic.py $(shell find $(PIC_PATH)) $(shell find $(CMP_PATH))
	python3 $< $(PIC_PATH) $(CMP_PATH) $@
