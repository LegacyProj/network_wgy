#include <Winsock2.h>
#include <iostream.h>

///////////////////////////////////////////////////////////////////////////////////////////
//These defines & structure definitions are taken from the 
//"ipexport.h" header file as provided with the Platform SDK. 
//Including them here allows you to compile the code without 
//the need to have the full Platform SDK installed.

typedef ULONG IPAddr;       // An IP address.

typedef struct ip_option_information {
    UCHAR   Ttl;                // Time To Live
    UCHAR   Tos;                // Type Of Service
    UCHAR   Flags;              // IP header flags
    UCHAR   OptionsSize;        // Size in bytes of options data
    UCHAR * OptionsData;        // Pointer to options data
} IP_OPTION_INFORMATION, *PIP_OPTION_INFORMATION;

typedef struct icmp_echo_reply {
    IPAddr  Address;            // Replying address
    ULONG   Status;             // Reply IP_STATUS
    ULONG   RoundTripTime;      // RTT in milliseconds
    USHORT  DataSize;           // Reply data size in bytes
    USHORT  Reserved;           // Reserved for system use
    PVOID   Data;               // Pointer to the reply data
    struct ip_option_information Options; // Reply options
} ICMP_ECHO_REPLY, *PICMP_ECHO_REPLY;

typedef HANDLE (WINAPI *lpIcmpCreateFile)(VOID);
typedef BOOL (WINAPI *lpIcmpCloseHandle)(HANDLE IcmpHandle);
typedef DWORD (WINAPI *lpIcmpSendEcho)(HANDLE IcmpHandle, IPAddr DestinationAddress,
                                       LPVOID RequestData, WORD RequestSize,
                                       PIP_OPTION_INFORMATION RequestOptions,
                                       LPVOID ReplyBuffer, DWORD ReplySize, DWORD Timeout);

///////////////////////////////////////////////////////////////////////////////////////////

const int DEF_MAX_HOP = 30;		//最大跳站数
const int DATA_SIZE = 32;		//ICMP包数据字段大小
const DWORD TIMEOUT= 3000;		//超时时间，单位ms

#pragma comment (lib,"Ws2_32.lib")

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
		cerr << "Failed to initialize the WinSock2 DLL\n"
			 << "error code: " << WSAGetLastError() << endl;
		return -1;
	}

	//将命令行参数转换为IP地址
	ULONG DestIp = inet_addr(argv[1]);
	if (DestIp == INADDR_NONE)
	{
		//转换不成功时按域名解析
		hostent* pHost = gethostbyname(argv[1]);
		if (pHost)
		{
			DestIp = (*(in_addr*)pHost->h_addr).s_addr;
		}
		else
		{
			cout << "Unable to resolve the target name " << argv[1] << endl;
			WSACleanup();
			return -1;
		}
	}
	
	//载入ICMP.DLL动态库
	HMODULE hInst = LoadLibrary("ICMP.dll");
	if (!hInst)
	{
		cout << "Could not load up the ICMP DLL\n";
		WSACleanup();
		return -1;
	}

	//获取所需的三个函数指针
	lpIcmpCreateFile IcmpCreateFile = (lpIcmpCreateFile)GetProcAddress(hInst, "IcmpCreateFile");
	lpIcmpSendEcho IcmpSendEcho = (lpIcmpSendEcho)GetProcAddress(hInst, "IcmpSendEcho");
	lpIcmpCloseHandle IcmpCloseHandle = (lpIcmpCloseHandle)GetProcAddress(hInst, "IcmpCloseHandle");
	if (IcmpCreateFile==NULL || IcmpSendEcho==NULL || IcmpCloseHandle==NULL)
	{
		cout << "Could not find ICMP functions in the ICMP DLL\n";
		WSACleanup();
		return -1;
	}

	//打开ICMP句柄
	HANDLE hIcmp = IcmpCreateFile(); 
	if (hIcmp == INVALID_HANDLE_VALUE)
	{
		cout << "Could not get a valid ICMP handle\n";
		WSACleanup();
		return -1;
	}

	cout << "\nTracing route to " << argv[1] << " [" << inet_ntoa(*(in_addr*)(&DestIp)) 
		 << "] with a maximum of " << DEF_MAX_HOP << " hops.\n" << endl;

	//设置IP报头TTL值
	IP_OPTION_INFORMATION IpOption; //该结构用来控制所发ICMP数据包的IP报头相应字段值
    ZeroMemory(&IpOption, sizeof(IP_OPTION_INFORMATION));
	IpOption.Ttl = 1;

	//设置要发送的ICMP数据
	char SendData[DATA_SIZE];
	memset(SendData, 'E', sizeof(SendData));
	
	//设置接收缓冲区
	char ReplyBuffer[sizeof(ICMP_ECHO_REPLY)+DATA_SIZE];
	ICMP_ECHO_REPLY* pEchoReply = (ICMP_ECHO_REPLY*)ReplyBuffer;

	BOOL bLoop = TRUE;
	int iMaxHop = DEF_MAX_HOP;
	while (bLoop && iMaxHop--)
	{
		//打印序号
		cout << (int)IpOption.Ttl << ":";

		//发送ICMP回显包并接收应答
		if (IcmpSendEcho(hIcmp, DestIp, SendData, sizeof(SendData), &IpOption, ReplyBuffer, sizeof(ReplyBuffer), TIMEOUT) != 0)
		{
			//正确收到应答包，打印时间和IP地址
			if (pEchoReply->RoundTripTime == 0)
			{
				cout << "\t<1 ms";
			}
			else
			{
				cout << "\t" << pEchoReply->RoundTripTime << " ms";
			}
			cout << "\t" << inet_ntoa(*(in_addr*)&(pEchoReply->Address)) << endl;

			//判断是否完成路由路径探测
			if (pEchoReply->Address == DestIp)
			{
				cout << "\nTrace complete.\n" << endl;
				bLoop = FALSE;
			}
		}
		else //超时返回,打印代表超时的"*"
		{
			cout << "\t" << "*" << "\tRequest timed out." << endl;
		}

		//TTL值加1
		IpOption.Ttl++;
	}
	IcmpCloseHandle(hIcmp);
	WSACleanup();
	return 0;
}
