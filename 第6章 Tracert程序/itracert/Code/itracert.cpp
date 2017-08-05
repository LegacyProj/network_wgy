/*----------------------------------------------------------
功能说明：该程序简单实现了Windows操作系统的tracert命令功能，
      可以输出IP报文从本机出发到达目的主机所经过的路由信息。
注意：程序编译时应使用1字节对齐方式调整边界!
-----------------------------------------------------------*/
#include <iostream.h>
#include <iomanip.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "itracert.h"

////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	//检查命令行参数
	if (argc != 2)
	{
		cerr << "\nUsage: itracert ip_or_hostname\n";
		return -1;
	}

	//初始化winsock2环境
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cerr << "\nFailed to initialize the WinSock2 DLL\n"
			 << "error code: " << WSAGetLastError() << endl;
		return -1;
	}

	//将命令行参数转换为IP地址
	u_long ulDestIP = inet_addr(argv[1]);
	if (ulDestIP == INADDR_NONE)
	{
		//转换不成功时按域名解析
		hostent* pHostent = gethostbyname(argv[1]);
		if (pHostent)
		{
			ulDestIP = (*(in_addr*)pHostent->h_addr).s_addr;

			//输出屏幕信息
			cout << "\nTracing route to " << argv[1] 
				 << " [" << inet_ntoa(*(in_addr*)(&ulDestIP)) << "]"
				 << " with a maximum of " << DEF_MAX_HOP << " hops.\n" << endl;
		}
		else //解析主机名失败
		{
			cerr << "\nCould not resolve the host name " << argv[1] << '\n'
				 << "error code: " << WSAGetLastError() << endl;
			WSACleanup();
			return -1;
		}
	}
	else
	{
		//输出屏幕信息
		cout << "\nTracing route to " << argv[1] 
			 << " with a maximum of " << DEF_MAX_HOP << " hops.\n" << endl;
	}

	//填充目的Socket地址
	sockaddr_in destSockAddr;
	ZeroMemory(&destSockAddr, sizeof(sockaddr_in));
	destSockAddr.sin_family = AF_INET;
	destSockAddr.sin_addr.s_addr = ulDestIP;

	//使用ICMP协议创建Raw Socket
	SOCKET sockRaw = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (sockRaw == INVALID_SOCKET)
	{
		cerr << "\nFailed to create a raw socket\n"
			 << "error code: " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}
	//设置端口属性
	int iTimeout = DEF_ICMP_TIMEOUT;

	if (setsockopt(sockRaw, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeout, sizeof(iTimeout)) == SOCKET_ERROR)
	{
		cerr << "\nFailed to set recv timeout\n"
			 << "error code: " << WSAGetLastError() << endl;
		closesocket(sockRaw);
		WSACleanup();
		return -1;
	}
	if (setsockopt(sockRaw, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeout, sizeof(iTimeout)) == SOCKET_ERROR)
	{
		cerr << "\nFailed to set send timeout\n"
			 << "error code: " << WSAGetLastError() << endl;
		closesocket(sockRaw);
		WSACleanup();
		return -1;
	}


	//创建ICMP包发送缓冲区和接收缓冲区
	char IcmpSendBuf[sizeof(ICMP_HEADER)+DEF_ICMP_DATA_SIZE];
	memset(IcmpSendBuf, 0, sizeof(IcmpSendBuf));
	char IcmpRecvBuf[MAX_ICMP_PACKET_SIZE];
	memset(IcmpRecvBuf, 0, sizeof(IcmpRecvBuf));

	//填充待发送的ICMP包
	ICMP_HEADER* pIcmpHeader = (ICMP_HEADER*)IcmpSendBuf;
	pIcmpHeader->type = ICMP_ECHO_REQUEST;
	pIcmpHeader->code = 0;
	pIcmpHeader->id = (USHORT)GetCurrentProcessId();
	memset(IcmpSendBuf+sizeof(ICMP_HEADER), 'E', DEF_ICMP_DATA_SIZE);

	//开始探测路由
	DECODE_RESULT stDecodeResult;
	BOOL bReachDestHost = FALSE;
	USHORT usSeqNo = 0;
	int iTTL = 1;
	int iMaxHop = DEF_MAX_HOP;
	while (!bReachDestHost && iMaxHop--)
	{
		//设置IP数据报头的ttl字段
		setsockopt(sockRaw, IPPROTO_IP, IP_TTL, (char*)&iTTL, sizeof(iTTL));

		//输出当前跳站数作为路由信息序号
		cout << setw(3) << iTTL << flush;

		//填充ICMP数据报剩余字段
		((ICMP_HEADER*)IcmpSendBuf)->cksum = 0;
		((ICMP_HEADER*)IcmpSendBuf)->seq = htons(usSeqNo++);
		((ICMP_HEADER*)IcmpSendBuf)->cksum = GenerateChecksum((USHORT*)IcmpSendBuf, sizeof(ICMP_HEADER)+DEF_ICMP_DATA_SIZE);
		
		//记录序列号和当前时间
		stDecodeResult.usSeqNo = ((ICMP_HEADER*)IcmpSendBuf)->seq;
		stDecodeResult.dwRoundTripTime = GetTickCount();
		
		//发送ICMP的EchoRequest数据报
		if (sendto(sockRaw, IcmpSendBuf, sizeof(IcmpSendBuf), 0, 
				   (sockaddr*)&destSockAddr, sizeof(destSockAddr)) == SOCKET_ERROR)
		{
			//如果目的主机不可达则直接退出
			if (WSAGetLastError() == WSAEHOSTUNREACH)
				cout << '\t' << "Destination host unreachable.\n" 
					 << "\nTrace complete.\n" << endl;
			closesocket(sockRaw);
			WSACleanup();
			return 0;
		}

		//接收ICMP的EchoReply数据报
		//因为收到的可能并非程序所期待的数据报，所以需要循环接收直到收到所要数据或超时
		sockaddr_in from;
		int iFromLen = sizeof(from);
		int iReadDataLen;
		while (1)
		{
			//等待数据到达
			iReadDataLen = recvfrom(sockRaw, IcmpRecvBuf, MAX_ICMP_PACKET_SIZE, 
									0, (sockaddr*)&from, &iFromLen);
			if (iReadDataLen != SOCKET_ERROR) //有数据包到达
			{
				//解码得到的数据包，如果解码正确则跳出接收循环发送下一个EchoRequest包
				if (DecodeIcmpResponse(IcmpRecvBuf, iReadDataLen, stDecodeResult))
				{
					if (stDecodeResult.dwIPaddr.s_addr == destSockAddr.sin_addr.s_addr)
						bReachDestHost = TRUE;

					cout << '\t' << inet_ntoa(stDecodeResult.dwIPaddr) << endl;
					break;
				}
			}
			else if (WSAGetLastError() == WSAETIMEDOUT) //接收超时，打印星号
			{
				cout << setw(9) << '*' << '\t' << "Request timed out." << endl;
				break;
			}
			else
			{
				cerr << "\nFailed to call recvfrom\n"
					 << "error code: " << WSAGetLastError() << endl;
				closesocket(sockRaw);
				WSACleanup();
				return -1;
			}
		}

		//TTL值加1
		iTTL++;
	}
	//输出屏幕信息
	cout << "\nTrace complete.\n" << endl;

	closesocket(sockRaw);
	WSACleanup();
	return 0;
}

//产生网际校验和
USHORT GenerateChecksum(USHORT* pBuf, int iSize) 
{
	unsigned long cksum = 0;
	while (iSize>1) 
	{
		cksum += *pBuf++;
		iSize -= sizeof(USHORT);
	}
	if (iSize) 
		cksum += *(UCHAR*)pBuf;

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);

	return (USHORT)(~cksum);
}


//解码得到的数据报
BOOL DecodeIcmpResponse(char* pBuf, int iPacketSize, DECODE_RESULT& stDecodeResult)
{
	//检查数据报大小的合法性
	IP_HEADER* pIpHdr = (IP_HEADER*)pBuf;
	int iIpHdrLen = pIpHdr->hdr_len * 4;
	if (iPacketSize < (int)(iIpHdrLen+sizeof(ICMP_HEADER)))
		return FALSE;

	//按照ICMP包类型检查id字段和序列号以确定是否是程序应接收的Icmp包
	ICMP_HEADER* pIcmpHdr = (ICMP_HEADER*)(pBuf+iIpHdrLen);
	USHORT usID, usSquNo;
	if (pIcmpHdr->type == ICMP_ECHO_REPLY)
	{
		usID = pIcmpHdr->id;
		usSquNo = pIcmpHdr->seq;
	}
	else if(pIcmpHdr->type == ICMP_TIMEOUT)
	{
		char* pInnerIpHdr = pBuf+iIpHdrLen+sizeof(ICMP_HEADER);		//载荷中的IP头
		int iInnerIPHdrLen = ((IP_HEADER*)pInnerIpHdr)->hdr_len * 4;//载荷中的IP头长
		ICMP_HEADER* pInnerIcmpHdr = (ICMP_HEADER*)(pInnerIpHdr+iInnerIPHdrLen);//载荷中的ICMP头
		usID = pInnerIcmpHdr->id;
		usSquNo = pInnerIcmpHdr->seq;
	}
	else
		return FALSE;

	if (usID != (USHORT)GetCurrentProcessId() || usSquNo !=stDecodeResult.usSeqNo) 
		return FALSE;

	//处理正确收到的ICMP数据报
	if (pIcmpHdr->type == ICMP_ECHO_REPLY ||
		pIcmpHdr->type == ICMP_TIMEOUT)
	{
		//返回解码结果
		stDecodeResult.dwIPaddr.s_addr = pIpHdr->sourceIP;
		stDecodeResult.dwRoundTripTime = GetTickCount()-stDecodeResult.dwRoundTripTime;

		//打印屏幕信息
		if (stDecodeResult.dwRoundTripTime)
			cout << setw(6) << stDecodeResult.dwRoundTripTime << " ms" << flush;
		else
			cout << setw(6) << "<1" << " ms" << flush;

		return TRUE;
	}

	return FALSE;
}
