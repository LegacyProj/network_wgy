#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<errno.h>
#include<time.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include"list.h"                  //list.h
#define BUFSIZE 512
#define QUESIZE 16

struct dgrm{                         //数据报结构体队列
	char *dgrstr;
	size_t dgrlen;
	struct sockaddr_in *peer;
	socklen_t sklen;
	struct list_head list;              //链表元素
};
typedef struct dgrm DGRM,*P_DGRM;
static int nqueue;
LIST_HEAD(dg_queue);               //创建链表

int s;
struct sockaddr_in peer_addr;
static socklen_t socklen=sizeof(peer_addr);

static void bail(const char *on_what){
	fputs(strerror(errno),stderr);
	fputs(":",stderr);
	fputs(on_what,stderr);
	fputc('\n',stderr);
	exit(1);
}

void do_sigio(int signum){
	int z;
	P_DGRM p_so;
	char buf[BUFSIZE];
   	for(;;){
    	if(nqueue>=QUESIZE){
       		write(STDOUT_FILENO,"request queue is full!\n",23);
       		return;
       	}
		p_so=(P_DGRM)malloc(sizeof(DGRM));
		p_so->dgrstr=malloc(BUFSIZE);
		p_so->peer=malloc(socklen);
		p_so->sklen=socklen;
   		z=recvfrom(s,p_so->dgrstr,BUFSIZE,0,
              (struct sockaddr *)p_so->peer,
              &socklen);
       	if(z<0){
       		if(errno==EWOULDBLOCK)
          		break;
        	else{
          		write(STDOUT_FILENO,"recvfrom error!\n",16);
          		exit(1);
         	}
       	}
       	p_so->dgrstr[z]=0;
       	p_so->dgrlen=z;
       	list_add_tail(&(p_so->list),&dg_queue);
       	nqueue++;
   	}
}

static void install_sigio(){
	struct sigaction sigio_action;

	memset(&sigio_action,0,sizeof(sigio_action));
	sigio_action.sa_flags=0;
	sigio_action.sa_handler=do_sigio;

	if(sigaction(SIGIO,&sigio_action,NULL)==-1)
		perror("Failed to set SIGIO");
}

void set_sockopt(int s,int flags){
	fcntl(s,F_SETOWN,getpid());
	if((flags=fcntl(s,F_GETFL,0))<0)
    	bail("F_GETFL error");
 	flags |=O_ASYNC | O_NONBLOCK;
 	if(fcntl(s,F_SETFL,flags)<0)
    	bail("F_SETFL error");
}

int main(int argc,char **argv){
	int z;
	char *srvr_addr=NULL;
	int len_inet;
	int portnumber;
	int flags;
	nqueue=0;

	struct sockaddr_in srvaddr;
	char dtfmt[BUFSIZE];
	time_t td;
	struct tm tv;
	sigset_t zeromask,newmask,oldmask;
	P_DGRM p_so;

	if(argc>2){
		srvr_addr=argv[1];
		if((portnumber=atoi(argv[2]))<0){
 			fprintf(stderr,"Usaeg:%s portnumber\a\n",argv[0]);
 			exit(1);
		}
	}
	else{
		srvr_addr="0";
		portnumber=9000;
	}
	s=socket(AF_INET,SOCK_DGRAM,0);
	if(s==-1)
 		bail("socket()");
 	install_sigio();
 	set_sockopt(s,flags);

 	memset(&srvaddr,0,sizeof srvaddr);
 	srvaddr.sin_family=PF_INET;
 	srvaddr.sin_port=htons(portnumber);
 	if(!inet_aton(srvr_addr,&srvaddr.sin_addr))
    	bail("bad address.");

  	len_inet=sizeof(srvaddr);

  	z=bind(s,(struct sockaddr *)&srvaddr,len_inet);
    if(z==-1)
     	bail("bind()");

  	sigemptyset(&zeromask);
  	sigemptyset(&newmask);
  	sigemptyset(&oldmask);
  	sigaddset(&newmask,SIGIO);
  	sigprocmask(SIG_BLOCK,&newmask,&oldmask);

  	for(;;){
       	while (nqueue==0)
     	sigsuspend(&zeromask);
     	sigprocmask(SIG_SETMASK,&oldmask,NULL);

  		list_for_each_entry(p_so,&dg_queue,list){
       		time(&td);
   			tv=*localtime(&td);
   			strftime(dtfmt,sizeof dtfmt,p_so->dgrstr,&tv);
   			z=sendto(s,dtfmt,strlen(dtfmt),0,(struct sockaddr *)(p_so->peer),p_so->sklen);
    		if(z<0)
       			bail("sendto()");
      		list_del(&(p_so->list));
     	}
       	sigprocmask(SIG_BLOCK,&newmask,&oldmask);
     	nqueue--;
	}
   	close(s);
   	return 0;
}
