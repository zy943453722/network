/*************************************************************************
	> File Name: fork_ser.c
	> Author: 
	> Mail: 
	> Created Time: 2017年08月18日 星期五 10时10分10秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main()
{
    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(4188);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int socketfd;
    char buf[1024] = {0};

    if((socketfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
        perror("socket");
    if(connect(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
        perror("connect");
    pid_t pid;
    pid = fork();
    if(pid == 0)
    {
       while(1)
       {
        memset(buf,0,sizeof(buf));
        if(fgets(buf,sizeof(buf),stdin) != NULL)
        {
            if(send(socketfd,buf,sizeof(buf),0) < 0)
                perror("send");
        }
       }
       exit(0);
    }
    else if(pid > 0)
    {
        while(1)
        {
          memset(buf,0,sizeof(buf));
          int rec = recv(socketfd,buf,sizeof(buf),0); 
          if(rec == -1)
          {
            perror("recv");
          }
           if(rec == 0)
           {
            printf("server close\n");
            break;
           }
         printf("recv:%s\n",buf);
        }
         exit(0);
    }
    else{
        perror("fork");
    }
    close(socketfd);
    return 0;
}
