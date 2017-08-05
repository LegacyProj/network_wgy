
#include "arp.h"
#include "StdAfx.h"
#pragma comment(lib, "wpcap")
extern string strSelDeviceName;
extern unsigned char* bLocalMac;
extern pcap_if_t* pDevGlobalHandle;
extern int nThreadSignal;
extern int GetMacSignal;


char* IpToStr(unsigned long ulIP)	
{							
	static char output[IPTOSBUFFERS][3 * 4 + 3 + 1];
	static short which;									
	unsigned char* chIP;
	chIP = (unsigned char*)&ulIP;							
	which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1); 
	sprintf(output[which], "%d.%d.%d.%d", chIP[0], chIP[1], chIP[2], chIP[3]); 
	return output[which];
}
char* MacToStr(unsigned char* chMAC)
{							
	static unsigned char uMac[18];
	for(int i=0; i < 17; i++)
	{
		if ((i+1) % 3)
		{
			if (!(i % 3))
			{
				if ((chMAC[i/3] >> 4) < 0x0A)
				{
					uMac[i] = (chMAC[i/3] >> 4) + 48;
				}
				else
				{
					uMac[i] = (chMAC[i/3] >> 4) + 55;
				}
				if ((chMAC[i/3] & 0x0F) < 0x0A)
				{
					uMac[i+1] = (chMAC[i/3] & 0x0F) + 48;
				}
				else
				{
					uMac[i+1] = (chMAC[i/3] & 0x0F) + 55;
				}
			}
		}
		else
		{
			uMac[i] = '-';
		}
	}
	uMac[17] = '\0';
	return (char*)uMac;
}

char* DelSpace(char* in)
{								
	int strLen = 0;
	while(in[strLen++] != '\0');
	static char chTemp[10240];
	if(10240<strLen)
	{
		MessageBox(NULL,"Over Flow ","Error",MB_OK|MB_ICONERROR);
		return NULL;
	}
	int i = 0;
	int j = 0;
	while(in[i] != '\0')
	{
		if(in[i] != ' ')
		{
			chTemp[j++] = in[i++];
		}
		else
		{
			chTemp[j++] = in[i++];
			while(in[i] == ' ')i++;					
		}
	}
	chTemp[j] = '\0';
	return chTemp;
}

unsigned char* BuildArpRequestPacket(unsigned char* source_mac, unsigned char* arp_sha, unsigned long chLocalIP, unsigned long arp_tpa, int PackSize)
{	//封装ARP请求包
	static arp_packet arpPackStru;
	static const arp_packet arpDefaultPack= {ETH_HRD_DEFAULT,ARP_HRD_DEFAULT};
	memcpy(&arpPackStru,&arpDefaultPack,sizeof(arpDefaultPack));
	memcpy(arpPackStru.eth.source_mac,source_mac,6);
	memcpy(arpPackStru.arp.sour_addr,arp_sha,6);
	arpPackStru.arp.sour_ip=chLocalIP;	
	arpPackStru.arp.dest_ip=arp_tpa;
	return (unsigned char *)&arpPackStru;
}



unsigned char* GetSelfMac(char* pDevName, unsigned long chLocalIP)
{		//获得自己的MAC
	pcap_t* pAdaptHandle;														//打开网卡适配器时用
	char errbuf[PCAP_ERRBUF_SIZE + 1]; 
	if((pAdaptHandle = pcap_open_live(pDevName, 60, 1, 100, errbuf)) == NULL)
	{	
		MessageBox(NULL, "无法打开适配器，可能与之不兼容!", "Note", MB_OK);
		return NULL;
	}
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	int res;
	unsigned short arp_op;
	static unsigned char arp_sha[6];
	unsigned long arp_spa = 0;
	unsigned long arp_tpa = 0;
	unsigned char source_mac[6] = {0,0,0,0,0,0};
	unsigned char* arp_packet_for_self;
	arp_packet_for_self = BuildArpRequestPacket(source_mac, source_mac, SPECIAL, chLocalIP, 60);
	while(!GetMacSignal)
	{
		pcap_sendpacket(pAdaptHandle, arp_packet_for_self, 60);
		Sleep(10);										
		res = pcap_next_ex(pAdaptHandle, &header, &pkt_data);
		if(res == 0)
		{
			continue;
		}
		memcpy(&arp_op, pkt_data + 20, 2);
		memcpy(arp_sha, pkt_data + 22, 6);
		memcpy(&arp_spa, pkt_data + 28, 4);	
		memcpy(&arp_tpa, pkt_data + 38, 4);	
		if(arp_op == htons(ARP_REPLY) && arp_spa == chLocalIP && arp_tpa == SPECIAL)
		{	
			GetMacSignal = 1;
			pcap_close(pAdaptHandle);
			return arp_sha;
		}
		Sleep(100);																		//若不成功再等100ms再发，让网卡歇歇:) 20061025
	}
	pcap_close(pAdaptHandle);
	return arp_sha;
}

void SendArpRequest(pcap_if_t* pDev, unsigned char* bLocalMac)
{	//发送ARP请求
	pcap_addr_t* pAdr = 0;
	unsigned long chLocalIp = 0;								//存放本地ip地址
	unsigned long arp_tpa = 0;
	unsigned long snd_tpa = 0;
	unsigned long nlNetMask = 0;
	int netsize = 0;
	const char* pDevName = strSelDeviceName.c_str();
	pcap_t* pAdaptHandle;								//打开网卡适配器时用
	char errbuf[PCAP_ERRBUF_SIZE + 1]; 
	if((pAdaptHandle = pcap_open_live(pDev->name, 60, 0, 100, errbuf)) == NULL)
	{	
		MessageBox(NULL, "无法打开适配器，可能与之不兼容!", "Send", MB_OK);
		return;
	}
	unsigned char* arp_packet_for_req;
	arp_packet_for_req = BuildArpRequestPacket(bLocalMac, bLocalMac, chLocalIp, chLocalIp, 60);	//构造包，并将只更改倒数第二个形参，在字符串里对之进行局部更改！效率高得多，不要重复申请STRUCT，会浪费大量内存空间20061024
	unsigned long ulOldMask=0;
	for (pAdr = pDev->addresses; pAdr; pAdr = pAdr->next)
	{
		if (!nThreadSignal)
		{
			break;
		}
		chLocalIp = ((struct sockaddr_in *)pAdr->addr)->sin_addr.s_addr;			//得到本地ip
		if (!chLocalIp) 
		{
			continue;
		}
		nlNetMask = ((struct sockaddr_in *)(pAdr->netmask))->sin_addr.S_un.S_addr;	//得到子网掩码
		if(ulOldMask==nlNetMask)
		{
			continue;
		}
		ulOldMask=nlNetMask;
		netsize = ~ntohl(nlNetMask);
		arp_tpa = ntohl(chLocalIp & nlNetMask);
	//	memcpy(arp_packet_for_req + 28, &chLocalIp, 4);								//将字串中源IP设为本次得到的本地IP
		for (int i=0; i < netsize; i++)
		{
			if (!nThreadSignal) 
			{
				break;
			}
			arp_tpa++;
			snd_tpa = htonl(arp_tpa);
			memcpy(arp_packet_for_req + 38, &snd_tpa, 4);							//目的IP在子网范围内按序增长	
			pcap_sendpacket(pAdaptHandle, arp_packet_for_req, 60);
			Sleep(5);
		}
	}
}

UINT StartArpScan(LPVOID mainClass)
{
	AfxGetApp()->m_pMainWnd->SendMessage(WM_PACKET, 0, 1);
	SendArpRequest(pDevGlobalHandle, bLocalMac);									//对选中设备的所有绑定的IP网段进行ARP请求
	AfxGetApp()->m_pMainWnd->SendMessage(WM_PACKET, 0, 2);
	return 0;
}

UINT WaitForArpRepeatPacket(LPVOID mainClass)
{								
	pcap_t* pAdaptHandle;														//打开网卡适配器时用
	const char* pDevName = strSelDeviceName.c_str();
	char errbuf[PCAP_ERRBUF_SIZE + 1]; 
	if((pAdaptHandle = pcap_open_live(pDevName, 60, 0, 100, errbuf)) == NULL)
	{	
		MessageBox(NULL, "无法打开适配器，可能与之不兼容!", "wait", MB_OK);
		return -1;
	}
	string ipWithMac;
	char* filter = "ether proto\\arp";
	bpf_program fcode;
	int res;
	unsigned short arp_op = 0;
	unsigned char arp_sha [6];
	unsigned long arp_spa = 0;
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	if (pcap_compile(pAdaptHandle, &fcode, filter, 1, (unsigned long)(0xFFFF0000)) < 0)
	{
		MessageBox(NULL,"过滤条件语法错误!", "wait", MB_OK);
		return -1;
	}
	//set the filter
	if (pcap_setfilter(pAdaptHandle, &fcode) < 0)
	{
		MessageBox(NULL,"适配器与过滤条件不兼容!", "wait", MB_OK);
		return -1;
	}
	while(1)
	{
		if (!nThreadSignal) 
		{
			break;
		}
		int i = 0;
		ipWithMac = "";
		res = pcap_next_ex(pAdaptHandle, &header, &pkt_data);
		if (!res)
		{
			continue;
		}
		memcpy(&arp_op, pkt_data + 20, 2);
		memcpy(arp_sha, pkt_data + 22, 6);
		memcpy(&arp_spa, pkt_data + 28, 4);
		ipWithMac += IpToStr(arp_spa);
		for (int j = strlen(IpToStr(arp_spa)); j < 16; j++)
		{
			ipWithMac += " ";
		}
		ipWithMac += "  --*->   ";
		ipWithMac += MacToStr(arp_sha);
		for (i = 6; i > 0; i--)
		{												
			if (arp_sha[i - 1] != bLocalMac[i - 1])
			{
				break;
			}
		}
		if(arp_op == htons(ARP_REPLY) && i)
		{
			AfxGetApp()->m_pMainWnd->SendMessage(WM_PACKET, WPARAM(&ipWithMac), 0);
		}
	}
	return 0;
}
