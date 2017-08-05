#include<iostream.h>
#include<fstream.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include "IPNode.h"
#include "NodeList.h"

#pragma comment(lib, "Ws2_32.lib")



// 定义IP头部
typedef struct IPHeader
{
	unsigned char	Version_HeaderLength;	// 版本(4位)+首部长度(4位)
	unsigned char 	TypeOfService;			// 服务类型
	unsigned short  TotalLength;			// 总长度
	unsigned short  Identification;			// 标识
	unsigned short  Flags_FragmentOffset;	// 标志(3位)+分片偏移(13位)
	unsigned char  	TimeToLive;				// 生存时间
	unsigned char  	Protocal;				// 协议
	unsigned short  HeaderChecksum;			// 首部校验和
	unsigned long  	SourceAddress;			// 源IP地址
	unsigned long  	DestAddress;			// 目的IP地址
}IPHEADER;