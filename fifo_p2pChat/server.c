/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: 2017年10月22日 星期日 15时24分59秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"
#define MAX 1024
//此通信仅限于2个进程都能搜索到fifo管道的路径，且客户端先发起会话,实现一人说一句的功能
void server(int readfd, int writefd)
{
    size_t len;
    ssize_t n;
    char buff[MAX];
    while(1)
    {
       if((n = read(readfd,buff,MAX)) == 0)
         perror("recv failed");
        buff[n] = '\0';
       printf("client: %s\n",buff);
       memset(buff,sizeof(buff),0);
       fgets(buff,MAX,stdin);
       len = strlen(buff);
        if(buff[len-1] = '\n')
          len--;
        write(writefd,buff,len);
    }
}

int main()
{
    int readfd,writefd;

    if((mkfifo(FIFO1,FILE_MODE)) < 0 && (errno != EEXIST))
       perror("Create FIFO1 failed\n");
    if((mkfifo(FIFO2,FILE_MODE)) < 0 && (errno != EEXIST))
    {
        unlink(FIFO1);
        perror("Create FIFO2 failed\n");
    }

    readfd = open(FIFO1,O_RDONLY,0);
    writefd = open(FIFO2,O_WRONLY,0);

    server(readfd,writefd);
    return 0;
}
