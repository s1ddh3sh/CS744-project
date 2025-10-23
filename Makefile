all: server loadgen

server: server.cpp kv-store.c db.c kv-store.h db.h
	g++ -I/usr/include/postgresql -Wall -o server server.cpp kv-store.c db.c -lpq -pthread

loadgen: loadgen.cpp
	g++ -I/usr/include/postgresql -Wall -o loadgen loadgen.cpp -lpq -pthread

clean:
	rm -f server loadgen