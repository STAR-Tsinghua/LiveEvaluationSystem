

LOG_PATH ?= ../LiveSystem/logs
DATA_PATH ?= ./data/
PIC_PATH ?= ./picture/
CMP_PATH ?= ../command

all: data fig pdf

data: data_latency.csv

fig: data_serverLatency.svg data_playerLatency.svg data_RGB_YUV_buffer.svg

pdf:svg2pic

# txt2csv
data_latency.csv: txt2csv_lantency.py $(shell find $(LOG_PATH))
	python3 $< $(LOG_PATH) $(DATA_PATH)$@

# csv2svg
data_serverLatency.svg: csv2svg_serverLatency.py $(shell find $(LOG_PATH))
	python $< $(DATA_PATH)data_latency.csv $(DATA_PATH)

data_playerLatency.svg: csv2svg_playerLatency.py $(shell find $(LOG_PATH))
	python $< $(DATA_PATH)data_latency.csv $(DATA_PATH)

data_RGB_YUV_buffer.svg: csv2svg_RGB_YUV_buffer.py $(shell find $(LOG_PATH))
	python $< $(DATA_PATH)data_latency.csv $(DATA_PATH)

# svg2pic
svg2pic: tool_svg2pic.py
	python3 $< $(DATA_PATH) $(PIC_PATH)