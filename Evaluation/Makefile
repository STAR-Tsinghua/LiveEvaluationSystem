

LOG_PATH ?= ../LiveSystem/logs
DATA_PATH ?= ./data/
PIC_PATH ?= ./picture/
CMP_PATH ?= ../command

all: data fig svg2pic

data: data_latency.csv

fig: data_latency.svg

data_latency.csv: txt2csv_lantency.py $(shell find $(LOG_PATH))
	python3 $< $(LOG_PATH) $(DATA_PATH)$@

data_latency.svg: csv2svg_latency.py $(shell find $(LOG_PATH))
	python $< $(DATA_PATH)data_latency.csv $(DATA_PATH)

svg2pic: svg2pic_latency.py
	python3 $< $(DATA_PATH) $(PIC_PATH)