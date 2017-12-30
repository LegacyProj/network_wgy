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


#define VOTE_REQ 0xA9
#define INQY_REQ 0xAA
#define INQY_RES 0xA8
#define IDSIZE 1
#define MAXTEAM 32
#define BUFSIZE 512

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
struct voteinfo votelist[MAXTEAM];
int make_vote(unsigned char *,int);
int select_fun();

int main(int argc, char **argv)
{
	int	sockfd;
	int port;
	int len;
	char *ms;
	unsigned char type;
	int num;
	int i;
	struct sockaddr_in	servaddr;

	if (argc != 3)
		perror("usage:<IPaddress> <port>");
		 if((port=atoi(argv[2]))<0){
    fprintf(stderr,"Usage %s port\n",argv[0]);
    exit(1);
    }

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	if(connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0){
	    perror("connect error\n");
 	    exit(1);}
    printf(" welcome!\n");
    printf("Please input \"vote Id\" to vote or \"inq id\" to inq or \"quit\" to quit\n");
    printf("input \"inq all\"to inq all teams\n");

while(1){
    memset(buf,0,BUFSIZE);
 	len=select_fun();

 	write(sockfd,buf,len);
 	memset(recevie,0,BUFSIZE);
 	read(sockfd,recevie,BUFSIZE);
 	type=recevie[0];
 	if(type==INQY_RES){
 	    num=(recevie[1]-3)/5;
 	    for(i=0;i<num;i++){
 	   votelist[i].id=recevie[3+i*5];
 	   votelist[i].num=recevie[4+i*5];+recevie[5+i*5]*16+recevie[6+i*5]*16*16+recevie[7+i*5]*16*16;
 	   printf("id=%d  num=%d\n",votelist[i].id,votelist[i].num);
 	   }
 	}
 	else
 	fputs(recevie,stdout);
}
}
int select_fun(){
   int i=0,j=0;
    int vote;
    unsigned char id[BUFSIZE]={0};
    unsigned char temp[5];
    unsigned char tempid;
    unsigned char teamid[BUFSIZE]={0};
    unsigned char *ms,*type;

 sss:
while(fgets(teamid,BUFSIZE,stdin)!=NULL){
if(!strcmp(teamid,"quit\n")){
printf("bye,thanks!\n");
exit(1);
}
type=strtok(teamid," ");
if(!strcmp(type,"vote"))
vote=1;
else if(!strcmp(type,"inq"))
vote=0;
else {
printf("error input\n");
goto sss;
}

while(ms=strtok(NULL," ")){
    if(!vote && !strcmp(ms,"all\n"))
    {
        for(i=1;i<=MAXTEAM;i++)
        id[i-1]=i;
        goto ssss;
    }
memset(temp,0,sizeof(temp));
tempid=atoi(strncpy(temp,ms,strlen(ms)));
if(vote && i>0){
printf("ONLY VOTE TO ONE TEAM \n");
     i=0;
    goto sss;
}
if(tempid>32 || tempid<1)
{
    printf("error id\n");
    for(j=0;j<=i;j++)
    id[j]=0;
    i=0;
    goto sss;
}
id[i]=tempid;
i++;
}
ssss:
 return make_vote(id,vote);
}
}
int make_vote(unsigned char *id,int vote){
struct vote v;
unsigned char type;
int num;

num=strlen(id);
if(vote)
type=VOTE_REQ;
else
type=INQY_REQ;
memset(buf,0,BUFSIZE);
*(unsigned char *)&v=type;
v.len=HSIZE+IDSIZE*num;
memcpy(buf,&v,HSIZE);
if(num)
memcpy(buf+HSIZE,id,IDSIZE*num);
return v.len;
}
