/*************************************************************************
	> File Name: fifo_norelationship.c
	> Author: 
	> Mail: 
	> Created Time: 2017年10月22日 星期日 10时36分42秒
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
//定义允许用户读、写、组成员读和其他用户读的权限
#define FILE_MODE (S_IRUSR | S_IWUSR |S_IRGRP |S_IROTH)
#define FIFO1 "/home/zy/zy/network/fifo.3"
#define FIFO2 "/home/zy/zy/network/fifo.4"

void server(int readfd,int writefd)
{
    int fd;
    ssize_t n;
    char buff[MAX + 1];
    if((n = read(readfd,buff,MAX)) == 0)//对于read函数只要该管道中存在一些数据就会马上返回，不必等待达到所请求的字节数
      perror("file is empty");//已经读到没有数据了
    buff[n] = '\0';//给读到的路径最后添加\0结束符
    if((fd = open(buff,O_RDONLY)) < 0)//以只读形式打开路径对应文件
    {
        sprintf(buff,"文件路径出错");
        n = strlen(buff);
        memset(buff,0,MAX);
        write(writefd,buff,n);//通知客户端说出错了
    }
    else
    {
       //memset(buff,0,MAX);
       while((n = read(fd,buff,MAX)) > 0)//不memset的情况下是怎么使buff清空的?
        write(writefd,buff,n);
       close(fd);
    }
} 

int main()
{
    int readfd,writefd;

    //当创建失败且并不是已经存在的错误时
    if((mkfifo(FIFO1,FILE_MODE)) < 0 && (errno != EEXIST))//mkfifo函数第一个参数指路径，即管道的名字，第二个参数指定文件权限
     perror("can't create FIFO1\n");
    if((mkfifo(FIFO2,FILE_MODE)) < 0 && (errno != EEXIST))
    {
        unlink(FIFO1);//删除指定路径的文件，并清空此文件的系统资源等
      perror("can't create FIFO2\n");
    }
        readfd = open(FIFO1,O_RDONLY,0);
        writefd = open(FIFO2,O_WRONLY,0);
        server(readfd,writefd);
    return 0;
}

