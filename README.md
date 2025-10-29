# CS744-project

Simple key-value server with an in-memory cache and PostgreSQL database, with a multi-threaded closed‑loop load generator.

## Components
- server.cpp — HTTP server (httplib) exposing /create, /get, /delete, /preload, /clear
- kv-store.c / kv-store.h — in-memory key-value cache (thread-safe)
- db.c / db.h — PostgreSQL access (libpq)
- loadgen.cpp — multi-threaded closed-loop load generator for workload experiments

## Build
Requirements:
- g++ (C++17)
- gcc
- libpq (PostgreSQL client)
- pthreads
- httplib (single-header library)

## Run
1. Start PostgreSQL and ensure connection settings in `db.c` are correct.
2. Bind postgres pid to core 0
```bash
pgrep postgres #get the lowest pid
sudo taskset -cp 0 {postgres-pid}
```
2. Build the executables
```bash
make
```
2. Start server:
```bash
taskset -c 0 ./server #server runs on core 0
```
Server runs on http://localhost:8080.

3. Run load generator in other terminal:
```bash
# ./loadgen <threads> <duration_sec> <workload>
taskset -c 2 ./loadgen 10 60 get_popular #load-gen runs on core 2
```

Workloads: `put_all`, `get_all`, `get_popular`, `mixed` (see loadgen usage/help in code).
