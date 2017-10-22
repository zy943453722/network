/*************************************************************************
	> File Name: pipe_fullDuplex.c
	> Author: 
	> Mail: 
	> Created Time: 2017年10月17日 星期二 20时35分10秒
 ************************************************************************/
//根据实验发现父进程根本读不到自己输入的p,只有子进程读到了这个p，但想用同一管道进行反向写时，却出错。说明pipe无法实现全双工通信。
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
int main()
{
    int fd[2],n;
    char c;
    pid_t childpid;

    pipe(fd);
    childpid = fork();
    if(childpid == 0)
    {
        //sleep(3);
        if((n = read(fd[0],&c,1)) != 1)
          perror("child: read returned\n");
        printf("child read %c\n",c);
        write(fd[0],"c",1);
        exit(0);
    }
    write(fd[1],"p",1);
    sleep(3);
    if((n = read(fd[1],&c,1)) != 1)
       perror("parent: read returned\n");
    printf("parent read %c \n",c);
    exit(0);
    return 0;
}
