# Pipe based interprocess communication in C

This project demonstrates two way communication between a parent process and its child process using POSIX unnamed pipes. It is a user space demonstration built on the operating system's `pipe()`, `fork()`, `read()`, and `write()` system calls.

## Process flow

The parent creates both pipes before `fork()`. After the fork, the parent and child close the pipe ends they do not use. The arrows below show the order of the message exchange and the point where closing a writer lets the reader receive EOF.

```mermaid
flowchart TD
    A[Parent creates two pipes] --> B[fork creates child]
    B -->|Parent process| P1[Close parent read end and child reply write end]
    B -->|Child process| C1[Close parent write end and child reply read end]
    P1 --> P2[Write message to parent to child pipe]
    P2 --> P3[Close parent to child write end]
    P3 --> C2[Child reads message until EOF]
    C1 --> C2
    C2 --> C3[Child writes reply to child to parent pipe]
    C3 --> C4[Close child to parent write end]
    C4 --> P4[Parent reads reply until EOF]
    P3 --> P4
    P4 --> P5[Parent closes reply read end]
    P5 --> P6[waitpid collects child exit status]
```

## Build and run

Requires a C compiler and a POSIX environment such as Linux, macOS, or WSL. Windows PowerShell alone does not provide `fork()` or POSIX unnamed pipes.

```sh
make
./pipe_ipc
```

Or compile directly:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -O2 main.c -o pipe_ipc
./pipe_ipc
```

Expected output (the order can vary because parent and child run concurrently):

```text
Child received: Hello child, this message traveled through a pipe.
Parent received: Hello parent, I received your message.
```

See [docs/architecture.md](docs/architecture.md) for the descriptor ownership table, detailed data flow, and pipe behavior.
