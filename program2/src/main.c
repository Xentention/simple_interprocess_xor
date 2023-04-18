//
// Created by xenia on 16.04.23.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define FIFO_NAME1 "/tmp/my_fifo1"
#define FIFO_NAME2 "/tmp/my_fifo2"

int create_FIFO(const char *FIFO_file);

int start_program1(char *filename, char *fifo_file);

void close_everything(int pipe_fd1, int pipe_fd2, FILE *pipe_fd_out);

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Wrong number of arguments.\n");
        exit(EXIT_FAILURE);
    }

    if (create_FIFO(FIFO_NAME1) != 0) {
        exit(EXIT_FAILURE);
    }

    if (create_FIFO(FIFO_NAME2) != 0) {
        exit(EXIT_FAILURE);
    }

    //Открываем все нужные потоки ввода в неблокирующем режиме
    int pipe_fd1 = open(FIFO_NAME1, O_RDONLY | O_NONBLOCK, 0777);
    int pipe_fd2 = open(FIFO_NAME2, O_RDONLY | O_NONBLOCK, 0777);
    FILE *pipe_fd_out = fopen(argv[3], "wb");

    //Запускаем и проверяем успешное выполнение программ 1
    if (start_program1(argv[1], FIFO_NAME1) != 0
        || start_program1(argv[2], FIFO_NAME2) != 0) {
        exit(EXIT_FAILURE);
    }

    long res1, res2;
    if (pipe_fd1 != -1 && pipe_fd2 != -1 && pipe_fd_out != NULL) {
        do {
            // Считываем по байту из каждого пайпа и выполняем XOR
            int temp1 = 0, temp2 = 0, temp = 0;
            res1 = read(pipe_fd1, &temp1, 1);
            res2 = read(pipe_fd2, &temp2, 1);
            if ((res1 > 0) && (res2 > 0)) {
                temp = temp1 ^ temp2;
                fwrite(&temp, 1, 1, pipe_fd_out);
            }
        } while ((res1 > 0) || (res2 > 0));
        close_everything(pipe_fd1, pipe_fd1, pipe_fd_out);
    } else {
        close_everything(pipe_fd1, pipe_fd1, pipe_fd_out);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

/**
 * При необходимости создает FIFO файл
 * @param FIFO_file путь до FIFO
 * @return 0 при удаче, -1 при ошибке
 */
int create_FIFO(const char *FIFO_file) {
    if (access(FIFO_file, F_OK) == -1) {
        long res = mkfifo(FIFO_file, 0777);
        if (res != 0) {
            fprintf(stderr, "Couldn't create fifo%s\n", FIFO_file);
            return -1;
        }
    }
    return 0;
}

/**
 * Запускает экземпляр program1 и дожидается ее выполнения
 * @param filename путь к файлу откуда program1 считает данные
 * @param fifo_file путь к FIFO файлу куда program1 запишет данные для IPC
 * @return 0 при успехе, -1 при ошибке
 */
int start_program1(char *filename, char *fifo_file) {

    pid_t child_pid = fork();
    char *args[] = {"start_program1", filename, fifo_file, NULL};
    switch (child_pid) {
        case -1:
            perror("Error: fork failed\n");
            return -1;
        case 0:
            if (execvp("./start_program1", args) == -1)
                perror("Error: command execution failed\n");
            exit(EXIT_FAILURE);
        default:
            while (waitpid(child_pid, (int *) 0, WNOHANG) != child_pid) {
                //ждем завершения процесса потомка
            }
    }
    return 0;
}

void close_everything(int pipe_fd1, int pipe_fd2, FILE *pipe_fd_out) {
    //Закрываем и если ошибка возникла только в одном из потоков
    if (pipe_fd1 != -1)
        (void) close(pipe_fd1);
    if (pipe_fd2 != -1)
        (void) close(pipe_fd2);
    if (pipe_fd_out != NULL)
        (void) fclose(pipe_fd_out);
    unlink(FIFO_NAME1);
    unlink(FIFO_NAME2);
}