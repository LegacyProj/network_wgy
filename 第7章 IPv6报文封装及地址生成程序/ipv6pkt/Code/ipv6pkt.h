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

#pragma pack(pop)

//64位长度整型的主机字节序转网络字节序函数
__int64 hton64(__int64 host64);


#endif //_IPV6PKT_H_

