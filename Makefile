CFLAGS=-g -std=gnu11 -W -Wall -O3
CC=gcc

SOURCES= \
	hash/*.c test_hash/*.c \
	hash_hashtable/private/*.c \
	stopwatch/*.c measurement/*.c \
	hash_array/*.c hash_hashtable/*.c \
	observation/*.c test_observation/*.c \
	rand/*.c test_rand/*.c \
	util/*.c \
	performance/*.c

all: bin/main bin/hash_test

bin/main: main.c $(SOURCES)
	$(CC) $(CFLAGS) main.c $(SOURCES) -o $@

bin/hash_test: hash_test.c $(SOURCES)
	$(CC) $(CFLAGS) hash_test.c $(SOURCES) -o $@
