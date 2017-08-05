// client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "WinSock2.h"
#include "iostream.h"
#include "stdio.h"
#pragma comment (lib,"Ws2_32.lib")

#define MAX_BUF_SIZE 65535
char ClientBuf[MAX_BUF_SIZE];
const char START_CMD[]		= "START";
const char GETCURTIME_CMD[]	= "GET CUR TIME";

//输出用户选择界面
void UserPrompt()
{
	cout << "Input the corresponding Num to select what you want the program to do" << endl << endl
		 << "\t1. Get current time(TCP)" << endl
		 << "\t2. Echo Mode(UDP)" << endl
		 << "\t3. Exit the program" << endl << endl
		 << "Enter Your choice: ";
}

int main(int argc, char* argv[])
{
	unsigned short ServerUDPPort;
	
	SOCKET cTCPSocket,cUDPSocket;
	WSADATA wsadata;
	SOCKADDR_IN TCPServer,UDPServer,RecvFrom;
	int RecvFromLength=sizeof(RecvFrom);

	char UserChoice;
	char portnum[5];
	unsigned long BytesReceived,BytesSent;
	int RetValue;

	//检查命令行参数
	if (argc != 3)
	{
		cout<<"Worng format!"<<endl<<"Correct usage: Client.exe <TCP Server IP> <TCP Server Port>"<<endl;
		return 1;
	}

	u_long ServerIP = inet_addr(argv[1]);
	u_short ServerTCPPort = (u_short)atoi(argv[2]);

	
	//初始化winsock库
	if( ( RetValue=WSAStartup(MAKEWORD(2,2),&wsadata) ) !=0 )
	{
		printf("WSAStartup() failed with error %d\n", RetValue);
		return 2;
	}

	//创建TCP Socket
	if( (cTCPSocket=WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED) ) ==INVALID_SOCKET)
	{
		printf("WSASocket() for cTCPSocket failed with error %d\n" ,WSAGetLastError() );
		return 3;
	}

	//创建UDP Socket
	if( (cUDPSocket=WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0,WSA_FLAG_OVERLAPPED) ) ==INVALID_SOCKET)
	{
		printf("WSASocket() for cUDPSocket failed with error %d\n" ,WSAGetLastError() );
		return 4;
	}

	TCPServer.sin_family=AF_INET;
	TCPServer.sin_port=htons(ServerTCPPort);
	TCPServer.sin_addr.S_un.S_addr=ServerIP;

	//通过TCP Socket连接server
	if ((RetValue=WSAConnect(cTCPSocket,(sockaddr *)&TCPServer,sizeof(TCPServer),NULL,NULL,NULL,NULL) )==SOCKET_ERROR)
	{
		printf("WSAConnect() failed for cTCPSocket with error %d\n",WSAGetLastError() );
		printf("Can't connect to server.\n");
		return 5;
	}

	//与server建立连接后读取Server发送过来的Server UDP端口和"START"
	BytesReceived=recv(cTCPSocket,ClientBuf,sizeof(ClientBuf),0 );
	if (BytesReceived == 0 || BytesReceived == SOCKET_ERROR)
	{
		cout<<endl<<"Server refused the connection or recv failed"<<endl;
		return 6;
	}
	memcpy(portnum,ClientBuf,sizeof(portnum));
	ServerUDPPort=(u_short)atoi(portnum);

	if (strcmp(START_CMD,ClientBuf+5)!=0)
	{
		cout<<endl<<"Server did not return right beginning indicator"<<endl;
		return 6;
	}
	else
	{
		cout<<endl<<"OK, NOW the server is ready for your service!"<<endl<<endl;
	}

	UDPServer.sin_family=AF_INET;
	UDPServer.sin_port=htons(ServerUDPPort);
	UDPServer.sin_addr.S_un.S_addr=ServerIP;

	while(TRUE)
	{
	
		//输出提示信息
		UserPrompt();
		cin>>UserChoice;

		switch(UserChoice)
		{
		case '1'://通过TCP得到server的系统时间

			//发送命令
			memset(ClientBuf,'\0',sizeof(ClientBuf));
			sprintf(ClientBuf,"%s",GETCURTIME_CMD);
			if ((BytesSent=send(cTCPSocket,ClientBuf,strlen(ClientBuf),0) )==SOCKET_ERROR)
			{
				printf("send() failed for cTCPSocket with error %d\n",WSAGetLastError() );
				printf("Can not send command to server by TCP.Maybe Server is down.\n");
				return 7;
			}

			//读取server发来的系统时间并显示
			memset(ClientBuf,'\0',sizeof(ClientBuf) );
			if ((BytesReceived=recv(cTCPSocket,ClientBuf,sizeof(ClientBuf),0) )==SOCKET_ERROR)
			{
				printf("recv() failed for cTCPSocket with error %d\n",WSAGetLastError() );
				printf("Can not get server current systime.Maybe Maybe Server is down.\n");
				return 8;
			}
			cout<<"Server Current Time: "<<ClientBuf<<endl<<endl;
			break;

		case '2': //通过UDP实现ECHO

			//提示用户输入文本
			memset(ClientBuf,'\0',sizeof(ClientBuf) );
			cout<<"请输入任意文本信息，按回车键后将发送至Server."<<endl;
			gets(ClientBuf);

			//发送文本
			if ((BytesSent=sendto(cUDPSocket,ClientBuf,strlen(ClientBuf),0,(sockaddr *)&UDPServer,sizeof(UDPServer) ) ) ==SOCKET_ERROR)
			{	
				printf("sendto() failed for cUDPSocket with error %d\n",WSAGetLastError() );
				printf("Can not send message by UDP.Maybe Server is down.\n");
				return 9;
			}
				
			//读取ECHO
			memset(ClientBuf,'\0',sizeof(ClientBuf) );
			if ((BytesReceived=recvfrom(cUDPSocket,ClientBuf,sizeof(ClientBuf),0,(sockaddr *)&RecvFrom, &RecvFromLength ) ) ==SOCKET_ERROR)
			{	
				printf("recvfrom () failed for cUDPSocket with error %d\n",WSAGetLastError() );
				printf("Can't get Echo Reply from server by UDP.Maybe Server is down.\n");
				return 10;
			}

			//检查ECHO是否来自Server
			if(UDPServer.sin_addr.S_un.S_addr==RecvFrom.sin_addr.S_un.S_addr &&UDPServer.sin_port==RecvFrom.sin_port)
			{
				cout<<"Get Echo From Server: "<<ClientBuf<<endl<<endl;
			}
			else
			{
				cout<<"NO Echo From Server"<<endl;
			}
			break;

		case '3':  //释放资源，退出程序
			closesocket(cTCPSocket);
			closesocket(cUDPSocket);
			WSACleanup();
			cout<<"program exit"<<endl;
			return 10;
		default:
			cout<<"Wrong choice ,should be 1,2 or 3"<<endl;
		}
	}

	return 11;
}


