#!/bin/bash
./trace_begin.sh
cd ~/Documents/DTP/examples
echo 'client begin'
# QUIC
# {360; 480; 720; 1080; 1440}p
./client_trace 192.168.10.2 5555 config/dash_chunk_size_QUIC/360p.txt 1.0 2 &> result/client_log/QUIC-client-360p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size_QUIC/480p.txt 1.0 2 &> result/client_log/QUIC-client-480p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size_QUIC/720p.txt 1.0 2 &> result/client_log/QUIC-client-720p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size_QUIC/1080p.txt 1.0 2 &> result/client_log/QUIC-client-1080p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size_QUIC/1440p.txt 1.0 2 &> result/client_log/QUIC-client-1440p.log

# Deadline
# {360; 480; 720; 1080; 1440}p
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/360p.txt 0.0 2 &> result/client_log/Deadline-client-360p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/480p.txt 0.0 2 &> result/client_log/Deadline-client-480p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/720p.txt 0.0 2 &> result/client_log/Deadline-client-720p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/1080p.txt 0.0 2 &> result/client_log/Deadline-client-1080p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/1440p.txt 0.0 2 &> result/client_log/Deadline-client-1440p.log

# Priority
# {360; 480; 720; 1080; 1440}p
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/360p.txt 1.0 2 &> result/client_log/Priority-client-360p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/480p.txt 1.0 2 &> result/client_log/Priority-client-480p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/720p.txt 1.0 2 &> result/client_log/Priority-client-720p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/1080p.txt 1.0 2 &> result/client_log/Priority-client-1080p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/1440p.txt 1.0 2 &> result/client_log/Priority-client-1440p.log

# DTP
# {360; 480; 720; 1080; 1440}p
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/360p.txt 0.5 2 &> result/client_log/DTP-client-360p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/480p.txt 0.5 2 &> result/client_log/DTP-client-480p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/720p.txt 0.5 2 &> result/client_log/DTP-client-720p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/1080p.txt 0.5 2 &> result/client_log/DTP-client-1080p.log
# ./client_trace 192.168.10.2 5555 config/dash_chunk_size/1440p.txt 0.5 2 &> result/client_log/DTP-client-1440p.log
cd -
./trace_end.sh