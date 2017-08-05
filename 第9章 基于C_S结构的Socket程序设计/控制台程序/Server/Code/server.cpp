// sever.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "iostream.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "WinSock2.h"
#include "Windows.h"
#include "server.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma pack(1)							//结构在存储时按字节对齐
long TcpClientCount = 0;

int main(int argc, char* argv[])
{
	//检查命令行参数
	if(argc != 2)
	{
		cerr << "Worng format!\nCorrect usage: Server.exe <TCP server port>";
		return -1;
	}

	//初始化winsock2环境
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cerr << "\nFailed to initialize the winsock 2 stack\n"
			 << "error code: " << WSAGetLastError() << endl;
		return -1;
	}

	//创建UDP服务器
	DWORD dwThreadId;
	CreateThread(NULL, 0, UdpServer, NULL, 0, &dwThreadId);

	//创建用于侦听的TCP Server Socket
	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, 0);	

	//获取TCP监听端口号
	u_short ListenPort = (u_short)atoi(argv[1]);
	//获取本机名
	char hostname[256];
	gethostname(hostname, sizeof(hostname));
	//获取本地IP地址
	hostent *pHostent = gethostbyname(hostname);
	//填充本地TCP Socket地址结构
	SOCKADDR_IN ListenAddr;
	memset(&ListenAddr, 0, sizeof(SOCKADDR_IN));
	ListenAddr.sin_family = AF_INET;
	ListenAddr.sin_port = htons(ListenPort);
	ListenAddr.sin_addr = *(in_addr*)pHostent->h_addr_list[0];

	//绑定TCP端口
	if (bind(ListenSocket, (sockaddr*)&ListenAddr, sizeof(ListenAddr)) == SOCKET_ERROR)
	{
		cerr << "\nFailed to bind the ListenSocket\n"
			 << "error code: " << WSAGetLastError() << endl;
		return -1;
	}

	//监听
	if ((listen(ListenSocket, SOMAXCONN)) == SOCKET_ERROR)
	{
		cerr << "\nFailed to listen the ListenSocket\n"
			 << "error code: " << WSAGetLastError() << endl;
		return -1;
	}
	cout << "TCP Server Started On TCP Port: "<< ListenPort << endl << endl;

	SOCKET TcpSocket;
	SOCKADDR_IN TcpClientAddr;
	while (TRUE)
	{
		//接受客户端连接请求
		int iSockAddrLen = sizeof(sockaddr);
		if ((TcpSocket = accept(ListenSocket, (sockaddr*)&TcpClientAddr, &iSockAddrLen)) == SOCKET_ERROR)
		{		
			cerr << "\nFailed to accept the client TCP Socket\n"
				 << "error code: " << WSAGetLastError() << endl;
			return -1;
		}

		//TCP线程数达到上限，停止接受新的Client
		if (TcpClientCount >= MAX_CLIENT)
		{
			closesocket(TcpSocket);
			cout << "Connection from TCP client "<< inet_ntoa(TcpClientAddr.sin_addr) << ":" << ntohs(TcpClientAddr.sin_port) << " refused for max client num\n" << endl;
			continue;
		}

		cout << "Connection from TCP client "<< inet_ntoa(TcpClientAddr.sin_addr) << ":" << ntohs(TcpClientAddr.sin_port) << " accepted\n" << endl;

		TcpThreadParam Param;
		Param.socket = TcpSocket;
		Param.addr = TcpClientAddr;

		//创建TCP服务线程
		DWORD dwThreadId;
		CreateThread(NULL, 0, TcpServeThread, &Param, 0, &dwThreadId);
		InterlockedIncrement(&TcpClientCount);
		
		cout << "Current Number of TCP Clients: " << TcpClientCount << '\n' << endl;
	}

	closesocket(ListenSocket);
	WSACleanup();
	return 0;
}

//TCP服务线程
DWORD WINAPI TcpServeThread(LPVOID lpParam)
{
	char ServerTCPBuf[MAX_BUF_SIZE];

	//获取线程参数
	SOCKET TcpSocket = ((TcpThreadParam*)lpParam)->socket;
	SOCKADDR_IN TcpClientAddr = ((TcpThreadParam*)lpParam)->addr;

	//输出提示信息
	cout<<"Thread: "<< GetCurrentThreadId() << " is serving client from " << inet_ntoa(TcpClientAddr.sin_addr) << ":" << ntohs(TcpClientAddr.sin_port) << endl << endl;

	//发送端口号+"START"
	sprintf(ServerTCPBuf, "%5d%s", UDPSrvPort, START_CMD);
	send(TcpSocket, ServerTCPBuf, strlen(ServerTCPBuf), 0);
	cout << "Waiting for command from Client(s)..." << endl << endl;

	int TCPBytesReceived;
	time_t CurSysTime;
	while (TRUE)
	{
		//读取client发来的请求命令: "GET CUR TIME"
		memset(ServerTCPBuf, '\0', sizeof(ServerTCPBuf));
		TCPBytesReceived = recv(TcpSocket, ServerTCPBuf, sizeof(ServerTCPBuf), 0);

		//TCPBytesReceived值为0表示client端已正常关闭连接
		//TCPBytesRecieved值为SOCKET_ERROR则表示socket的状态不正常,无法继续数据通讯
		//两种情况下都表明本线程的任务已结束，需要退出
		if (TCPBytesReceived == 0 || TCPBytesReceived == SOCKET_ERROR)
		{
			cout << "Client from " << inet_ntoa(TcpClientAddr.sin_addr) << ":" << ntohs(TcpClientAddr.sin_port) << " disconnected. Thread: " << GetCurrentThreadId() <<" is ending" << endl << endl;
			break;
		}

		//检查收到的字符串是否为命令:"GET CUR TIME"
		if (strcmp(ServerTCPBuf, GETCURTIME_CMD) != 0)
		{
			cout << "Unknowm command from Client " << inet_ntoa(TcpClientAddr.sin_addr) << endl << endl;
			continue;
		}

		cout << "Request for Current time from client " << inet_ntoa(TcpClientAddr.sin_addr) << ":" << ntohs(TcpClientAddr.sin_port) << " by TCP" << endl << endl;

		//获取系统时间并发送给client
		time(&CurSysTime);
		memset(ServerTCPBuf, '\0', sizeof(ServerTCPBuf));
		strftime(ServerTCPBuf, sizeof(ServerTCPBuf), "%Y-%m-%d %H:%M:%S", localtime(&CurSysTime));
		send(TcpSocket, ServerTCPBuf, strlen(ServerTCPBuf), 0);
		cout << "Server Current Time: " << ServerTCPBuf << '\n' << endl;
	}
		
	InterlockedDecrement(&TcpClientCount);
	closesocket(TcpSocket);
	return 0;
}


//UDP服务器线程
DWORD WINAPI UdpServer(LPVOID lpParam)
{	
	char ServerUDPBuf[MAX_BUF_SIZE];		//UDP Buffer
	SOCKADDR_IN UDPClientAddr;              //存储Client的地址信息

	//创建UDP server socket
	SOCKET UDPSrvSocket = socket(AF_INET, SOCK_DGRAM, 0);

	//获取本机名
	char hostname[256];
	gethostname(hostname, sizeof(hostname));
	//获取本地IP地址
	hostent *pHostent = gethostbyname(hostname);
	//填充本地UDP Socket地址结构
	SOCKADDR_IN UDPSrvAddr;
	memset(&UDPSrvAddr, 0, sizeof(SOCKADDR_IN));
	UDPSrvAddr.sin_family = AF_INET;
	UDPSrvAddr.sin_port = htons(UDPSrvPort);
	UDPSrvAddr.sin_addr = *(in_addr*)pHostent->h_addr_list[0];

	//绑定UDP端口
	if (bind(UDPSrvSocket, (sockaddr*)&UDPSrvAddr, sizeof(UDPSrvAddr)) == SOCKET_ERROR )
	{
		cerr << "bind() failed for UDPSrvSocket\n"
			 << "error code: " << WSAGetLastError() << endl;
		return -1;
	}

	cout<<"UDP Server Started On UDP Port: " << UDPSrvPort << endl << endl;

	while (TRUE)
	{
		memset(ServerUDPBuf, '\0', sizeof(ServerUDPBuf));

		//接收UDP请求
		int iSockAddrLen = sizeof(sockaddr);
		if ((recvfrom(UDPSrvSocket, ServerUDPBuf, sizeof(ServerUDPBuf), 0, (sockaddr*)&UDPClientAddr, &iSockAddrLen)) == SOCKET_ERROR)
		{
			cerr << "recvfrom() failed for UDPSrvSocket\n"
				 << "error code: " << WSAGetLastError() << endl;
			continue;
		}

		cout << "Client Command: Echo\n\n";

		cout << "\"" << ServerUDPBuf<< "\"" << " received from " << inet_ntoa(UDPClientAddr.sin_addr) << ":" << ntohs(UDPClientAddr.sin_port) << " by UDP" << endl << endl;

		//ECHO
		iSockAddrLen = sizeof(sockaddr);
		if ((sendto(UDPSrvSocket, ServerUDPBuf, strlen(ServerUDPBuf), 0, (sockaddr*)&UDPClientAddr, iSockAddrLen)) == SOCKET_ERROR )
		{
			cerr << "sendto() failed for UDPSrvSocket\n"
				 << "error code: " << WSAGetLastError() << endl;
			continue;
		}

		cout << "Echo " << "\"" << ServerUDPBuf << "\"" << " to clinet " << inet_ntoa(UDPClientAddr.sin_addr) << ":" <<ntohs(UDPClientAddr.sin_port) << " by UDP" << endl << endl;
	}
	return 0;
}
