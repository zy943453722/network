/*************************************************************************
	> File Name: deadlock.c
	> Author: 
	> Mail: 
	> Created Time: 2017年10月09日 星期一 15时27分47秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

int a = 0;
int b = 0;
pthread_mutex_t mutex_a;
pthread_mutex_t mutex_b;

void *another(void *arg)
{
    pthread_mutex_lock(&mutex_b);
    printf("lock b\n");
    //sleep(3);
    //++b;
    a++;
    printf("a = %d\n",a);
    pthread_mutex_lock(&mutex_a);
    //b += a++;
    //printf("ccccc\n");
    pthread_mutex_unlock(&mutex_a);
    pthread_mutex_unlock(&mutex_b);
    pthread_exit(NULL);
}

int main()
{
    pthread_t id;
    pthread_mutex_init(&mutex_a,NULL);
    pthread_mutex_init(&mutex_b,NULL);
    pthread_create(&id,NULL,another,NULL);
    pthread_mutex_lock(&mutex_a);
    printf("lock a\n");
    //sleep(3);
    ++a;
    printf("000a = %d\n",a);
    pthread_mutex_lock(&mutex_b);
    //a+= b++;
    //printf("bbbbbb\n");
    pthread_mutex_unlock(&mutex_b);
    pthread_mutex_unlock(&mutex_a);
    pthread_join(id,NULL);
    pthread_mutex_destroy(&mutex_a);
    pthread_mutex_destroy(&mutex_b);
    return 0;
}//主线程试图占用a锁，但没有立即释放a，而是又去申请b锁。
/* 问题一：为什么没有sleep时还会进入回调函数*/
//加a锁只是保护a锁的资源，可以在加a锁的基础上加b锁，但要使a锁和b锁分别保护不同的临界区，否则就会出现读脏数据。
