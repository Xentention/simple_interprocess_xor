//
// Created by xenia on 16.04.23.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define FIFO_NAME1 "/tmp/my_fifo1"
#define FIFO_NAME2 "/tmp/my_fifo2"

int start_program1(char* filename, char* fifo_file);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Wrong number of arguments.\n");
        exit(EXIT_FAILURE);
    }

    //Открываем все нужные потоки ввода в неблокирующем режиме
    int pipe_fd1 = open(FIFO_NAME1, O_RDONLY | O_NONBLOCK, 0777);
    int pipe_fd2 = open(FIFO_NAME2, O_RDONLY | O_NONBLOCK, 0777);
    FILE *pipe_fd_out = fopen(argv[3], "wb");

    //Запускаем и проверяем успешное выполнение программ 1
    if(start_program1(argv[1], FIFO_NAME1) != 0
       || start_program1(argv[2], FIFO_NAME2) != 0){
        exit(EXIT_FAILURE);
    }

    long res1, res2;
    if(pipe_fd1 != -1 && pipe_fd2 != -1 && pipe_fd_out != NULL){
        do {
            int temp1 = 0, temp2 = 0, temp = 0;
            res1 = read(pipe_fd1, &temp1, 1);
            res2 = read(pipe_fd2, &temp2, 1);
            if ((res1 > 0) && (res2 > 0)) {
            temp = temp1 ^ temp2;
            fwrite(&temp, 1, 1, pipe_fd_out);
            }
        } while ((res1 > 0) || (res2 > 0));
        (void) close(pipe_fd1);
        (void) close(pipe_fd2);
        (void) fclose(pipe_fd_out);
    }
    else {
        if(pipe_fd1 != -1)
            (void) close(pipe_fd1);
        if(pipe_fd2 != -1)
            (void) close(pipe_fd2);
        if(pipe_fd_out != NULL)
            (void) fclose(pipe_fd_out);
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}


int start_program1(char* filename, char* fifo_file){
    pid_t child_pid = fork();
    char* args[] = {"start_program1",filename, fifo_file, NULL};
    switch (child_pid) {
        case -1:
            perror("Error: fork failed\n");
            break;
        case 0:
            if(execvp("./start_program1", args) == -1)
                perror("Error: command execution failed\n");
            exit(EXIT_FAILURE);
        default:
            while (waitpid(child_pid, (int *) 0, WNOHANG) != child_pid) {
                //ждем завершения процесса потомка
            }
    }
    return 0;
}