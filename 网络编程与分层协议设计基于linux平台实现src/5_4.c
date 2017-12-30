#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#define BUFSIZE 512
#define max(x,y) (x)>(y)? x:y
void do_all(FILE *,int);//处理函数
int
main(int argc, char **argv)
{
	int	sockfd;
	struct sockaddr_in	servaddr;

	if (argc != 2)
		perror("usage: tcpcli <IPaddress>");

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(7);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	do_all(stdin, sockfd);		/*  调用处理函数*/

	exit(0);
}
  void do_all(FILE *fp,int sockfd){
  int maxfd1,flags,stdin_flags;
   ssize_t n1,n2;
  fd_set rset,wset;
/*两个缓冲区
 *read_buffer容纳从标准输入到服务器去的数据
 *recv_buffer容纳从服务器到标准输出来的数据
*/
  char send_buffer[BUFSIZE],recv_buffer[BUFSIZE];
/*以下定义四个指针；
 *  *sdiptr指针指向从标准输入读入数据可以存放的下一个字节；
 *  *sdoptr指向下一个必须写到套接口的字节。
 *  *rviptr指向从套接口读入的数据可以存放的下一个字节
 *   *rvoptr指向下一个必须写到标准输出的字节
*/

       char *sdiptr,*sdoptr,*rviptr,*rvoptr;
/*
设置套接字于非阻塞模式
*/
       flags= fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

	flags = fcntl(STDOUT_FILENO, F_GETFL, 0);
	fcntl(STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);
    /*初始化缓存指针*/

	sdiptr = sdoptr = send_buffer;
	rviptr = rvoptr = recv_buffer;
	stdin_flags = 0;

	maxfd1 = max(max(STDIN_FILENO, STDOUT_FILENO), sockfd) + 1;//选取描述字中最大的一个，加1之后作为select的第一个参数
	while(1) {
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		if (stdin_flags == 0 && sdiptr < &send_buffer[BUFSIZE])
			FD_SET(STDIN_FILENO, &rset);	       /* 从标准输入读 */
		if (rviptr < &recv_buffer[BUFSIZE])
			FD_SET(sockfd, &rset);			/* 从套接口读 */
		if (sdoptr != sdiptr)
			FD_SET(sockfd, &wset);			/* 写数据到套接口 */
		if (rvoptr != rviptr)
			FD_SET(STDOUT_FILENO, &wset);	       /* 写数据到标准输出 */

		select(maxfd1, &rset, &wset, NULL, NULL);


		if (FD_ISSET(STDIN_FILENO, &rset)) {             /*标准输入可读*/
			if ( (n1 = read(STDIN_FILENO, sdiptr, &send_buffer[BUFSIZE] - sdiptr)) < 0) {
				if (errno != EWOULDBLOCK)
					perror("read error on stdin");

			} else if (n1 == 0) {

				fprintf(stderr, " EOF on stdin\n");

				stdin_flags = 1;			/* 标准输入处理完 */
				if (sdoptr == sdiptr)
					shutdown(sockfd, SHUT_WR);  /* 发送FIN*/

			} else {

				//fprintf(stderr, "read %d bytes from stdin\n",n1);

				sdiptr += n1;			       /* 从标准输入读入n1个字节数据后sdiptr向前移动n */
				FD_SET(sockfd, &wset);	      /* 打开描述字集中与sockfd相应的位 */
			}
		}

		if (FD_ISSET(sockfd, &rset)) {                 /* 套接口可读   */
			if ( (n1 = read(sockfd, rviptr, &recv_buffer[BUFSIZE] - rviptr)) < 0) {
				if (errno != EWOULDBLOCK)
					perror("read error on socket");

			} else if (n1== 0) {

				fprintf(stderr, "EOF on socket\n");

				if (stdin_flags)
					return;		/* 正常退出*/
				else
					perror("do_all: server terminated prematurely");

			} else {

				//fprintf(stderr, "read %d bytes from socket\n",n1);
				rviptr += n1;		        /* 从套接口读n1个字节*/
				FD_SET(STDOUT_FILENO, &wset);	/* 打开描述字集中与标准输出相应的位*/
			}
		}

		if (FD_ISSET(STDOUT_FILENO, &wset) && ( (n1 = rviptr - rvoptr) > 0)) {   /* 标准输出可写*/
			if ( (n2 = write(STDOUT_FILENO, rvoptr, n1)) < 0) {
				if (errno != EWOULDBLOCK)
					perror("write error to stdout");

			} else {

				//fprintf(stderr, " wrote %d bytes to stdout\n",n2);

				rvoptr += n2;		/* 写n2个字节到标准输出 */
				if (rvoptr == rviptr)
					rvoptr = rviptr = recv_buffer;	/* 重置指针 */
			}
		}

		if (FD_ISSET(sockfd, &wset) && ( (n1 = sdiptr - sdoptr) > 0)) {    /*套接口可写*/
			if ( (n2 = write(sockfd, sdoptr, n1)) < 0) {
				if (errno != EWOULDBLOCK)
					perror("write error to socket");

			} else {

				//fprintf(stderr, " wrote %d bytes to socket\n",n2);

				sdoptr += n2;	/*  写n2个字节到套接口*/
				if (sdoptr == sdiptr) {
					sdiptr = sdoptr = send_buffer;	/* 重置指针 */
					if (stdin_flags)
						shutdown(sockfd, SHUT_WR);	/* 发送 FIN */
				}
			}
		}
	}
}

