/*************************************************************************
	> File Name: client.c
	> Author: 
	> Mail: 
	> Created Time: 2017年10月22日 星期日 10时53分14秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<errno.h>
#define MAX 1024
#define FILE_MODE (S_IRUSR | S_IWUSR |S_IRGRP |S_IROTH)
#define FIFO1 "/home/zy/zy/network/fifo.3"
#define FIFO2 "/home/zy/zy/network/fifo.4"
void client(int readfd,int writefd)
//此处读写不是一对，是父进程读，子进程写才是一对
{
    size_t len;//相当于无符号整形
    ssize_t n;//相当于有符号整形
    char buff[MAX];
    fgets(buff,MAX,stdin);
    len = strlen(buff);
    if(buff[len-1] == '\n')
       len--;//删除fgets存入的\n符号
    write(writefd,buff,len);
    //memset(buff,0,MAX);
    while((n = read(readfd,buff,MAX)) > 0)//如果能检测到路径中的内容
      write(STDOUT_FILENO,buff,n);//将buff中的内容保存到stdout的文件描述符，并向屏幕输出
}


int main()
{
    int readfd,writefd;
        writefd = open(FIFO1,O_WRONLY,0);
        readfd = open(FIFO2,O_RDONLY,0);
        client(readfd,writefd);
        close(readfd);
        close(writefd);
        unlink(FIFO1);//客户端读出数据后关闭管道
        unlink(FIFO2);
    return 0;
}
