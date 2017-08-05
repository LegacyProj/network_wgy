#pragma once

struct EthernetHead
{
	unsigned char bDestMac[6];//dest MAC
	unsigned char bSourceMac[6];//source MAC
	unsigned short usEthernetType;//ethernet type
};


struct IpHead
{
    unsigned char  ucVersionAndHeadLength;        // Version (4 bits) + Internet header length (4 bits)
    unsigned char  ucTos;            // Type of service 
    unsigned short usTotalLength;           // Total length 
    unsigned short usIdentification; // Identification
    unsigned short usFlagsAndFragmentOffset;       // Flags (3 bits) + Fragment offset (13 bits)
    unsigned char  ucTtl;            // Time to live
    unsigned char  ucProtocol;          // Protocol
    unsigned short usCrc;            // Header checksum
    unsigned long  dwSourceAddr;      // Source address
    unsigned long  dwDestAddr;      // Destination address
};
struct TcpHead            //定义TCP 首部
{
	USHORT usSourcePort; //16 位源端口
	USHORT usDestPort; //16 位目的端口
	ULONG dwSeq;
	ULONG dwAck;
	UCHAR ucLength;           //4 位首部长度/4 位保留字
	UCHAR ucFlag;            //6 位标志位
	USHORT usWindow; //16 位窗口大小
	USHORT usCrc;//16 位校验和
	USHORT usUrgent;//16 位紧急数据偏移量
	UINT unMssOpt;
	USHORT usNopOpt;
	USHORT usSackOpt;
};
struct IpPacket
{
	EthernetHead theEthHead;
	IpHead theIpHead;
};
struct TcpPacket
{
	IpPacket theIpPacket;
	TcpHead theTcpHead;
};
struct TcpFakeHeader
{
    DWORD dwSourceAddr;						//源地址
    DWORD dwDestAddr;						//目的地址
    BYTE bZero;							//置空
    BYTE bProtocolType;							//协议类型
    USHORT bTcpLength;						//TCP长度
};
struct UdpFakeHeader
{
    DWORD dwSourceAddr;						//源地址
    DWORD dwDestAddr;						//目的地址
    BYTE bZero;							//置空
    BYTE bProtocolType;							//协议类型
    USHORT bUdpLength;						//UDP长度
};
struct UdpHead
{
	u_short usSourcePort;			// Source port
	u_short usDestPort;			// Destination port
	u_short usLength;			// Datagram length
	u_short usCrc;			// Checksum
};
struct UdpPacket
{
	EthernetHead theEthHead;
	IpHead theIpHead;
	UdpHead theUdpHead;
};
USHORT CheckSum(const char *buf, int size) ;
USHORT CheckSum(USHORT *buffer, int size);
unsigned short TcpCheckSum(const char *pTcpData, const char *pPshData, UINT nTcpCount);
unsigned short UdpCheckSum(const char *pUdpData, const char *pPshData, UINT nUdpCount);
