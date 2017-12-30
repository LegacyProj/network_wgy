#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<errno.h>
#include<strings.h>
#include<string.h>
#include<sys/types.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<signal.h>
#include<pthread.h>

#define VOTE_REQ 0xA9
#define INQY_REQ 0xAA
#define INQY_RES 0xA8
#define IDSIZE 1
#define MAXTEAM 64
#define BUFSIZE 256

struct vote{
    unsigned char type : 2,
                   prec : 6;
    unsigned short len;
    unsigned char content[0];
}__attribute__((packed));

struct voteinfo{
   unsigned char id;
   unsigned int num;
}__attribute__((packed));

#define HSIZE sizeof(struct vote)
#define VISIZE sizeof(struct voteinfo)
unsigned char buf[BUFSIZE];
unsigned char recevie[BUFSIZE];
unsigned char teamid[MAXTEAM];
static struct voteinfo votelist[MAXTEAM],vote[MAXTEAM];
int listenfd, *iptr;
pthread_mutex_t counter_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t buffer_mutex=PTHREAD_MUTEX_INITIALIZER;

int init(struct voteinfo *);
void do_all(int);
int reply_vote(struct voteinfo *votelist,int num);
static void * doit(void *);

int main(int argc,char *argv[]){
struct sockaddr_in servaddr, cliaddr;
int clilen;
int port;
pthread_t tid;
init(votelist);
if(argc!= 2){
    fprintf(stderr,"Usage:a.out <port>",argv[0]);
    exit(1);
     }

    if((port=atoi(argv[1]))<0){
    fprintf(stderr,"Usage %s port\n",argv[0]);
    exit(1);
    }
 if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
printf("socket error");
exit(1);
}

bzero(&servaddr, sizeof(servaddr));
servaddr.sin_family = AF_INET;
servaddr.sin_port = htons(port);
servaddr.sin_addr.s_addr = INADDR_ANY;

if( bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
printf("bind error");
exit(1);
}
if( listen(listenfd, 5) < 0 ) {
printf("listen error");
exit(1);
}

while(1) {
    clilen=sizeof(cliaddr);
    iptr=malloc(sizeof(int));
if( (*iptr = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0 ) {
printf("accept error");
exit(1);
}
pthread_create(&tid,NULL,&doit,iptr);
}
}

static void * doit(void *arg)
{
    int conn;
    conn=*((int *)arg);
    free(arg);
    pthread_detach(pthread_self());
    do_all(conn);
    close(conn);
    return (NULL);
}
int init(struct voteinfo *v){
int i;
for(i=0;i<MAXTEAM;i++){
v[i].id=i;
v[i].num=0;}
}

void do_all(int connfd){
    unsigned char *ms,*mss;
    unsigned char type,id;
     int i;
    int number;
    int len;
    char receive[BUFSIZE];
    char buffer[BUFSIZE];
    init(vote);
    while(1){
    memset(buffer,0,BUFSIZE);
if(read(connfd,buffer,BUFSIZE)==0)
break;
type=buffer[0];
if(type==VOTE_REQ){
    id=buffer[3];
    pthread_mutex_lock(&counter_mutex);
votelist[id].num++;
sprintf(receive,"Vote sucessful,thanks!\nThe num is %d\n",votelist[id].num);
write(connfd,receive,strlen(receive));
pthread_mutex_unlock(&counter_mutex);
}
if(type==INQY_REQ){
number=buffer[1]-3;
for(i=0;i<number;i++){
vote[i].id=votelist[buffer[i+3]].id;
vote[i].num=votelist[buffer[i+3]].num;
}
pthread_mutex_lock(&buffer_mutex);
len=reply_vote(vote,number);
write(connfd,buf,len);
pthread_mutex_unlock(&buffer_mutex);
}
}
}

int reply_vote(struct voteinfo *vote,int num){
struct vote v;
unsigned char type=INQY_RES;
memset(buf,0,BUFSIZE);
*(unsigned char *)&v=type;
v.len=HSIZE+VISIZE*num;
memcpy(buf,&v,HSIZE);
memcpy(buf+HSIZE,vote,VISIZE*num);
return v.len;
}
