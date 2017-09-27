/*************************************************************************
	> File Name: poll_cli.c
	> Author: 
	> Mail: 
	> Created Time: 2017年09月27日 星期三 20时06分38秒
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

int main()
{
    
    int sock;
    if((sock = socket(PF_INET,SOCK_STREAM,0)) < 0)
        perror("socket");

    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(51880);
//    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //inet_aton("127.0.0.1",&serveraddr.sin_addr);
    if(connect(sock,(struct sockaddr*)&serveraddr,sizeof(serveraddr))< 0)
        perror("connect");
    char sendbuff[1024] = {0};
    char recvbuff[1024] = {0};
    struct pollfd que[2];
    int nready = 0;
    int maxfd;
    int std = fileno(stdin);

    que[0].fd = std;
    que[0].events = POLLIN;

    que[1].fd = sock;
    que[1].events = POLLIN;

    if(std > sock)
        maxfd = std;
    else
        maxfd = sock;
    while(1)
    {
        nready = poll(que,2,-1);
        if(nready < 0)
            perror("select");
        if(nready = 0)
            continue;
        if(que[1].revents & POLLIN)
        {
            int ret = recv(sock,recvbuff,sizeof(recvbuff),0);
            if(ret == -1)
                perror("recv");
            if(ret == 0)
            {
                printf("server close\n");
                exit(EXIT_SUCCESS);
            }

            fputs(recvbuff,stdout);
            memset(recvbuff,0,sizeof(recvbuff));
        }
        if(que[0].revents & POLLIN)
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
