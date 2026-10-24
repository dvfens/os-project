#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum { MESSAGE_CAPACITY = 256 };

static int write_all(int fd, const char *data, size_t length) {
    size_t written = 0;

    while (written < length) {
        ssize_t result = write(fd, data + written, length - written);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        written += (size_t)result;
    }
    return 0;
}

static ssize_t read_message(int fd, char *buffer, size_t capacity) {
    size_t used = 0;

    while (used + 1 < capacity) {
        ssize_t result = read(fd, buffer + used, capacity - used - 1);
        if (result == 0) {
            break;
        }
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        used += (size_t)result;
    }

    buffer[used] = '\0';
    return (ssize_t)used;
}

static void close_if_open(int *fd) {
    if (*fd >= 0) {
        (void)close(*fd);
        *fd = -1;
    }
}

int main(void) {
    int parent_to_child[2] = { -1, -1 };
    int child_to_parent[2] = { -1, -1 };

    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
        perror("pipe");
        close_if_open(&parent_to_child[0]);
        close_if_open(&parent_to_child[1]);
        close_if_open(&child_to_parent[0]);
        close_if_open(&child_to_parent[1]);
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close_if_open(&parent_to_child[0]);
        close_if_open(&parent_to_child[1]);
        close_if_open(&child_to_parent[0]);
        close_if_open(&child_to_parent[1]);
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        char message[MESSAGE_CAPACITY];
        const char reply[] = "Hello parent, I received your message.";

        close_if_open(&parent_to_child[1]);
        close_if_open(&child_to_parent[0]);

        ssize_t length = read_message(parent_to_child[0], message, sizeof(message));
        close_if_open(&parent_to_child[0]);
        if (length < 0) {
            perror("child read");
            _exit(EXIT_FAILURE);
        }

        printf("Child received: %s\n", message);
        if (write_all(child_to_parent[1], reply, sizeof(reply) - 1) == -1) {
            perror("child write");
            close_if_open(&child_to_parent[1]);
            _exit(EXIT_FAILURE);
        }
        close_if_open(&child_to_parent[1]);
        _exit(EXIT_SUCCESS);
    }

    close_if_open(&parent_to_child[0]);
    close_if_open(&child_to_parent[1]);

    const char message[] = "Hello child, this message traveled through a pipe.";
    if (write_all(parent_to_child[1], message, sizeof(message) - 1) == -1) {
        perror("parent write");
        close_if_open(&parent_to_child[1]);
        close_if_open(&child_to_parent[0]);
        (void)waitpid(pid, NULL, 0);
        return EXIT_FAILURE;
    }
    close_if_open(&parent_to_child[1]);

    char reply[MESSAGE_CAPACITY];
    ssize_t reply_length = read_message(child_to_parent[0], reply, sizeof(reply));
    close_if_open(&child_to_parent[0]);
    if (reply_length < 0) {
        perror("parent read");
        (void)waitpid(pid, NULL, 0);
        return EXIT_FAILURE;
    }
    printf("Parent received: %s\n", reply);

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
        fprintf(stderr, "Child process failed.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
