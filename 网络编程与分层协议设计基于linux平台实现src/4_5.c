#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<netdb.h>
#include<netinet/in.h>
#include<arpa/inet.h>
	
int main(int argc,char *argv[]){
	struct hostent *pt;
	char **ptr;
	char str[INET_ADDRSTRLEN];
	if(argc != 2){
		printf("input adress!\n");
		exit(1);
	}
	if((pt=gethostbyname(argv[1]))==NULL)
		printf("gethostname error\n");
	printf("official hostname: %s\n",pt->h_name);
	ptr=pt->h_aliases;
	printf("alias hostname: %s\n",*ptr);
	switch(pt->h_addrtype){
	case AF_INET:
    	 ptr=pt->h_addr_list;
    	 for(;*ptr!=NULL;ptr++)
       		printf("IP addrss: %s\n",inet_ntop(pt->h_addrtype,*ptr,str,sizeof(str)));
		break;
	default:
		printf("unknown address type");
		break;
	}
	
	return 0;
}

