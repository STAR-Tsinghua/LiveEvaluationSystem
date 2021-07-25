LOG_PATH_DTP ?= ../LiveSystem/logs/ev_dtp
LOG_PATH_UDP ?= ../LiveSystem/logs/ev_udp
DATA_PATH ?= ./data/
PIC_PATH ?= ./picture/
CMP_PATH ?= ../command
SAVE_PATH ?= ./save/
SAVE_DTP_PDF_PATH ?= ./save/dtp/pdf/
SAVE_DTP_DATA_PATH ?= ./save/dtp/data/
SAVE_UDP_PDF_PATH ?= ./save/udp/pdf/
SAVE_UDP_DATA_PATH ?= ./save/udp/data/

all: all_dtp

all_dtp: data_dtp fig_dtp pdf

all_udp: data_udp fig_udp pdf

# save needed resaults
save_dtp: tool_save.py
	python3 $< $(PIC_PATH) $(DATA_PATH) $(SAVE_DTP_PDF_PATH) $(SAVE_DTP_DATA_PATH)

save_udp: tool_save.py
	python3 $< $(PIC_PATH) $(DATA_PATH) $(SAVE_UDP_PDF_PATH) $(SAVE_UDP_DATA_PATH)

data_dtp: data_latency_dtp.csv

data_udp: data_latency_udp.csv

fig_dtp: data_serverLatency.svg data_playerLatency.svg data_frames_jitter data_buffer_container.svg data_frame_latency.svg data_frames_count.svg data_frames_state.svg

fig_udp: data_serverLatency_udp.svg data_playerLatency_udp.svg data_frames_jitter_udp data_buffer_container_udp.svg data_frame_latency_udp.svg data_frames_count_udp.svg data_frames_state_udp.svg

pdf:svg2pic

# txt2csv
data_latency_dtp.csv: txt2csv_lantency.py $(shell find $(LOG_PATH))
	python3 $< $(LOG_PATH_DTP) $(DATA_PATH)$@

data_latency_udp.csv: txt2csv_lantency.py $(shell find $(LOG_PATH))
	python3 $< $(LOG_PATH_UDP) $(DATA_PATH)$@

# svg2pic
svg2pic: tool_svg2pic.py
	python3 $< $(DATA_PATH) $(PIC_PATH)

# csv2svg # 生成多个svg    ============== dtp ==============
data_frames_state.svg: csv2svg_frames_state.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)clean_frame_data.csv $(DATA_PATH)

data_frames_count.svg: csv2svg_frames_count.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)clean_frame_data.csv $(DATA_PATH)

data_serverLatency.svg: csv2svg_serverLatency.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)data_latency_dtp.csv $(DATA_PATH)

data_playerLatency.svg: csv2svg_playerLatency.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)data_latency_dtp.csv $(DATA_PATH)

data_frames_jitter: csv2svg_frames_jitter.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)data_latency_dtp.csv $(DATA_PATH)

data_buffer_container.svg: csv2svg_buffer_container.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)data_latency_dtp.csv $(DATA_PATH)

data_frame_latency.svg: csv2svg_frame_latency.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)data_latency_dtp.csv $(DATA_PATH)

# csv2svg # 生成多个svg    ============== udp ==============
data_frames_state_udp.svg: csv2svg_frames_state.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)clean_frame_data.csv $(DATA_PATH)

data_frames_count_udp.svg: csv2svg_frames_count.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)clean_frame_data.csv $(DATA_PATH)

data_serverLatency_udp.svg: csv2svg_serverLatency.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)data_latency_udp.csv $(DATA_PATH)

data_playerLatency_udp.svg: csv2svg_playerLatency.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)data_latency_udp.csv $(DATA_PATH)

data_frames_jitter_udp: csv2svg_frames_jitter.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)data_latency_udp.csv $(DATA_PATH)

data_buffer_container_udp.svg: csv2svg_buffer_container.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)data_latency_udp.csv $(DATA_PATH)

data_frame_latency_udp.svg: csv2svg_frame_latency.py $(shell find $(LOG_PATH))
	python3 $< $(DATA_PATH)data_latency_udp.csv $(DATA_PATH)
