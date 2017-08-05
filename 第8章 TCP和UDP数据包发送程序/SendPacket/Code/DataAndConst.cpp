#include "StdAfx.h"
#include ".\dataandconst.h"

USHORT CheckSum(const char *buf, int size) 
{ 
	USHORT *buffer=(USHORT *)buf;
	unsigned long cksum=0; 
	while(size >1) 
	{ 
		cksum+=*buffer++; 
		size -=sizeof(USHORT); 
	} 
	if(size ) 
	{ 
		cksum += *(UCHAR*)buffer; 
	} 

	cksum = (cksum >> 16) + (cksum & 0xffff); 
	cksum += (cksum >>16); 
	return (USHORT)(~cksum); 
} 
USHORT CheckSum(USHORT *buffer, int size) 
{ 
	unsigned long cksum=0; 
	while(size >1) 
	{ 
		cksum+=*buffer++; 
		size -=sizeof(USHORT); 
	} 
	if(size ) 
	{ 
		cksum += *(UCHAR*)buffer; 
	} 

	cksum = (cksum >> 16) + (cksum & 0xffff); 
	cksum += (cksum >>16); 
	return (USHORT)(~cksum); 
} 


unsigned short TcpCheckSum(const char *pTcpData, const char *pPshData, UINT nTcpCount)
{
	unsigned short sCheckSum = ~CheckSum(pTcpData,nTcpCount);
	unsigned long checkSum = sCheckSum;
	checkSum <<= 16;
	sCheckSum = ~CheckSum(pPshData,12);
	checkSum += sCheckSum;	

	return CheckSum((char*)&checkSum,4);
}
unsigned short UdpCheckSum(const char *pTcpData, const char *pPshData, UINT nTcpCount)
{
	unsigned short sCheckSum = ~CheckSum(pTcpData,nTcpCount);
	unsigned long checkSum = sCheckSum;
	checkSum <<= 16;
	sCheckSum = ~CheckSum(pPshData,12);
	checkSum += sCheckSum;	

	return CheckSum((char*)&checkSum,4);
}
