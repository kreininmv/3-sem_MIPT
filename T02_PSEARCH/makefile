CFLAGS=-c -std=c++1z -O3 # Compilation flags
LFLAGS=-fsanitize=address -g -lstdc++fs -lpthread # Linker flags
CC=g++ # Compiler

HEADERS=psearch.h # Headers

all: psearch

psearch: T02_PSEARCH.o psearch.o
	$(CC) T02_PSEARCH.o psearch.o $(LFLAGS) -o psearch

main.o: T02_PSEARCH.cpp $(HEADERS)
	$(CC) $(CFLAGS) T02_PSEARCH.cpp

kmp_search.o: psearch.cpp $(HEADERS)
	$(CC) $(CFLAGS) psearch.cpp

clean:
	rm -rf *.o psearch