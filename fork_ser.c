/*************************************************************************
	> File Name: fork_ser.c
	> Author: 
	> Mail: 
	> Created Time: 2017年08月18日 星期五 10时10分21秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
int a[1024] = {-1};
int main()
{
    int i = 0;
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;//因服务器需要绑定，且需要接受，所以既需要服务器地址也需要客户端地址
    char buf[1024];
    int socketfd,sin_size;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(4188);
    //可以使其监听外部服务器发送到服务器端的所有网卡请求
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if((socketfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
       perror("socket");
     printf("%d",socketfd);
    if(bind(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
       perror("bind");
    if(listen(socketfd,SOMAXCONN) < 0)
       perror("listen");

    int connfd;
    pid_t pid,pid1;  
    socklen_t cli_len = sizeof(clientaddr);

    while(1)//保证出现多个客户端
    {
        connfd = accept(socketfd,(struct sockaddr*)&clientaddr,&cli_len);
        a[i] = connfd;
        printf("%d\n",a[i]);
        i++;
        printf("IP = %s PORT = %d\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
        pid = fork();
        if(pid == -1)
        {
            perror("fork");
        }
        else if(pid == 0)
        {
            close(socketfd);
            pid1 = fork();
            if(pid1 > 0)
            {
              while(1)
              {
                   memset(buf,0,sizeof(buf));
                   int rec = recv(connfd,buf,sizeof(buf),0);
                   if(rec == -1)
                   {
                      perror("recv");
                   }
                   if(rec == 0)
                   {
                    printf("client close\n");
                    break;
                   }
                   printf("recv:%s\n",buf);
              }
                exit(0);
            }
            else if(pid1 == 0)
            {
                while(1)
                {
                 memset(buf,0,sizeof(buf));
                 if(fgets(buf,sizeof(buf),stdin) != NULL)
                 {
                  if(send(a[i],buf,sizeof(buf),0) < 0)
                     perror("send");
                 }
                }
                exit(0);
            }
            else
            {
                perror("fork");
            }
            exit(EXIT_SUCCESS);
        }
        else
        {
            close(connfd);
        }
        if(i >= 1024)
        {
            printf("客户端数目超出！");
            break;
        }
    }
    return 0;
}
