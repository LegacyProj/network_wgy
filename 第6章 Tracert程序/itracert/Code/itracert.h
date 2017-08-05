#ifndef _ITRACERT_H_
#define _ITRACERT_H_

//IP数据报头
typedef struct
{
	unsigned char	hdr_len	:4;		// length of the header
	unsigned char	version	:4;		// version of IP
	unsigned char	tos;			// type of service
	unsigned short	total_len;		// total length of the packet
	unsigned short	identifier;		// unique identifier
	unsigned short	frag_and_flags;	// flags
	unsigned char	ttl;			// time to live
	unsigned char	protocol;		// protocol (TCP, UDP etc)
	unsigned short	checksum;		// IP checksum

	unsigned long	sourceIP;		// source IP address
	unsigned long	destIP;			// destination IP address

} IP_HEADER;

//ICMP数据报头
typedef struct
{
	BYTE	type;		//8位类型
	BYTE	code;		//8位代码
	USHORT	cksum;		//16位校验和
	USHORT	id;			//16位标识符
	USHORT	seq;		//16位序列号

} ICMP_HEADER;

//解码结果
typedef struct
{
	USHORT usSeqNo;			//包序列号
	DWORD dwRoundTripTime;	//往返时间
	in_addr dwIPaddr;		//对端IP地址

} DECODE_RESULT;

//ICMP类型字段
const BYTE ICMP_ECHO_REQUEST	= 8;	//请求回显
const BYTE ICMP_ECHO_REPLY		= 0;	//回显应答
const BYTE ICMP_TIMEOUT			= 11;	//传输超时

const DWORD DEF_ICMP_TIMEOUT	= 3000;	//默认超时时间，单位ms
const int DEF_ICMP_DATA_SIZE	= 32;	//默认ICMP数据部分长度
const int MAX_ICMP_PACKET_SIZE	= 1024;	//最大ICMP数据报的大小
const int DEF_MAX_HOP = 30;				//最大跳站数

USHORT GenerateChecksum(USHORT* pBuf, int iSize);
BOOL DecodeIcmpResponse(char* pBuf, int iPacketSize, DECODE_RESULT& stDecodeResult);

#endif // _ITRACERT_H_