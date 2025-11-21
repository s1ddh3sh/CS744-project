#!/bin/bash


WORKLOADS=("get_popular" "put_all")
DURATION=300
THREAD_LIST=(1 2 4 5 6 8 10 12 14 16)

echo "Building executables (if needed):"
make


for THREADS in "${THREAD_LIST[@]}"; do
    echo "============================================================"
    echo "       Running experiments for THREADS = $THREADS"
    echo "============================================================"


    echo "Starting server on core: 0-1"
    taskset -c 0-1 ./server &
    SERVER_PID=$!
    echo "Server started with PID $SERVER_PID"

    sleep 5

    for WORKLOAD in "${WORKLOADS[@]}"; do
        RESULTS_DIR="results/${WORKLOAD}/${THREADS}"
        MONITOR_DIR="results/${WORKLOAD}/${THREADS}"

        mkdir -p "$RESULTS_DIR"
        mkdir -p "$MONITOR_DIR"

        echo "------------------------------------------------------------"
        
        echo "Running workload: $WORKLOAD (threads=$THREADS, duration=${DURATION}s)"

        echo "Starting system monitoring"

        MPSTAT_LOG="${MONITOR_DIR}/mpstat_core1.txt"
        mpstat -P 1 1 $((DURATION+ 5)) > "$MPSTAT_LOG" &
        MPSTAT_PID=$!

        IOSTAT_LOG="${MONITOR_DIR}/iostat.txt"
        iostat -dx nvme0n1p5 1 $((DURATION + 5)) > "$IOSTAT_LOG" &
        IOSTAT_PID=$!

        echo "Monitoring started (mpstat PID=$MPSTAT_PID, iostat PID=$IOSTAT_PID)"
        sleep 2

        #Run the loadgen

        echo "--- Running $WORKLOAD load with $THREADS threads on core: 2-7 ---"

        taskset -c 2-7 ./loadgen $THREADS $DURATION $WORKLOAD > "$RESULTS_DIR/${THREADS}_threads.txt"


        sleep 5

        echo "Stopping Monitoring for workload: $WORKLOAD"

        
        kill $MPSTAT_PID 2>/dev/null
        kill $IOSTAT_PID 2>/dev/null

        sleep 2

        echo "Monitoring stopped for workload: $WORKLOAD."
        echo "CPU stats saved to $MPSTAT_LOG"
        echo "Disk stats saved to $IOSTAT_LOG"
        echo "Results are in $RESULTS_DIR"
        echo "------------------------------------------------------------"
        
        sleep 2
    done

    echo "Stopping Server with PID: $SERVER_PID"

    kill $SERVER_PID 2>/dev/null
    sleep 2

    echo "All workloads completed for THREADS = $THREADS."
    echo ""
done
echo "All workloads completed. Results in the /results."