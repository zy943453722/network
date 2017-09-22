/*************************************************************************
	> File Name: select_ser.c
	> Author: 
	> Mail: 
	> Created Time: 2017年09月18日 星期一 21时38分02秒
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<sys/types.h>  /* See NOTES */
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/select.h>
#include<sys/time.h>

typedef struct client
{
   int connfd;
   char name[20];
}Client;

int main()
{
    char buf[1024];
    //char sendbuf[1024];
    char recvbuf[1024];
    int listenfd;
    if((listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
        perror("socket");
    struct sockaddr_in serveraddr;
    bzero(&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8888);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
   
    int on = 1;
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0)
        perror("setsockopt");
    if((bind(listenfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))) < 0)
       perror("bind");
    if(listen(listenfd,SOMAXCONN) < 0)
       perror("listen");

    struct sockaddr_in clientaddr;
    socklen_t clilen = sizeof(clientaddr);
    int connfd;
    Client cli[FD_SETSIZE];
    for(int i = 0; i < FD_SETSIZE; i++)
    {
        cli[i].connfd = -1;//保证所有文件描述符初始化为-1    
    }
    
    fd_set rset,allset;//用两个fd_set保证FD_ISSET清空描述符集合后，经过accept之后的事件不会丢掉
    FD_ZERO(&rset);
    FD_ZERO(&allset);
    int maxfd = listenfd; // 记录当前描述符集合的最大文件描述符
    int nready = 0; //表示准备好的文件描述符个数
    int count = 0;//表示连接人数
    int a = 0;
    char name[20];
    while(1)
    {
        rset = allset;
        FD_SET(listenfd,&rset);//设置监听对象

        nready = select(maxfd + 1, &rset,NULL,NULL,NULL);
        if(nready < 0)
          perror("nready");
        else if(nready == 0)
          continue;
        else
        {
            if(cli[FD_SETSIZE - 1].connfd < 0)//表示当描述符集还有空间时
            {
                if(FD_ISSET(listenfd,&rset))
                {
                   connfd = accept(listenfd,(struct sockaddr*)&clientaddr,&clilen);
                  while(1)//此循环保证名字相同时能再次收到新名字
                  {
                    memset(name,0,sizeof(name));
                    recv(connfd,name,sizeof(name),0);
                    //fputs(name,stdout);
                     int b = 0;
                     for(b = 0; b < FD_SETSIZE;b++)
                     { 
                       if(strcmp(name,cli[b].name) == 0)
                        {
                            memset(buf,0,sizeof(buf));
                            sprintf(buf,"exit");
                            send(connfd,buf,sizeof(buf),0);
                            break;
                        }
                      }
                      if(b == FD_SETSIZE)
                      {
                            break;
                      }
                   }
                    for(a = 0; a < FD_SETSIZE; a++)
                     {
                        if(cli[a].connfd < 0)
                       {
                         cli[a].connfd = connfd;
                           memset(buf,0,sizeof(buf));
                           sprintf(buf,"ok");
                           send(connfd,buf,sizeof(buf),0);
                         strcpy(cli[a].name,name);
                           printf("%s进入聊天室\n",cli[a].name);
                         break;
                       }
                      }//为了把对应某个客户端的文件描述符放进去
                        printf("IP = %s PORT = %d\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
                       count++;
                       printf("当前的连接人数为:%d\n",count);
                       FD_SET(connfd,&allset);
                       if(connfd > maxfd)
                       maxfd = connfd;
                       if(--nready <= 0)//--的目的是经过FD_ISSET之后就绪集合减少一个已经处理了的事件
                       continue;
                }
                      for(int i = 0; i < FD_SETSIZE; i++)//处理多个客户端
                      {
                       connfd = cli[i].connfd;
                        if(connfd == -1)
                           continue;
                        if(FD_ISSET(connfd,&rset))
                        {
                           memset(recvbuf,0,sizeof(recvbuf));
                           int ret = recv(connfd,recvbuf,sizeof(recvbuf),0);
                           if(ret == -1)
                              perror("recv");
                           else if(ret == 0)
                           {
                              printf("%s close!\n",cli[i].name);
                              count--;
                              printf("当前连接人数：%d\n",count);
                              FD_CLR(connfd,&allset);//关闭之前用于临时存储的allset的事件
                              cli[i].connfd = -1;
                            }
                          else
                          {
                             memset(buf,0,sizeof(buf));
                             sprintf(buf,"%s:%s",cli[i].name,recvbuf);
                             printf("%s\n",buf);
                             memset(buf,0,sizeof(buf));
                             for(int j = 0; j < FD_SETSIZE; j++)
                             {
                               if(cli[j].connfd >= 0 && i != j)
                               {
                                sprintf(buf,"%s:%s",cli[i].name,recvbuf);  
                                send(cli[j].connfd,buf,sizeof(buf),0);
                               } 
                              }
                              if( --nready <= 0 )
                                 break;//表示客户端全都退出了   
                            }
                         }
                      }
            }
            else
            {
                    printf("链接人数超出上限！");
                    break;
            }
        }
    }
    return 0;
}
