/*************************************************************************
	> File Name: pipe.c
	> Author: 
	> Mail: 
	> Created Time: 2017年10月16日 星期一 20时29分30秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/wait.h>
#define MAX 1024
//服务器作为子进程获取路径名，客户端作为父进程获取内容
//读和写是对于文件来说的不是对于管道来说的
//读文件是0，写文件是1
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
    memset(buff,0,MAX);
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
       memset(buff,0,MAX);
       while((n = read(fd,buff,MAX)) > 0)
        write(writefd,buff,n);
       close(fd);
    }
} 
int main()
{
    int pipe1[2],pipe2[2];
    pid_t pid;
    pipe(pipe1);
    pipe(pipe2);//创建2个管道

    pid = fork();
    if(pid == 0)//子进程
    {
        close(pipe1[1]);
        close(pipe2[0]);
        server(pipe1[0],pipe2[1]);
        exit(0);
    }
    else if(pid < 0)
    {
        perror("fork");
    }
    else
    {
        close(pipe1[0]);
        close(pipe2[1]);
        client(pipe2[0],pipe1[1]);
        waitpid(pid,NULL,0);//父进程等待子进程最终返回读到的内容
        exit(0);
    }
    return 0;
}
