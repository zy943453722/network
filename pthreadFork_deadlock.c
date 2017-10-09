/*************************************************************************
	> File Name: pthreadFork.c
	> Author: 
	> Mail: 
	> Created Time: 2017年10月09日 星期一 20时55分39秒
 ************************************************************************/

#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<wait.h>
/*此程序是父进程中创建一个子线程，而后再创建一个子进程，经检验发现，子进程会自动继承父进程中的互斥锁、条件变量等，而不会自动创建父进程相同数量的线程
子进程不清楚父进程中的互斥锁状态，因此会造成死锁*/
/*解决方法用pthread_atfork函数，确保fork调用后父子进程都有一个清楚的锁状态*/
/*此函数第一个参数在fork之前调用给父进程的互斥锁加锁，第二个参数在父进程执行释放prepare锁住的互斥锁，第三个参数在子进程中执行释放prepare锁住的互斥锁。*/
pthread_mutex_t mutex;
void *another(void *arg)
{
    printf("child thread\n");
    pthread_mutex_lock(&mutex);
    sleep(3);
    pthread_mutex_unlock(&mutex);
}

void prepare()
{
    pthread_mutex_lock(&mutex);
}

void parent()
{ 
  pthread_mutex_unlock(&mutex);
}

void child()
{
    pthread_mutex_unlock(&mutex);
}

int main()
{
    pthread_mutex_init(&mutex,NULL);
    pthread_t id;
    pthread_create(&id,NULL,another,NULL);
    sleep(1);
    pthread_atfork(&prepare,&parent,&child);
    int pid = fork();
    if(pid < 0)
    {
        pthread_join(id,NULL);
        pthread_mutex_destroy(&mutex);
        return 1;
    }
    else if(pid == 0)
    {
        printf("child process\n");
        pthread_mutex_lock(&mutex);
        printf("i can not run\n");
        pthread_mutex_unlock(&mutex);
        exit(0);
    }
    else
    {
        wait(NULL);
    }
    pthread_join(id,NULL);
    pthread_mutex_destroy(&mutex);
    return 0;
}
