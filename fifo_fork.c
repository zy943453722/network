/*************************************************************************
	> File Name: fifo.c
	> Author: 
	> Mail: 
	> Created Time: 2017年10月20日 星期五 22时00分36秒
 ************************************************************************/
//fifo在文件系统中有一个名字，该名字允许某个进程创建，与它无亲缘关系的另一个进程也能打开，其只能通过调用unlink才能从文件系统中删除。

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
#define FIFO1 "/home/zy/zy/network/fifo.1"
#define FIFO2 "/home/zy/zy/network/fifo.2"
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
    pid_t childpid;

    //当创建失败且并不是已经存在的错误时
    if((mkfifo(FIFO1,FILE_MODE)) < 0 && (errno != EEXIST))//mkfifo函数第一个参数指路径，即管道的名字，第二个参数指定文件权限
     perror("can't create FIFO1\n");
    if((mkfifo(FIFO2,FILE_MODE)) < 0 && (errno != EEXIST))
    {
        unlink(FIFO1);//删除指定路径的文件，并清空此文件的系统资源等
      perror("can't create FIFO2\n");
    }

    childpid = fork();
    if(childpid == 0)
    {
        readfd = open(FIFO1,O_RDONLY,0);//有路径就可以用open打开且可以指定开放一端还是多端
        writefd = open(FIFO2,O_WRONLY,0);
        server(readfd,writefd);
        exit(0);
    }
    else if(childpid < 0)
      perror("fork\n");
    else
    {
        writefd = open(FIFO1,O_WRONLY,0);
        readfd = open(FIFO2,O_RDONLY,0);
        client(readfd,writefd);
        waitpid(childpid,NULL,0);
        close(readfd);
        close(writefd);
        unlink(FIFO1);//用来删除临时文件（若有进程正在打开或其他文件指向该文件节点，unlink只删除目录项，进程结束后才会立即释放）
        unlink(FIFO2);
        exit(0);
    }
    return 0;
}

