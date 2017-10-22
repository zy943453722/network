/*************************************************************************
	> File Name: client.c
	> Author: 
	> Mail: 
	> Created Time: 2017年10月22日 星期日 15时25分07秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include<fcntl.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"
#define MAX 1024

void client(int readfd, int writefd)
{
    ssize_t n;
    size_t len;
    char buff[MAX];

    while(1)
    {
        fgets(buff,MAX,stdin);
        len = strlen(buff);
        if(buff[len-1] == '\n')
            len--;
        write(writefd,buff,len);
        memset(buff,sizeof(buff),0);
        if((n = read(readfd,buff,MAX)) ==  0)
            perror("recv failed");
        printf("server: %s\n",buff);
    }
}

int main()
{
    int readfd,writefd;

    writefd = open(FIFO1,O_WRONLY,0);
    readfd = open(FIFO2,O_RDONLY,0);

    client(readfd,writefd);

    close(readfd);
    close(writefd);
    unlink(FIFO1);
    unlink(FIFO2);
    return 0;
}
