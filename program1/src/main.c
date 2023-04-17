#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Wrong number of arguments.\n");
        return EBADF;
    }
    const char *FILE_NAME = argv[1];
    const char *FIFO_NAME = argv[2];

    if (access(FIFO_NAME, F_OK) == -1) {
        long res = mkfifo(FIFO_NAME, 0777);
        if (res != 0) {
            fprintf(stderr, "Couldn't create fifo%s\n", FIFO_NAME);
            exit(EXIT_FAILURE);
        }
    }

    FILE *file_fd = fopen(FILE_NAME, "rb");
    int pipe_fd = open(FIFO_NAME, O_WRONLY| O_NONBLOCK, 0777);
    //char buf[1];
    int buf = 0;
    if (pipe_fd != -1 && file_fd != NULL) {
        // Пока не дошли до конца файла, выписываем все в FIFO
        while (feof(file_fd) == 0) {
            unsigned long res = fread(&buf, sizeof buf, 1, file_fd);
            if(res <= 0){
                break;
            }
            write(pipe_fd, &buf, sizeof buf);
        }
        (void)fclose(file_fd);
        (void)close(pipe_fd);
    } else {
        //Закрываем и если ошибка возникла только в одном из потоков
        if(file_fd != NULL)
            (void)fclose(file_fd);
        if(pipe_fd != -1)
            (void)close(pipe_fd);
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

