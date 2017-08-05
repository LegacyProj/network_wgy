#ifndef _IPV6PKT_H_
#define _IPV6PKT_H_

#pragma pack(push, 1) //should be aligned on 1 byte boundaries

#define MAX_PAYLOAD_LEN 65535

//IPv6基本首部
typedef struct
{
	u_long	ver_cls_lab;	// 4-bit版本号 + 8-bit流量等级 + 20-bit流标签
	u_short	payload_len;	// 16-bit 载荷长度
	u_char	next_header;	// 8-bit 下一首部
	u_char	hop_limit;		// 8-bit 跳数限制
	struct 
	{
		__int64 prefix_subnetid;
		u_char interface_id[8];
	} src_ip;				// 128-bit 源地址
	struct 
	{
		__int64 prefix_subnetid;
		u_char interface_id[8];
	} dst_ip;				// 128-bit 目的地址

} IPV6_HEADER;

//TCP头部
typedef struct
{
    u_short src_port;		//16位源端口号
    u_short dest_port;		//16位目的端口号
	u_long seq;				//32位序列号
	u_long ack;				//32位确认序号
	u_char hdr_len;			//4位首部长度 + 6位保留
	u_char flags;			//6位标志位
	u_short window_size;	//16位窗口大小
	u_short checksum;		//16位校验和
	u_short urgent_pointer; //16位紧急指针

} TCP_HEADER;

//TCP伪头部
typedef struct
{
    u_char src_ip[16];		//128位源地址
    u_char dst_ip[16];		//128位目的地址
	u_long pkt_len;			//32位上层协议包总长度
	u_long next_hdr;		//24位0 + 8位下一首部

} PSEUDO_HEADER;

#pragma pack(pop)

//64位长度整型的主机字节序转网络字节序函数
__int64 hton64(__int64 host64);

//计算网际校验和
USHORT checksum(USHORT* pBuf, int iSize);

#endif //_IPV6PKT_H_

