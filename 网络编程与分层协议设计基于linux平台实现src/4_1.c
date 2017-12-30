#include<stdio.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
	
int main(int argc,char *argv[]){
	struct in_addr ip_net;
	if(!inet_aton("192.168.9.1",&ip_net))
		printf("inet_aton error\n");
	printf("The net ID is: %x\n",inet_netof(ip_net));
	
	return 0;
}
