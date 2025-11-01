#!/bin/bash


WORKLOAD="get_popular"
DURATION=60
THREADS=(8)

RESULTS_DIR="results"

echo "Starting server on core: 1"
taskset -c 1 ./server &
SERVER_PID=$!
echo "Server started with PID $SERVER_PID"


sleep 5


for THREADS in "${THREADS[@]}"; do
    echo "--- Running $WORKLOAD with $THREADS threads on core: 2 ---"

    taskset -c 2 ./loadgen $THREADS $DURATION $WORKLOAD > $RESULTS_DIR/${WORKLOAD}/${THREADS}threads.txt
    
    echo "Results saved to $RESULTS_DIR/${THREADS}_threads.txt"
    sleep 10

done

kill $SERVER_PID
echo "Server stopped."
