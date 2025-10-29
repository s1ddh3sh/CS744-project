#!/bin/bash
DB_CORE=0
SERVER_CORE=1
LOADGEN_CORES=2

echo "Starting server..."
taskset -c $SERVER_CORE ./server &

sleep 1
echo "Starting load generator..."
taskset -c $LOADGEN_CORES ./loadgen 10 60 get_all
