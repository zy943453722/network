/*************************************************************************
	> File Name: poll_ser.c
	> Author: 
	> Mail: 
	> Created Time: 2017年09月27日 星期三 20时06分31秒
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<sys/types.h>  /* See NOTES */
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<poll.h>
#define INFTIM -1
#define OPEN_MAX 1024

int main()
{
    int listenfd;
    if((listenfd = socket(PF_INET,SOCK_STREAM,0)) < 0)
        perror("socket");

    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(51880);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //inet_aton("127.0.0.1",&serveraddr.sin_addr);

    int on = 1;

    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))< 0)//地址重复利用
       perror("setsockopt");
    if(bind(listenfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
       perror("bind");
    if(listen(listenfd,SOMAXCONN) < 0)
       perror("listen");

    struct sockaddr_in peeraddr;
    socklen_t peer_len;
    char buf[1024];

    int conn;
    struct pollfd cli[2048];//创建pollfd结构体数组
    int i;
    int count = 0;
    for(i = 0;i < 2048; i++)
    {
        cli[i].fd = -1;
    }
    int maxfd  = listenfd;
    int nready = 0;    
    cli[0].fd =listenfd;
    cli[0].events = POLLIN;
    while(1)
    {

        nready = poll(cli,maxfd + 1,1);//此时数组中最大个数为0+1=1，最后一个参数直到出现连接才停止等待
        if(nready < 0)
            perror("select");
        if(nready = 0)
            continue;
        if(cli[0].revents & POLLIN)//检测事件是否发生并且是否是读事件
        {
            peer_len = sizeof(peeraddr);
            conn = accept(listenfd,(struct sockaddr *)&peeraddr,&peer_len);
        
            for(i = 0; i < 2048;i++)
            {
                if(cli[i].fd < 0)
                {
                    cli[i].fd = conn;
                    break;
                }
            }
            if( i == 2048)
            {
                fprintf(stderr,"超出连接数量");
                exit(EXIT_FAILURE);
            }
            printf("IP = %s PORT = %d\n",inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));
            count++;
            printf("连接的人数有%d\n",count);
            cli[i].events = POLLIN;
            if(--nready <= 0)
                continue;
        }
        for(i =0; i < 2048;i++)
        {
            conn = cli[i].fd;
            if(conn == -1)
                continue;
            if(cli[i].revents & POLLIN)
            {
                int ret = recv(conn,buf,sizeof(buf),0);
                if(ret == -1)
                {
                    perror("read");
                }
                if(ret == 0)
                {
                    printf("client close\n");
                    cli[i].fd = -1;
                    count--;
                }
                //fputs(buf,stdout);
                printf("recv cli data[%d]:%s\n", ret,buf);
                send(conn,buf,ret-1,0);
                if( --nready <= 0)
                    break;
            }
        }
    }
    return 0;
}
