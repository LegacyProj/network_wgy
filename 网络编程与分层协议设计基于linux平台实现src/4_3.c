#include<stdio.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
	
int main(int argc,char *argv[]){
	struct in_addr inet_addr;

	if(!inet_aton("172.168.9.1", &inet_addr))
		printf("inet_aton error\n");
	printf("The host ID is:%08x\n", htonl(inet_addr.s_addr & 0xFFFF0000));
	
	return 0;
}
