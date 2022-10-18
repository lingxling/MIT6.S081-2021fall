#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXLEN 1024

void copy(char **p1, char *p2) {
    *p1 = malloc(strlen(p2) + 1);
    strcpy(*p1, p2);
}

int readLine(char **pars, int i) {
    char buf[MAXLEN];
    int j = 0;
    while (read(0, buf + j, 1)) {  // 每次读取一个字符，直到遇到\n，即读取一条命令
        if (buf[j] == '\n') {
            buf[j] = 0;
            break;
        }
        ++j;
        if (j == MAXLEN) {
            fprintf(2, "Parameters are too long!\n");
            exit(1);
        }
    }

    if (j == 0) {  // 没有读取到内容，即已经到pars的结尾
        return -1;
    }
    int k = 0;
    while (k < j) {  // 一条命令可能有多个参数，每个参数保存在pars[i]中
        if (i > MAXARG) {
            fprintf(2, "Too much parameters!\n");
            exit(1);
        }
        while ((k < j) && (buf[k] == ' ')) {  // 去掉命令的前导空格
            ++k;
        }
        
        int l = k;
        while ((k < j) && (buf[k] != ' ')) {
            ++k;
        }
        
        buf[k++] = 0;
        copy(&pars[i], buf + l);
        ++i;
    }
    return i;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Please enter more parameters!\n");
        exit(1);
    }

    char *pars[MAXARG];
    for (int i = 1; i < argc; ++i) {
        copy(&pars[i-1], argv[i]);
    }
    int e;
    while ((e = readLine(pars, argc - 1)) != -1) {
        pars[e] = 0;
        if (fork() == 0) {  // 子进程执行命令
            exec(pars[0], pars);
            exit(0);
        }
        else {
            wait(0);  // 父进程等待子进程完成
        }
    }
    exit(0);
}