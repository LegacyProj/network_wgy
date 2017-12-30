#include<stdio.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
	
int main(int argc,char *argv[]){
	struct in_addr ip_net,in;
	if(!inet_aton("172.168.9.1",&ip_net))
		printf("inet_aton error\n");
	printf("The host ID is:%08x\n",inet_lnaof(ip_net));
	
	return 0;
}
