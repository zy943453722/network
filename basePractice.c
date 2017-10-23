/*************************************************************************
	> File Name: basePractice.c
	> Author: 
	> Mail: 
	> Created Time: 2017年10月23日 星期一 19时40分11秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <mysql/mysql.h>

#define MAX 1024
int main(int argc,char *argv[])
{
    int res;//定义查询返回值，成功为0，失败为非0
    MYSQL my;//创建数据库链接句柄
    //char query[MAX];
    //char *end;//初始化指向字符串的尾部指针，用于计算sql语句长度.
    MYSQL_RES *resultSet;//创建查询结果集
    int rows;//获取结果集的行数
    int columns;//获取结果集的列数
    MYSQL_FIELD *fields;//存放结果集的字段名、大小、类型等(相当于一个结构体)
    int num_fields;//定义字段的个数
    MYSQL_ROW resultROW;//定义结果集的某一行的字段集合，可以获取某个下标的字段值

    mysql_init(&my);//初始化与连接相适应的MYSQL对象,里面包含mysql_server_init这个初始化c API库的函数
   // mysql_options(&my,MYSQL_SET_CHARSET_NAME,"gbk"); 
    mysql_set_character_set(&my,"utf8");//为当前连接设置默认的字符集
    if(mysql_real_connect(&my,"127.0.0.1","root","826451379","Student",3306,NULL,0))
    {
       printf("connection success!\n");
        //end = strmov(query,"select * from info where S_id like '3150931042'");//字符串拷贝函数
        res = mysql_query(&my,"select * from info where S_name like '%张%'");
       //res = mysql_real_query(&my,query,(unsigned int)(end - query));//当有二进制字符或者NULL时用这个查询,最后一个参数是指sql语句的长度，查询速度更快 
       if(res == 0)//查询成功
        {
           resultSet = mysql_store_result(&my);//将查询到的所有结果放入结果集
           if(resultSet == NULL)
            printf("没有查询到数据！\n");
           else
            {
                rows = mysql_num_rows(resultSet);
                columns = mysql_num_fields(resultSet);
                fields = mysql_fetch_fields(resultSet);
                printf("\t返回结果为;\n");
                for(int i = 0; i < columns; i++)
                {
                   printf("%15s",fields[i].name);
                }//输出字段名
                printf("\n");
                for(int j = 1; j < rows+1; j++)
                {
                    resultROW = mysql_fetch_row(resultSet);
                    for(int k = 0; k < columns; k++)
                    {
                        printf("%15s",resultROW[k]);
                    }
                    printf("\n");
                }
                mysql_free_result(resultSet);//释放结果集
            }
            mysql_close(&my);
        }
        else
        {
           printf("select errno %d: %s\n",mysql_errno(&my),mysql_error(&my));        
        }
        mysql_close(&my);
    }
    else
    {
        printf("connection failed!\n");
    }
    return 0;
}
