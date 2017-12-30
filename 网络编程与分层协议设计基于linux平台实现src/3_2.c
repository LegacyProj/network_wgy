#include<stdio.h>
#include<stdlib.h>
	
int main()
{
	union {
		int m;
		char c[sizeof(int)];
	}un;
	
	un.m=0x0A170E06;
	if(sizeof(int)==4)
	{
		if(un.c[0]==0x0A)
			printf("the cpu is big-endian\n");
		else if(un.c[0]=0x06)
			printf("the cpu is little-endian\n");
		else
			printf("unknown\n");
		printf("%x %x %x %x\n",un.c[0],un.c[1],un.c[2],un.c[3]);
	}
	else 
		printf("sizeof(short)=%d\n",sizeof(int));
	exit(0);
}
