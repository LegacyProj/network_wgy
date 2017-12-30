#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
	
int main()
{
	sigset_t newmask,oldmask,pendmask;
	sigemptyset(&newmask);
	sigaddset(&newmask,SIGTSTP);
	printf("SIGTSTP is blocked.\n");
	if(sigprocmask(SIG_BLOCK,&newmask,&oldmask)==-1)perror("SIG_BLOCK error");
		sleep(10);
	if(sigpending(&pendmask)==-1)
		perror("sigpending error");
	if(sigismember(&pendmask,SIGTSTP))
		printf("\nSIGTSTP pending\n");
	printf("SIGTSTP handler is restored.\n");
	if(sigprocmask(SIG_SETMASK,&oldmask,NULL)==-1)
		perror("SIG_SETMASK error");
	for(;;);
	
	return 0;

}
