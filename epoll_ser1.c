/*************************************************************************
	> File Name: epoll_ser1.c
	> Author: 
	> Mail: 
	> Created Time: 2017年11月13日 星期一 22时43分40秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<sys/select.h>
#include<sys/epoll.h>
#include <fcntl.h>

#define BACKLOG 10
#define MYPORT 12345
#define MaxClients 500
#define localIP 127.0.0.1
#define netIP 192.168.1.107


struct Client
{
    char IP[20];
    char PORT[20];
    char name[20];
    int fd;
};
struct sockaddr_in my_addr;
struct sockaddr_in their_addr;
struct Client clients[MaxClients];
int Init()
{
    int sockfd;
    if((sockfd = socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("socket");
        exit(1);
    }
    int on = 1;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(MYPORT);
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))< 0)//地址重复利用

        bzero(&(my_addr.sin_zero),8);
    if(bind(sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if(listen(sockfd,BACKLOG)==-1)
    {
        perror("listen");
        exit(1);
    }  
    return sockfd;
}
void Devide_Msg(char *buf,char *dname,char* newbuf,char *name)
{
    int m = 0;
    int n = 0;
    while(buf[m] != ':')
    {
        name[m] = buf[m];  //name 存发消息客户端的名字
        m++;                                //gao:abcde
    }
    if(buf[m+1] == '@')
    {
        int i = 0;
        for(i = m+2 ;buf[i] != ':';i++)
        {
            dname[n++] = buf[i]; 
        }
        n = 0;
        for(int p = i+1;buf[p] != '\0';p++)
        {
            newbuf[n++] = buf[p];   //newbuf 存客户端发送的消息
        }
    }
    else
    {
        n = 0;
        for(int i = m+1;buf[i] != '\0';i++)
        {
            newbuf[n++] = buf[i];   //newbuf 存客户端发送的消息
        }
    }

}
void setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}
int Get_sendfd(char *msg,char *name,char *newbuf,char *dname)
{
    int sendfd = -1;
    if(strcmp(dname,"\0")!= 0)
    {
        for(int k = 0 ; k < 50 ;k++)
        {
            if(strcmp(clients[k].name,dname) == 0)
            {
                sendfd = clients[k].fd;
                break;
            }
        }
    }
    return sendfd;
}

int main()
{
    int i = 0;
    int total = 0;
    char buf[1024] = {'\0'};
    char myput[1024] = {'\0'};
    int numbytes;
    int new_fd;
    int conn = -1;
    int sockfd = Init();
    //setnonblocking(sockfd);
    int sin_size;
    int sendfd;
    char name[20];
    char newbuf[200] ;
    char dname[20];
    char msg[300];
    char sendmsg[300];
    memset(buf,0,sizeof(buf));
    memset(name,0,sizeof(name));
    memset(dname,0,sizeof(dname));
    memset(newbuf,0,sizeof(newbuf));
    memset(msg,0,sizeof(msg));
    memset(sendmsg,0,sizeof(sendmsg));
    int epollfd = epoll_create(MaxClients);   //创建epoll文件描述符
    struct epoll_event rgfd; //用来注册新事件
    struct epoll_event Epoll_Cli[MaxClients]; //存放Epoll返回的可读写事件
    for(int i = 0;i <MaxClients;i++)
    {
        Epoll_Cli[i].data.fd = -1;
        clients[i].fd = -1;
    }

    rgfd.data.fd = sockfd;
    rgfd.events = EPOLLIN;
    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd,&rgfd) == -1) //将监听的fd注册到epoll中
    {
        perror("register failed");
    }
    while(1)
    {
        int rtnum = epoll_wait(epollfd,Epoll_Cli,MaxClients,-1);
        for(int t = 0;t < rtnum ; t++)
        {

            if( (Epoll_Cli[t].data.fd == sockfd) && (Epoll_Cli[t].events & EPOLLIN) )
            {
                sin_size = sizeof(their_addr);
                if(  (conn = (accept(sockfd,(struct sockaddr *)&their_addr,&sin_size)  )) == -1)
                {
                    perror("accept");
                    continue;
                }
                for(int k = 0; k < MaxClients;k++)
                {
                    if(clients[k].fd == -1)
                    {
                        Epoll_Cli[k].data.fd = conn;
                        clients[k].fd = conn;
                        // printf("accept fd:%d\n",clients[k].fd);
                        break;
                    }
                }
                printf("新客户已经链接！IP:%s,Port:%d\n",inet_ntoa(their_addr.sin_addr),ntohs(their_addr.sin_port));
                printf("当前在线人数:%d\n",++total);
                //printf("newconnfd:%d\n",conn);
                rgfd.data.fd = conn;
                rgfd.events = EPOLLIN ;
                if(epoll_ctl(epollfd,EPOLL_CTL_ADD,conn,&rgfd) == -1) //将新连接的fd注册到epoll中
                {
                    perror("epoll_ctl");
                }
            }
            else if(Epoll_Cli[t].events & EPOLLIN) 
            {
                conn = Epoll_Cli[t].data.fd;
                if(conn == -1)
                    continue;
                numbytes = recv(conn,buf,sizeof(buf),0);
                Devide_Msg(buf,dname,newbuf,name);
                for(int i = 0;i < MaxClients;i++)
                {
                    if(clients[i].fd == conn)
                    {
                        strcpy(clients[i].name,name);
                        break;
                    }
                }
                if(numbytes == -1)
                {
                    perror("recieve");
                    exit(0);
                }
                else if(numbytes == 0)
                {
                    printf("Port = %d已经关闭连接!\n",ntohs(their_addr.sin_port));
                    printf("当前在线人数:%d\n",--total);
                    clients[t].fd = -1;
                    Epoll_Cli[t].data.fd = -1;
                    close(conn);
                    memset(clients[t].name,0,sizeof(clients[t].name));
                }
                else
                {
                    printf("name=%s\n",name);
                    printf("dname=%s\n",dname);
                    printf("msg=%s\n",newbuf);
                    sendfd = Get_sendfd(msg,name,newbuf,dname);
                    if(sendfd != -1)
                    {
                        strcat(sendmsg,name);
                        strcat(sendmsg,":");
                        strcat(sendmsg,newbuf);
                        msg[strlen(sendmsg)] = '\0';
                        send(sendfd,sendmsg,sizeof(sendmsg),0);
                    }
                }
            }
        }
        memset(buf,0,sizeof(buf));
        memset(name,0,sizeof(name));
        memset(dname,0,sizeof(dname));
        memset(newbuf,0,sizeof(newbuf));
        memset(msg,0,sizeof(msg));
        memset(sendmsg,0,sizeof(sendmsg));

    }

    return 0;
}

