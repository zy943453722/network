/*************************************************************************
	> File Name: compareETorLT.c
	> Author: 
	> Mail: 
	> Created Time: 2017年11月12日 星期日 15时19分45秒
 ************************************************************************/

#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<pthread.h>
#include<stdbool.h>
#include<libgen.h>

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10
/*此函数只需要配合一个poll的客户端就可以运行
 * 通过结果看出，两种模式下触发事件的次数差很多，LT很多，ET很少，因此ET消耗资源少*/
//将文件描述符设置成非阻塞的*/
int setnonblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}
//将fd上的EPOLLIN事件注册到epollfd指定的内核事件表中，最后一个参数指定是否采用ET模式
void addfd(int epollfd,int fd,bool enable_et)
{
    struct epoll_event event;//此数组用于存放要注册的事件，并将这些时间加入内核事件表中
    event.data.fd = fd;
    event.events = EPOLLIN;//读事件
    if(enable_et)
    {
       event.events |= EPOLLET;//表示是边沿触发事件
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);//将填好的事件类型及具体描述符注册到内核事件表中
    setnonblocking(fd);
}
/*LT模式的工作流程*/
void lt(struct epoll_event *events,int number,int epollfd, int listenfd)
{
    char buf[BUFFER_SIZE];
    for(int i = 0; i < number; i++)//相当于在就绪的事件数组中寻找某fd的连接这个读事件
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd)//看这些触发的读事件中有没有listenfd这个文件，有的话处理连接
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listenfd,(struct sockaddr*)&client_address,&client_addrlength);
            addfd(epollfd,connfd,false);//连接完成了，为了实现通讯，将connfd注册到内核事件表
        }
        else if(events[i].events & EPOLLIN)//处理读事件且是注册的事件，读事件无非就是连接和收数据
        {
            printf("event trigger once\n");//检测socket读缓存中还是否存在未读出的数据
            memset(buf,'\0',BUFFER_SIZE);
            int ret = recv(sockfd,buf,BUFFER_SIZE-1,0);
            if(ret <= 0)
            {
                close(sockfd);
                continue;
            }
            printf("get %d bytes of content:%s\n",ret,buf);
        }
        else
        {
            printf("something else happened\n");
        }
    }
}
void et(struct epoll_event* events,int number,int epollfd,int listenfd)
{
    
    char buf[BUFFER_SIZE];
    for(int i = 0; i < number; i++)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd)
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listenfd,(struct sockaddr*)&client_address,&client_addrlength);
            addfd(epollfd,connfd,true);
        }
        else if(events[i].events & EPOLLIN)
        {
            printf("event trigger once\n");//检测socket读缓存中还是否存在未读出的数据
            memset(buf,'\0',BUFFER_SIZE);
            int ret = recv(sockfd,buf,BUFFER_SIZE-1,0);
            if(ret < 0)
            {
                /*因为这是边沿触发，有事件就绪就必须处理*/
                if((errno == EAGAIN) || (errno == EWOULDBLOCK))//对于非阻塞IO，此条件成立表示数据全部读取完毕，之后epoll就能再次触发sockfd上的EPOLLIN事件，以驱动下一次读操作
                {
                    printf("read later\n");
                    break;
                }
                close(sockfd);
                break;
            }
            else if(ret == 0)
              close(sockfd);
            else
              printf("get %d bytes of content:%s\n",ret,buf);
        }
        else
        {
            printf("something else happened\n");
        }
    }
}
int main(int argc,char* argv[])
{
    if(argc <= 2)//文件名，ip，端口至少3个变量
    {
        printf("uage:%s ip_address port_number\n",basename(argv[0]));//basename函数，获取得到特定路径中最有一个/后的内容
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET,SOCK_STREAM,0);
    assert(listenfd >= 0);
    ret = bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd,5);//全连接队列大小为5
    assert(ret != -1);

    struct epoll_event events[MAX_EVENT_NUMBER];//创建就绪事件结构体数组，有就绪就从内核事件表复制到此数组中
    int epollfd = epoll_create(5);//告知内核事件表大小为5
    assert(epollfd != -1);
    addfd(epollfd,listenfd,true);//注册socket这个连接请求事件的文件描述符

    while(1)
    {
        int ret = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);//等待注册的事件发生，并将就绪的事件复制到事件数组中
        //当ret=0时即并没有就绪的事件发生，进入lt/et函数也不会进行处理，相当于再走while循环
        if(ret < 0)
        {
            printf("epoll failure\n");
            break;
        }
        //lt(events,ret,epollfd,listenfd);//使用LT即电平触发模式
        et(events,ret,epollfd,listenfd);//使用ET即边沿触发模式
    }
    close(listenfd);
    return 0;
}
