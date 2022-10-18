#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int p[2];
    char buf[2];
    pipe(p);
    int pid = fork();
    if (pid == 0) { // child
        if (read(p[0], buf, 1) != 1) {
            fprintf(2, "Can't read from parent!\n");
            exit(1);
        }
        close(p[0]);

        fprintf(1, "child receive: %c\n", buf[0]);
        fprintf(1, "%d: received ping\n", getpid());

        if (write(p[1], "b", 1) != 1) {
            fprintf(2, "Can't write to parent!\n");
            exit(1);
        }
        close(p[1]);

        exit(0);
    }
    else {  // parent
        if (write(p[1], "a", 1) != 1) {
            fprintf(2, "Can't write to child!\n");
            exit(1);
        }
        close(p[1]);  

        wait(0); // 如果不等待子进程结束，可能读的是"a"

        if (read(p[0], buf, 1) != 1) {
            fprintf(2, "Can't read from child!\n");
            exit(1);
        }
        close(p[0]);

        fprintf(1, "parent receive: %c\n", buf[0]);
        fprintf(1, "%d: received pong\n", getpid());
        
        exit(0);
    }
}