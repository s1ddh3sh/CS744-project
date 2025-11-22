# HTTP-based KV server with in-mem Cache

Simple key-value server with an in-memory cache and PostgreSQL database, with a multi-threaded closed‑loop load generator.

## Components
- server.cpp - HTTP server (httplib) exposing /create, /get, /delete, /preload, /clear
- kv-store.c - in-memory key-value cache (thread-safe)
- db.c - PostgreSQL access (libpq)
- loadgen.cpp - multi-threaded closed-loop load generator for workload experiments

## Workloads: 
- `put_all` : 50% PUT + 50% DELETE requests
- `get_popular` : random GET requests for popular 1000 keys

`pre-population` of keys is used to avoid any DBMISS.

## Run

Start PostgreSQL and ensure connection settings in `db.c` are correct.

### Option 1 : Quick run via bash script
Just run the `run_exp.sh` script to run <br> Note : This is a complete testing script which runs for all load levels and on all the workloads.
```bash
bash run_exp.sh
```
This script handles all the setup and execution steps. <br>
Outputs are stored in: `results/{WORKLOAD}/{THREADS}_threads.txt` 
### Option 2 : Run Manually


1. Build the executables (./server, ./loadgen)
```bash
make
```
2. Start server:
```bash
taskset -c 1 ./server #server runs on core 1
```
Server runs on http://localhost:8080.

3. Run load generator in other terminal: <br>
  `./loadgen [threads] [duration_sec] [workload]`
```bash
taskset -c 2-7 ./loadgen 10 60 get_popular #loadgen runs on cores 2-7
```

