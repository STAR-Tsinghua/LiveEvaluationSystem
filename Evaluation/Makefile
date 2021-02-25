

LOG_PATH ?= ../LiveSystem/logs
DATA_PATH ?= ./data/
PIC_PATH ?= ./picture/
CMP_PATH ?= ../command

all: data fig pdf

data: data_latency.csv

fig: data_serverLatency.svg data_playerLatency.svg data_frames_jitter data_buffer_container.svg data_frame_latency.svg

pdf:svg2pic

# txt2csv
data_latency.csv: txt2csv_lantency.py $(shell find $(LOG_PATH))
	python3 $< $(LOG_PATH) $(DATA_PATH)$@

# csv2svg
data_serverLatency.svg: csv2svg_serverLatency.py $(shell find $(LOG_PATH))
	python $< $(DATA_PATH)data_latency.csv $(DATA_PATH)

data_playerLatency.svg: csv2svg_playerLatency.py $(shell find $(LOG_PATH))
	python $< $(DATA_PATH)data_latency.csv $(DATA_PATH)

# 生成多个svg
data_frames_jitter: csv2svg_frames_jitter.py $(shell find $(LOG_PATH))
	python $< $(DATA_PATH)data_latency.csv $(DATA_PATH)

data_buffer_container.svg: csv2svg_buffer_container.py $(shell find $(LOG_PATH))
	python $< $(DATA_PATH)data_latency.csv $(DATA_PATH)

data_frame_latency.svg: csv2svg_frame_latency.py $(shell find $(LOG_PATH))
	python $< $(DATA_PATH)data_latency.csv $(DATA_PATH)

# svg2pic
svg2pic: tool_svg2pic.py
	python3 $< $(DATA_PATH) $(PIC_PATH)