# Pipe based interprocess communication in C

This project demonstrates communication between a parent process and its child using POSIX unnamed pipes.

## How it works

- `pipe()` creates a pair of file descriptors. Each pipe carries bytes in one direction.
- `fork()` creates a child process. The child inherits both pipes' file descriptors.
- The parent closes the pipe ends it does not use, sends a message to the child, then closes that write end. Closing it signals end of input (EOF).
- The child reads the message and sends a reply through a second pipe.
- The parent reads the reply and calls `waitpid()` to collect the child process's exit status.

Two pipes are used because an ordinary pipe is one-way. The program also handles interrupted reads and writes and closes unused descriptors so that EOF is delivered correctly.

## Build and run

Requires a POSIX environment with a C compiler, such as Linux, macOS, or WSL. Windows PowerShell alone does not provide `fork()` or POSIX `pipe()`.

```sh
make
./pipe_ipc
```

Or compile directly:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -O2 main.c -o pipe_ipc
./pipe_ipc
```

Expected output (the order of the two lines can vary because the processes run concurrently):

```text
Child received: Hello child, this message traveled through a pipe.
Parent received: Hello parent, I received your message.
```
