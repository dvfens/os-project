# Pipe IPC architecture

The diagram is constant: changing the message typed by the user does not change which processes or pipes are connected.

```text
                 parent_to_child pipe
          write end ───────────────────► read end
       ┌──────────────┐               ┌──────────────┐
       │ Parent       │               │ Child        │
       │              │               │              │
       │              │               │              │
       └──────────────┘               └──────────────┘
          read end ◄─────────────────── write end
                  child_to_parent pipe
```

The parent creates both pipes before calling `fork()`. The child inherits their file descriptors, then each process closes the ends it does not use:

| Pipe | Writer | Reader | Purpose |
| --- | --- | --- | --- |
| `parent_to_child` | Parent | Child | Carries the parent's message to the child. |
| `child_to_parent` | Child | Parent | Carries the child's reply back to the parent. |

Each pipe is unidirectional. Two pipes provide communication in both directions. Closing a pipe's write end after sending data lets the reader observe end of file (EOF) after all buffered data has been read.

## Message flow

1. The parent writes a message to `parent_to_child` and closes its write end.
2. The child reads the message until EOF.
3. The child writes a reply to `child_to_parent` and closes its write end.
4. The parent reads the reply and waits for the child to exit.

The message contents can vary; the process and pipe topology remains the same.
