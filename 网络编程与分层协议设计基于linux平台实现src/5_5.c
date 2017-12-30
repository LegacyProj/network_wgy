#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include<stdlib.h>
#include<errno.h>
#include<netdb.h>
#define max(x,y) (x)>(y)? x:y
#define count 65535

int do_proxy(int fd,struct hostent *h,int port);                               /*代理服务处理函数*/
void sig_child(int signo);                                                     /*处理SIGCHLD信号*/

int main(int argc,char *argv[]){
    struct sockaddr_in servaddr, cliaddr;
    int listenfd, connfd;
    pid_t childpid;
    int clilen;
    int port1;
    int port2;
    struct hostent *host;

    if(argc!= 4){
    fprintf(stderr,"Usage:a.out <ipaddr> <port> <port>\n",argv[0]);
    exit(1);
     }

    if((port1=atoi(argv[3]))<0){
    fprintf(stderr,"Usage %s port\n",argv[0]);
    exit(1);
    }

    if((port2=atoi(argv[2]))<0){
    fprintf(stderr,"Usage %s port\n",argv[0]);
    exit(1);
    }

     if((host=gethostbyname(argv[1])) == NULL){
      perror("gethost name error");
    exit(1);
    }
      /*创建用于监听用户连接的套接字
       *当用户发来连接时，调用accept接受连接后，fork出子进程处理连接
       *父进程则继续监听
       *我们还调用sig_child函数处理SIGCHLD信号
     */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port1);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
      perror("socket error");
      exit(1);
    }

    if( bind(listenfd, (struct sockaddr *)(&servaddr), sizeof(servaddr)) ==-1 ) {
      perror("bind error");
      exit(1);
    }

    if( listen(listenfd, 5) < 0 ) {
     perror("listen error");
     exit(1);
    }

   signal(SIGCHLD, sig_child);                       /*处理SIGCHLD信号*/

   for(;;) {
    if((connfd = accept( listenfd, (struct sockaddr *)(&cliaddr),&clilen )) == -1){
     perror("accept error");
     exit(1);
    }

    if((childpid = fork()) == -1 ) {                /*创建子进程出错*/
     perror("fork error");
     exit(1);
     }
    if(childpid==0){                               /*子进程调用do_proxy函数处理proxy服务 */
     close(listenfd);
     do_proxy(connfd,host,port2);
     exit(0);
    }

    if(childpid>0){                               /*父进程继续监听*/
    close(connfd);
    }
  }
  exit(0);
}
 /* proxy函数
  *该函数处理两个套接口，用select选取活跃的套接字进行处理，
  *当客服端套接口描述字fd可读时，我们从fd中读取数据到buf，并发送到服务器
  *当服务器套接口描述字sockfd可读时，我们从sockfd中读取数据到buf，并发送到客服端
*/
 int do_proxy(int fd,struct hostent * h,int port){
   struct sockaddr_in addr;
   int sockfd;
   int maxfd;
   int n;
   fd_set set;
   char buf[count];

   bzero(&addr, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_port = htons(port);
   addr.sin_addr = *((struct in_addr *)h->h_addr);

   if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
   perror("socket error");
   exit(0);
   }

   if(connect(sockfd, (struct sockaddr *)(&addr), sizeof(addr)) < 0 ) {
    perror("connect error");
    exit(0);
}

   while(1) {
   FD_ZERO(&set);
   FD_SET(fd, &set);
   FD_SET(sockfd, &set);
   maxfd = max(sockfd, fd);

   if(select(maxfd + 1, &set, NULL, NULL, NULL) < 0 ) {
   perror("select error:");
   exit(0);
   }

    /* 客服端套接口可读  */
   if( FD_ISSET(fd, &set) ) {
   n = read(fd, (void *)buf, count);
   if( n <= 0)
   break;
   if( write(sockfd, (const void *)buf, n) != n ) {
   printf("write error");
   continue;
   }
}

   /* 服务器套接口可读 */
   if( FD_ISSET(sockfd, &set) ) {
   n = read(sockfd, (void *)buf, count);
   if( n <= 0)
   break;
   if( write(fd, (const void *)buf, n) != n ) {
   printf("write error");
   continue;
     }
   }
}
close(fd);
close(sockfd);
}
   /*
    *信号处理函数
   */

  void sig_child(int signo) {
  int status;
  pid_t pid;
  if((pid = waitpid(-1, &status, WNOHANG)) < 0 ) {
  printf("wait error");
  exit(1);
  }
  printf("child %d quitted", pid);
  return;
}
