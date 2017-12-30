#include<stdio.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
	
int main(int argc,char *argv[]){
	unsigned long net_addr;

	net_addr=inet_network("192.168.9.1") & 0xFFFFFF00;
	printf("The net ID is:%x\n",net_addr);
	
	return 0;
}
