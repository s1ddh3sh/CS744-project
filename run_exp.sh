#!/bin/bash

if [ $# -ne 3 ]; then
    echo "Usage: $0 <threads> <duration> <workload>"
    echo "Example: $0 16 60 get_popular"
    exit 1
fi

WORKLOAD=$3
DURATION=$2
THREADS=$1

RESULTS_DIR="results/${WORKLOAD}/${THREADS}"
MONITOR_DIR="results/${WORKLOAD}/${THREADS}"

mkdir -p "$RESULTS_DIR"
mkdir -p "$MONITOR_DIR"


echo "Building executables (if needed):"
make


echo "Starting server on core: 1"
taskset -c 1 ./server &
SERVER_PID=$!
echo "Server started with PID $SERVER_PID"

sleep 5

echo "Starting system monitoring"

MPSTAT_LOG="${MONITOR_DIR}/mpstat_core1.txt"
mpstat -P 1 1 $((DURATION+ 10)) > "$MPSTAT_LOG" &
MPSTAT_PID=$!

IOSTAT_LOG="${MONITOR_DIR}/iostat.txt"
iostat -dx nvme0n1 1 $((DURATION + 10)) > "$IOSTAT_LOG" &
IOSTAT_PID=$!

echo "Monitoring started (mpstat PID=$MPSTAT_PID, iostat PID=$IOSTAT_PID)"
sleep 2

#Run the loadgen

echo "--- Running $WORKLOAD load with $THREADS threads on core: 2 ---"

taskset -c 2 ./loadgen $THREADS $DURATION $WORKLOAD > "$RESULTS_DIR/${THREADS}_threads.txt"


sleep 5


kill $SERVER_PID 2>/dev/null
kill $MPSTAT_PID 2>/dev/null
kill $IOSTAT_PID 2>/dev/null


echo "Server and monitoring stopped."
echo "CPU stats saved to $MPSTAT_LOG"
echo "Disk stats saved to $IOSTAT_LOG"
echo "All results are in $RESULTS_DIR"
