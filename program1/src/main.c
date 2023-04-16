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

    FILE *file_fd = fopen(FILE_NAME, "rb");
    int pipe_fd = open(FIFO_NAME, O_WRONLY| O_NONBLOCK, 0777);
    char buf[1];
    if (pipe_fd != -1 && file_fd != NULL) {
        // Пока не дошли до конца файла, выписываем все в FIFO
        while (feof(file_fd) == 0) {
            fread(buf, 1, 1, file_fd);
            long res = write(pipe_fd, buf, sizeof buf);
            if(res <= 0){
                break;
            }
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

void create_fifo_ifnotexist(const char *FIFO_NAME){
    if (access(FIFO_NAME, F_OK) == -1) {
        long res = mkfifo(FIFO_NAME, 0777);
        if (res != 0) {
            fprintf(stderr, "Couldn't create fifo%s\n", FIFO_NAME);
            exit(EXIT_FAILURE);
        }
    }
}