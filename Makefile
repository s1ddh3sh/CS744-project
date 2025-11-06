all: server loadgen

server: src/server.cpp src/cache/kv-store.c db/db.c src/cache/kv-store.h db/db.h
	g++ -I/usr/include/postgresql -Wall -o server src/server.cpp src/cache/kv-store.c db/db.c -lpq -pthread

loadgen: src/loadgen.cpp
	g++ -I/usr/include/postgresql -Wall -o loadgen src/loadgen.cpp -lpq -pthread

clean:
	rm -f server loadgen