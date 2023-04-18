#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

void close_everything(int pipe_fd, FILE *file_fd, const char *FIFO_NAME);


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Wrong number of arguments.\n");
        return EBADF;
    }
    const char *FILE_NAME = argv[1];
    const char *FIFO_NAME = argv[2];

    if (access(FIFO_NAME, F_OK) == -1) {
        fprintf(stderr, "Couldn't access fifo%s\n", FIFO_NAME);
        exit(EXIT_FAILURE);
    }

    FILE *file_fd = fopen(FILE_NAME, "rb");
    int pipe_fd = open(FIFO_NAME, O_WRONLY | O_NONBLOCK, 0777);
    int buf = 0;
    if (pipe_fd != -1 && file_fd != NULL) {
        // Пока не дошли до конца файла, выписываем все в FIFO
        while (feof(file_fd) == 0) {
            unsigned long res = fread(&buf, 1, 1, file_fd);
            if (res <= 0) {
                break;
            }
            write(pipe_fd, &buf, 1);
        }
    } else {
        close_everything(pipe_fd, file_fd, FIFO_NAME);
        exit(EXIT_FAILURE);
    }
    close_everything(pipe_fd, file_fd, FIFO_NAME);
    exit(EXIT_SUCCESS);
}

void close_everything(int pipe_fd, FILE *file_fd, const char *FIFO_NAME) {
    //Закрываем и если ошибка возникла только в одном из потоков
    if (file_fd != NULL)
        (void) fclose(file_fd);
    if (pipe_fd != -1)
        (void) close(pipe_fd);
    unlink(FIFO_NAME);
}

