#include "IPMonitor.h"

#define BURRER_SIZE 65535

void main(int argc,char * argv[])
{
	// 判断输入的命令行格式是否正确
	if (argc != 2)
	{
		cout << "请按以下格式输入命令行: IPMonitor duration_time"
			<< endl << "    其中duration_time为监控时间, 单位为秒"<<endl;
		return;
	}

	WSADATA wsData;

	// 初始化Winsock DLL
	if (WSAStartup(MAKEWORD(2,2),&wsData) != 0)
	{
		cout << "WSAstartup failed!" << endl;
		return;
	}

	// 创建Raw Socket
	SOCKET sock;
	if ( (sock = WSASocket(AF_INET, SOCK_RAW, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED)) 
		== INVALID_SOCKET )
	{
		cout << "Create socket failed!" << endl;
		return;
	}

	// 设置IP头操作选项，表示用户可以亲自对IP头进行处理
	BOOL bFlag = TRUE;
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char *)&bFlag, sizeof(bFlag)) == SOCKET_ERROR)
	{
		cout << "Setsockopt failed!" << endl;
		return;
	}

	// 获取本地主机名
	char pHostName[128];
	if (gethostname(pHostName, 100) == SOCKET_ERROR)
	{
		cout << "Gethostname failed!" << endl;
		return;
	}

	// 通过本地主机名获取本地IP地址
	hostent * pHostIP;
	if((pHostIP = gethostbyname(pHostName)) == NULL)
	{
		cout<<"Gethostbyname failed!"<<endl;
		return;
	}

	// 填充sockaddr_in结构
	sockaddr_in addr_in;
	addr_in.sin_addr = *(in_addr *)pHostIP->h_addr_list[0];		 // 设定IP地址
	addr_in.sin_family = AF_INET;			// 设定地址类型
	addr_in.sin_port = htons(8000);			// 设定端口

	// 把原始套接字绑定到本机地址上
	if(bind(sock,(PSOCKADDR)&addr_in,sizeof(addr_in)) == SOCKET_ERROR)
	{
		cout << "Bind failed!" << endl;
		return;
	}

	// 把网卡设置为混杂模式，以便接收所有的IP包
#define  IO_RCVALL _WSAIOW(IOC_VENDOR,1)
	unsigned long pBufferLen[10];
	unsigned long dwBufferInLen = 1;
	unsigned long dwBytesReturned = 0;
	if ((WSAIoctl(sock, IO_RCVALL, &dwBufferInLen, sizeof(dwBufferInLen), &pBufferLen,
		sizeof(pBufferLen), &dwBytesReturned, NULL, NULL)) == SOCKET_ERROR)
	{
		cout<<"Ioctlsocket failed!"<<endl;
		return;
	}
	
	// 把socket设置为非阻塞模式
	unsigned long dwTemp = 1;
    ioctlsocket(sock, FIONBIO, &dwTemp);

	// 设置接收缓冲区
	char pBuffer[BURRER_SIZE];

	// 定义存放IP数据包的链表
	CNodeList IpList;

	double dwDuration = atof(argv[1]);	// 输入参数为捕获时间
	time_t beg;
	time_t end;
	time(&beg);		// 获得当前系统时间

	// 输出本地IP地址
	cout << endl;
	cout << "本机IP:" 
		 << inet_ntoa(*(in_addr *)&(addr_in.sin_addr.S_un.S_addr)) << endl << endl;
	cout << "开始捕获..." << endl << endl;

	while (1)
	{		
		time(&end);			// 获得当前系统时间
		//如果捕获时间到，就结束捕获
		if (end-beg >= dwDuration)
		{
			break;
		}

		// 捕获经过网卡的IP数据包
		int nPacketSize = recv(sock,pBuffer,BURRER_SIZE,0);
		if (nPacketSize > 0)
		{
			IPHEADER * pIpHdr;
			// 通过指针把缓冲区中的内容强制转换为IPHEADER数据结构
			pIpHdr = (IPHEADER *)pBuffer;
			// 判断IP包的源IP地址或目的IP地址是否为本地主机的IP地址
			if (pIpHdr->SourceAddress == addr_in.sin_addr.S_un.S_addr 
				|| pIpHdr->DestAddress == addr_in.sin_addr.S_un.S_addr) 
			{
				// 如果源IP地址或目的IP地址是本机IP，则将该IP数据包加入链表
				IpList.addNode(pIpHdr->SourceAddress, pIpHdr->DestAddress, pIpHdr->Protocal);
			}
		}
	}

	// 输出统计结果
	cout << "IP数据包统计结果: (" << dwDuration << " 秒)"<< endl << endl;
	IpList.print(cout);
	cout << endl;

	return;
}