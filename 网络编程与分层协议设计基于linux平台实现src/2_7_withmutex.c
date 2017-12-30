/*这是使用了互斥量之后实现的之前的程序*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

struct addself
{
	int m_count;
	pthread_mutex_t m_lock;
};
void * thr_fn1(void *arg)
{
	int temp;
	for(temp=1;temp<=100;temp++){
		((struct addself *)arg)->m_count++;usleep(1);
	}
	pthread_exit((void*)1);
}
void *thr_fn2(void  *arg)
{
	int temp;
	for(temp=1;temp<=100;temp++){
		((struct addself *)arg)->m_count++;
	}
	pthread_exit((void*)2);
}

int main()
{
	int err1,err2;
	pthread_t tid1,tid2;
	struct addself * ptr;

	if((ptr=(struct addself *)malloc(sizeof(struct addself)))==NULL){
		perror("malloc struct addself error");
		exit(1);
	}
	ptr->m_count=0;
	if(pthread_mutex_init(&ptr->m_lock,NULL)!=0){
		perror("initialize the mutex error");
		free(ptr);
		exit(1);
	}
	err1=pthread_create(&tid1,NULL,thr_fn1,ptr);
	if(err1!=0)perror("can not create thread 1");
	err2=pthread_create(&tid2,NULL,thr_fn2,ptr);
	if(err2!=0)
		perror("can not create thread 2");
	err1=pthread_join(tid1,NULL);
	err2=pthread_join(tid2,NULL);
	if(err1==0&&err2==0)
	{
		printf("the thread 1 and thread 2 have add i with 1 for 100 times respectively\n");
		printf("the result of m_count is %d\n",ptr->m_count);
	}
	exit(0);
}

