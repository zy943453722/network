/*************************************************************************
	> File Name: popen.c
	> Author: 
	> Mail: 
	> Created Time: 2017年10月19日 星期四 20时40分42秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#define MAXLINE 1024
int main()
{
    size_t n;
    char buff[MAXLINE],command[MAXLINE];
    FILE *fp;

    if(fgets(buff,MAXLINE,stdin) < 0)//从标准输入读取路径
       perror("path error!\n");
    n = strlen(buff);
    if(buff[n-1] == '\n')
      n--;
    snprintf(command,sizeof(command),"cat %s",buff);//command参数是一个shell命令行
    fp = popen(command,"r");//这是用于读出command的输出
    memset(buff,sizeof(buff),0);
    while(fgets(buff,MAXLINE,fp) != NULL)
      fputs(buff,stdout);
    pclose(fp);//等待其中的命令结束，然后返回shell的终止状态
    exit(0);
    return 0;
}
