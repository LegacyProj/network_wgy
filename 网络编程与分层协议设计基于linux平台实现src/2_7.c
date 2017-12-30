#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
	
int i=0;

void * thr_fn1(void *arg)
{
	int temp;
	for(temp=1;temp<=100;temp++)
	{	i++;
		usleep(1);
	}
	pthread_exit((void*)1);
}
void *thr_fn2(void *arg)
{
	int temp;
	for(temp=1;temp<=100;temp++){i++;}
	pthread_exit((void*)2);
}

int main()
{
	int err1,err2;
	pthread_t tid1,tid2;
	
	err1=pthread_create(&tid1,NULL,thr_fn1,NULL);
	if(err1!=0)
		perror("can not create thread 1");
	err2=pthread_create(&tid2,NULL,thr_fn2,NULL);
	if(err2!=0)
		perror("can not create thread 2");
	err1=pthread_join(tid1,NULL);
	err2=pthread_join(tid2,NULL);
	if(err1==0&&err2==0)
	{
	printf("the thread 1 and thread 2 have add i with 1 for 100 times respectively\n");
	printf("the result of i is %d\n",i);
	}
	exit(0);
}
/*
$ ./2_7
the thread 1 and thread 2 have add i with 1 for 100 times respectively
the result of i is 200
$ ./2_7
the thread 1 and thread 2 have add i with 1 for 100 times respectively
the result of i is 199
$ ./2_7
the thread 1 and thread 2 have add i with 1 for 100 times respectively
the result of i is 200
$ ./2_7
the thread 1 and thread 2 have add i with 1 for 100 times respectively
the result of i is 200
此程序是不使用互斥量的两个线程同时分别对一个公共变量进行自增1操作,各加100次.对此程序的
测试结果是差不多运行此程序25次就会出现一个错误(既i的值不为200),当然这个统计值跟各机器的
实际情况有关.
*/
