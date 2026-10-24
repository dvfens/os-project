CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

.PHONY: all run clean

all: pipe_ipc

pipe_ipc: main.c
	$(CC) $(CFLAGS) -o $@ $<

run: pipe_ipc
	./pipe_ipc

clean:
	rm -f pipe_ipc
