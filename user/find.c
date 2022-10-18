#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)  // 获取path中最后一个‘/’之后的内容
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void find(char *path, char *target_file) {
  char buf[512], *p;
  int fd;
  struct dirent de;  // 目录信息
  struct stat st;  // 

  if((fd = open(path, 0)) < 0){  // 将fd定向到path
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){  // 读入fd指向的文件info
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if (read(fd, &de, sizeof(de)) != sizeof(de)) {  // 判断能否读入fd指向的文件
    fprintf(2, "find, cannot read %s\n", path);
    exit(1);
  }

  switch(st.type){
  case T_FILE:
    if (strcmp(de.name, target_file) == 0) {
      printf("%s/%s\n", path, target_file);
    }
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';  // buf下一层文路径
    while(read(fd, &de, sizeof(de)) == sizeof(de)){  // 读取fd指向的文件
      if((de.inum == 0) || (strcmp(de.name, ".") == 0) || (strcmp(de.name, "..") == 0)){
        continue;
      }
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("find: cannot stat %s\n", buf);
        continue;
      }
      if (st.type == T_FILE) {
        if (strcmp(de.name, target_file) == 0) printf("%s\n", buf);
      }
      else if (st.type == T_DIR) find(buf, target_file);  // 如果当前是目录，需要递归比对
    }
    break;
  }
  close(fd);    
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(2, "Usage: find [dir] [target_file]");
        exit(1);
    }
    else {
        find(argv[1], argv[2]);
        exit(0);
    }
}