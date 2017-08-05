// stdafx.cpp : 只包括标准包含文件的源文件
// CsmaCaEmluator.pch 将成为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中
//引用任何所需的附加头文件，而不是在此文件中引用

	
DWORD dwBus;

DWORD WINAPI HostA(LPVOID)
{
	int i=0;
	int nSendTimes=0;
	while (i!=10)
	{
Wait:
		while(dwBus!=0)
		{
			Sleep(100);
		}
		Sleep(rand()%100);
		if(dwBus!=0)
		{
			goto Wait;
		}
		dwBus=SEND +i;
		Sleep(1000);
		if(dwBus==ACK+i)
		{
			printf("发送第%d个数据包成功。\r\n",i+1);
			i++;
			nSendTimes=0;
			Sleep(1000);
		}
		else
		{
			nSendTimes++;
			if(nSendTimes<10)
			{
				printf("未能收到第%d个数据包的ACK响应，第%d次重新发送。\r\n",i+1,nSendTimes);
				int nSleepTime=(rand()%3)*(int)pow(2,nSendTimes);
			
				Sleep(nSleepTime*50);//随机等待一段时间后继续发送
			}
			else
			{
				printf("放弃发送第%d个数据包。\r\n",i+1);
				i++;
			}
		}
	}
	return 0;
}
DWORD WINAPI HostB(LPVOID)
{
	do
	{
		while((dwBus&0xfffffff0)!=SEND)
		{
			Sleep(10);
		}
		dwBus=ACK+(dwBus&0x0000000f);
	}while(dwBus!=SEND+9);
	return 0;
}
