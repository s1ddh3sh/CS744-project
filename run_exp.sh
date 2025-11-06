#!/bin/bash

if [ $# -ne 3 ]; then
    echo "Usage: $0 <threads> <duration> <workload>"
    echo "Example: $0 16 60 get_popular"
    exit 1
fi

WORKLOAD=$3
DURATION=$2
THREADS=$1

RESULTS_DIR="results"
echo "Building executables (if needed):"
make
echo "Starting server on core: 1"
taskset -c 1 ./server &
SERVER_PID=$!
echo "Server started with PID $SERVER_PID"


sleep 5

echo "--- Running $WORKLOAD load with $THREADS threads on core: 2 ---"

taskset -c 2 ./loadgen $THREADS $DURATION $WORKLOAD > $RESULTS_DIR/${WORKLOAD}/${THREADS}_threads.txt

echo "Results saved to $RESULTS_DIR/${THREADS}_threads.txt"
sleep 5


kill $SERVER_PID
echo "Server stopped."
