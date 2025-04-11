# Makefile for IPC Job Dispatcher

CC = g++
# CFLAGS = -Wall -Wextra -O2 -std=c++11
CFLAGS = -std=c++11
SRC = Main.cpp
OUT = dispatcher

all: build

build:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) -lpthread

run: build
	./$(OUT) < input.txt

clean:
	rm -f $(OUT)
