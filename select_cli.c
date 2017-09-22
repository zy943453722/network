/*************************************************************************
	> File Name: selectcli.c
	> Author: 
	> Mail: 
	> Created Time: 2017年09月18日 星期一 21时38分58秒
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<sys/types.h>  /* See NOTES */
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main()
{
    char buff[1024];
    char name[20];
    char a[4] = "exit";
    char b[2] = "ok";
    int sock;
    if((sock = socket(PF_INET,SOCK_STREAM,0)) < 0)
        perror("socket");

    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8888);
//    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //inet_aton("127.0.0.1",&serveraddr.sin_addr);
    if(connect(sock,(struct sockaddr*)&serveraddr,sizeof(serveraddr))< 0)
        perror("connect");
    while(1)
    {
       printf("请输入您的用户名:\n");
       scanf("%s",name);
       getchar();
    //fgets(name,sizeof(name),stdin);
    //name[strlen(name)-1] = '\0';//把/n给去掉
      send(sock,name,strlen(name),0);
      memset(buff,0,sizeof(buff));
      recv(sock,buff,sizeof(buff),0);
   //fputs(buff,stdout);
      if(strncmp(buff,a,4) == 0)
      {
        printf("当前用户已存在！\n");
      }
      if(strncmp(buff,b,2) == 0)
      {
         break;
      }
    }
      char sendbuff[1024] = {0};
      char recvbuff[1024] = {0};
      fd_set rset;
      FD_ZERO(&rset);
      int nready = 0;
      int maxfd;
      int std = fileno(stdin);//获取文件描述符
      if(std > sock)
         maxfd = std;
      else
         maxfd = sock;
      while(1)
      {
        FD_SET(sock,&rset);
        FD_SET(std,&rset);

        nready = select(maxfd + 1,&rset,NULL,NULL,NULL);
        if(nready < 0)
            perror("select");
        if(nready == 0)
            continue;
        if(FD_ISSET(sock,&rset))
        {
            int ret = recv(sock,recvbuff,sizeof(recvbuff),0);
            if(ret == -1)
                perror("recv");
            else if(ret == 0)
            {
                printf("server close\n");
                exit(EXIT_SUCCESS);
            }
            else
            {
              fputs(recvbuff,stdout);
            memset(recvbuff,0,sizeof(recvbuff));
            }
        }
        if(FD_ISSET(std,&rset))
        {
            if(fgets(sendbuff,sizeof(sendbuff),stdin) == NULL)
                break;
            send(sock,sendbuff,sizeof(sendbuff),0);
            memset(sendbuff,0,sizeof(sendbuff));
         }
       }
    close(sock);
    return 0;
}
