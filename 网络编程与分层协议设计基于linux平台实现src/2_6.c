/*
[第二章]$ ./2_6
please input a string like this: xxxx  xxxx  xxxx  xxxx
aaaa   bbbbb  ccccc
start to sleep for 20 seconds please send SIGUSR1 to this process
you can execute the command like this:kill -s usr1 pid
please input a string like this: xxxx  xxxx  xxxx  xxxx
mmmmmm      dddddddddd   ooooooooooo   ffffffffff      wwwwwwwww
mmmmmm
aaaa
dddddddddd
ooooooooooo
ffffffffff
wwwwwwwww
$

这是本例的运行的一个实例,按照strtok的原意其输出结果本来应该为:
mmmmmm
aaaa
bbbbb
ccccc
但是之所以会出现上边的输出结果,是由于strtok为不可重入函数,在strtok()函数内部使用静态全局变量存放字符串.在一个程序内部的任何位置对strtok调用都会造成对该静态全局变量的重写.
因此此函数是不安全的.建议使用strtok_r()函数替代strtok().
*/

#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
	
#define MAXLINE 4096
	
void usr_handler(int signal_number)
{
	char tem_buffer[MAXLINE],*p;
	
	bzero(tem_buffer,sizeof(tem_buffer));
	printf("please input a string like this: xxxx  xxxx  xxxx  xxxx\n");
	fgets(tem_buffer,sizeof(tem_buffer),stdin);
	tem_buffer[strlen(tem_buffer)-1]='\0';
	p=strtok(tem_buffer," ");
	printf("%s\n",p);
	return ;
}
int main()
{
	struct sigaction sig_usr_act;
	char rec_bufer[MAXLINE],*ptr;
	
	bzero(&sig_usr_act,sizeof(sig_usr_act));
	bzero(&rec_bufer,sizeof(rec_bufer));
	sig_usr_act.sa_flags=0;
	sig_usr_act.sa_handler=&usr_handler;
	if(sigaction(SIGUSR1,&sig_usr_act,NULL)==-1)
		perror("Failed to set handler for SIGUSR1");
	/*x stands for any character that can input throught the keyboard*/
	printf("please input a string like this: xxxx  xxxx  xxxx  xxxx\n");
	fgets(rec_bufer,sizeof(rec_bufer),stdin);
	rec_bufer[strlen(rec_bufer)-1]='\0';
	ptr=strtok(rec_bufer," ");
	printf("start to sleep for 20 seconds please send SIGUSR1 to this process\n");
	printf("you can execute the command like this:kill -s usr1 pid\n");
	sleep(20);
	printf("%s\n",ptr);
	while(ptr=strtok(NULL," "))
		printf("%s\n",ptr);
	exit(0);
}














