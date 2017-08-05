// SendPacket.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "dataandconst.h"
pcap_t * InitWinpcap()
{
	printf("Please Choose the Adaptor through which you send data:\r\n");
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int inum;
	int i=0;
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	
	/* Retrieve the device list */
	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}
	
	/* Print the list */
	for(d=alldevs; d; d=d->next)
	{
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}
	
	if(i==0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
		return NULL;
	}
	
	printf("Enter the interface number (1-%d):",i);
	scanf("%d", &inum);
	
	if(inum < 1 || inum > i)
	{
		printf("\nInterface number out of range.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return NULL;
	}
	
	/* Jump to the selected adapter */
	for(d=alldevs, i=0; i< inum-1 ;d=d->next, i++);
	
	/* Open the device */
	/* Open the adapter */
	if ((adhandle= pcap_open_live(d->name,	// name of the device
							 65536,			// portion of the packet to capture. 
											// 65536 grants that the whole packet will be captured on all the MACs.
							 1,				// promiscuous mode (nonzero means promiscuous)
							 1000,			// read timeout
							 errbuf			// error buffer
							 )) == NULL)
	{
		fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return NULL;
	}
	pcap_freealldevs(alldevs);
	return adhandle;
}
int _tmain(int argc, _TCHAR* argv[])
{
	if(3!=argc)
	{
		printf("Wrong Parament!\r\n");
		return 0;
	}
	//printf (argv[1]);
	DWORD dwDestIp=	inet_addr(argv[1]);
	if(dwDestIp==INADDR_NONE)
	{
		printf("Wrong Ip Address!\r\n");
		return 0;
	}
	if(strlen(argv[2])>1024)
	{
		printf("Too long Parament!\r\n");
		return 0;
	}

	pcap_t *hWpcapHandle=InitWinpcap();
	UCHAR bLocalMac[6];
	DWORD dwDefaultGateway= 0;
	DWORD dwLocalIP = 0;
	DWORD dwNetMask= 0;
	char strName[64];
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	ULONG ulLen = 0;

	gethostname(strName,64);
    ::GetAdaptersInfo(pAdapterInfo,&ulLen);
	pAdapterInfo = (PIP_ADAPTER_INFO)::GlobalAlloc(GPTR, ulLen);

	// 取得本地适配器结构信息
	if(::GetAdaptersInfo(pAdapterInfo,&ulLen) ==  ERROR_SUCCESS)
	{
		if(pAdapterInfo != NULL)
		{
			memcpy(bLocalMac, pAdapterInfo->Address, 6);
			dwDefaultGateway= ::inet_addr(pAdapterInfo->GatewayList.IpAddress.String);
			dwLocalIP = ::inet_addr(pAdapterInfo->IpAddressList.IpAddress.String);
			dwNetMask= ::inet_addr(pAdapterInfo->IpAddressList.IpMask.String);
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	char bDestMac[8];
	memset(bDestMac,0xff,6);

	TcpPacket *pTcpPacket;
	pTcpPacket=(TcpPacket *)new char[sizeof(TcpPacket)+strlen(argv[2])+1];
	strcpy(((char*)pTcpPacket)+sizeof(TcpPacket),argv[2]);
	
	ulLen=6;
	if(SendARP(dwDestIp,0,(PULONG)bDestMac,&ulLen)!=NO_ERROR)
	{
		printf("Get Mac Error!\r\n");
		return 0;
	}
	memcpy(pTcpPacket->theIpPacket.theEthHead.bDestMac,bDestMac,6);
	memcpy(pTcpPacket->theIpPacket.theEthHead.bSourceMac,bLocalMac,6);
	pTcpPacket->theIpPacket.theEthHead.usEthernetType=0x8;

	pTcpPacket->theIpPacket.theIpHead.ucVersionAndHeadLength=0x45;
	pTcpPacket->theIpPacket.theIpHead.ucTos=0;
	pTcpPacket->theIpPacket.theIpHead.usTotalLength=htons(48+strlen(argv[2]));
	pTcpPacket->theIpPacket.theIpHead.usIdentification=1234;
	pTcpPacket->theIpPacket.theIpHead.usFlagsAndFragmentOffset=0;
	pTcpPacket->theIpPacket.theIpHead.ucTtl=119;
	pTcpPacket->theIpPacket.theIpHead.ucProtocol=6;//tcp
	pTcpPacket->theIpPacket.theIpHead.dwSourceAddr=dwLocalIP;
	pTcpPacket->theIpPacket.theIpHead.dwDestAddr=dwDestIp;
	pTcpPacket->theIpPacket.theIpHead.usCrc=0;
	pTcpPacket->theIpPacket.theIpHead.usCrc=CheckSum((const char *)(&(pTcpPacket->theIpPacket.theIpHead)),sizeof(IpHead));

	pTcpPacket->theTcpHead.usDestPort=htons(1000);
	pTcpPacket->theTcpHead.usSourcePort=htons(3000);
	pTcpPacket->theTcpHead.dwSeq=ntohl(198327);
	pTcpPacket->theTcpHead.dwAck=0;
	pTcpPacket->theTcpHead.ucLength=0x70;
	pTcpPacket->theTcpHead.ucFlag=4;
	pTcpPacket->theTcpHead.usWindow=0xFFFF;	 //16 位窗口大小
	pTcpPacket->theTcpHead.usCrc=0;//16 位校验和
	pTcpPacket->theTcpHead.usUrgent=0;//16 位紧急数据偏移量
	pTcpPacket->theTcpHead.unMssOpt=htonl(0x020405B4);
	pTcpPacket->theTcpHead.usNopOpt= 0x0101;
	pTcpPacket->theTcpHead.usSackOpt= 0x0204;
	pTcpPacket->theTcpHead.usCrc=0;
	
	TcpFakeHeader theTcpFakeHeader;
	theTcpFakeHeader.bZero=0;
	theTcpFakeHeader.bTcpLength=htons(28+strlen(argv[2]));
	theTcpFakeHeader.bProtocolType=6;
	theTcpFakeHeader.dwDestAddr=dwDestIp;
	theTcpFakeHeader.dwSourceAddr=dwLocalIP;
	pTcpPacket->theTcpHead.usCrc=TcpCheckSum((char *)(&(pTcpPacket->theTcpHead)),(char *)(&theTcpFakeHeader),sizeof(TcpHead)+strlen(argv[2]));
	
	if (pcap_sendpacket(hWpcapHandle,(u_char *)pTcpPacket,sizeof(TcpPacket)+strlen(argv[2])	) != 0)
	{
		printf("\nError Sending the TCP Packet: \n", pcap_geterr(hWpcapHandle));
	}
	else
	{
		printf("Send TCP Packet Success!\r\n");
	}

	UdpPacket *pUdpPacket=(UdpPacket *)pTcpPacket;
	
	strcpy(((char*)pUdpPacket)+sizeof(UdpPacket),argv[2]);

	memcpy(pUdpPacket->theEthHead.bDestMac,bDestMac,6);
	memcpy(pUdpPacket->theEthHead.bSourceMac,bLocalMac,6);
	pUdpPacket->theEthHead.usEthernetType=0x8;
	pUdpPacket->theIpHead.ucVersionAndHeadLength=0x45;
	pUdpPacket->theIpHead.ucTos=0;
	pUdpPacket->theIpHead.usTotalLength=htons(28+strlen(argv[2]));
	pUdpPacket->theIpHead.usIdentification=1234;
	pUdpPacket->theIpHead.usFlagsAndFragmentOffset=0;
	pUdpPacket->theIpHead.ucTtl=119;
	pUdpPacket->theIpHead.ucProtocol=17;//udp
	pUdpPacket->theIpHead.dwSourceAddr=dwLocalIP;
	pUdpPacket->theIpHead.dwDestAddr=dwDestIp;;
	pUdpPacket->theIpHead.usCrc=0;
	pUdpPacket->theIpHead.usCrc=CheckSum((USHORT*)(&(pUdpPacket->theIpHead)),sizeof(IpHead));
		
	pUdpPacket->theUdpHead.usSourcePort=ntohs(3000);
	pUdpPacket->theUdpHead.usDestPort=ntohs(2000);
	pUdpPacket->theUdpHead.usLength=ntohs(8+strlen(argv[2]));
	pUdpPacket->theUdpHead.usCrc=0;


	UdpFakeHeader theUdpFakeHeader;
	theUdpFakeHeader.bZero=0;
	theUdpFakeHeader.bUdpLength=htons(sizeof(UdpHead)+strlen(argv[2]));
	theUdpFakeHeader.bProtocolType=17;
	theUdpFakeHeader.dwSourceAddr=dwLocalIP;
	theUdpFakeHeader.dwDestAddr=dwDestIp;
	
	pUdpPacket->theUdpHead.usCrc=UdpCheckSum((char *)&(pUdpPacket->theUdpHead),(char *)&theUdpFakeHeader,sizeof(UdpHead)+strlen(argv[2]));
	

	if (pcap_sendpacket(hWpcapHandle,(u_char *)pUdpPacket,sizeof(UdpPacket)+strlen(argv[2])	) != 0)
	{
		printf("\nError sending the packet: \n", pcap_geterr(hWpcapHandle));
		return 0;
	}
	printf("Send UDP Packet Success!\r\n");
	delete [](char*)pTcpPacket;
	
	return 0;
}

