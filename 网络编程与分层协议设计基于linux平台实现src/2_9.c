#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
	
int main(int argc,char **argv)
{
	FILE * fp;
	long int file_size;
	if(argc!=2){
		fprintf(stderr,"Usage: %s  filename\n",argv[0]);
		exit(1);
	}
	if((fp=fopen(argv[1],"r"))==NULL){
		fprintf(stderr,"open file %s error:%s\n",argv[1],strerror(errno));
		exit(1);
	}
	if(fseek(fp,0,SEEK_END)==-1){
		fprintf(stderr,"seek file %s error:%s\n",argv[1],strerror(errno));
		exit(1);
	}
	if((file_size=ftell(fp))==-1){
		fprintf(stderr,"get filesize  error:%s\n",strerror(errno));
		exit(1);
	}
	printf("the size of file  %s  is  %d(in bytes)\n",argv[1],file_size);
	exit(0);
}
