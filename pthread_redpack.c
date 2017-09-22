/*************************************************************************
	> File Name: pthread_redpack.c
	> Author: 
	> Mail: 
	> Created Time: 2017年09月20日 星期三 20时20分17秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<signal.h>
#include<string.h>
#include<time.h>

#define MAX 200
#define MIN 0.01
#define Total 20
typedef struct person
{
    int id;
    char name[20];
    int flag;
    int money;
}Person;
Person per[Total];
double money = 0.0;
int num = 0;
pthread_mutex_t lock;//创建锁

void *producer(void* value)//生产者只负责产生红包金额和红包个数
{
   double n = *(double*)value;
   pthread_mutex_lock(&lock);
   money = n;
   pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

void *consumer(void *n)//消费者用于随机获取红包个数
{
   int nn = *(int*)n;
   pthread_mutex_lock(&lock);
    //printf("number%d\n",nn);
   if(money > 0 && per[nn].flag == 0)//还没抢过
    {
        if(num > 1)
        {
          srand((unsigned)time(NULL));//初始化随机数种子       
          double avg = money / num;//获取平均值
          double t = avg*2;
          double randmoney = (rand()%(int)(t*100) + (int)MIN*100)/100.00 + MIN;//初始化随机数
          money-=randmoney;
          printf("%s取走%.2lf元\n",per[nn].name,randmoney);
          printf("剩余%.2lf元\n",money);
          per[nn].id = nn;
          per[nn].money = randmoney;
          per[nn].flag = 1;
          num--;
        }
        else
        {
            printf("%s取走%.2lf元\n",per[nn].name,money);
            printf("剩余0元\n");
            money = 0;
            per[nn].id = nn;
            per[nn].money = money;
            per[nn].flag = 1;
            num--;
        }
    }
  //printf("mutex\n");
    pthread_mutex_unlock(&lock);
    //printf("exit\n");
    pthread_cancel(pthread_self());
}
int main()
{
    char *name[] = {"zy","tom","alice","tony","sam","fred","amy","zyu","ll","yl","eh","dh","tjm","lk","sx","jx","gl","yj","gc","efw"};
    int i = 0;
    double value = 0.0;
    pthread_mutex_init(&lock,0);
    while(1)
    {
       printf("请输入红包的金额：\n");
       scanf("%lf",&value);
       printf("请输入红包个数:\n");
       scanf("%d",&num);
        if(value >= MIN && value <= MAX && num > 0)
        {
            break;
        }
        printf("输入有误，请重输！\n");
    }
       for(i = 0; i < Total; i++)
       {
          strcpy(per[i].name,name[i]);
          per[i].flag = 0;
          per[i].id = -1;//看有哪些人抢到了红包
       }
   pthread_t th_a,th_b[Total];
   // void *retval;//创建目标线程返回的退出信息
 
   //pthread_attr_t attr;//创建进程属性对象
   //pthread_attr_init(&attr);
   //pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);//使调用线程脱离与进程只能跟其他线程同步
   pthread_create(&th_a,NULL,producer,(void*)&value);
    pthread_join(th_a,NULL);
    int args[Total];
   for(i = 0; i < Total;i++)
    {
        args[i] = i;
        if(per[i].flag == 0)
        {
            pthread_create(&th_b[i],NULL,consumer,(void*)&args[i]);
    //        usleep(100);
            //printf("create success\n");
           // printf("i = %d\n",i);
        }
    }
    for(i = 0; i < Total; i++)
    {
        pthread_join(th_b[i],NULL);
       //printf("i++ = %d\n",i);
    }
    pthread_mutex_destroy(&lock);
    return 0;
}
