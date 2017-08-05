// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__D82421A2_CB27_4352_AE1B_69E5E3335D8B__INCLUDED_)
#define AFX_STDAFX_H__D82421A2_CB27_4352_AE1B_69E5E3335D8B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <stdlib.h>
#include <stdio.h>

#include <pcap.h>

#include <iostream>

using namespace std;
#define BROADMAC		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF} //广播MAC
#define EH_TYPE			0x0806							//ARP类型
#define ARP_HRD			0X0001							//硬件类型：以太网接口类型为1		
#define ARP_PRO			0x0800							//协议类型：IP协议类型为0X0800
#define ARP_HLN			0x06							//硬件地址长度：MAC地址长度为6B
#define ARP_PLN			0x04							//协议地址长度：IP地址长度为4B
#define ARP_REQUEST		0x0001							//操作：ARP请求为1
#define ARP_REPLY		0x0002							//操作：ARP应答为2
#define ARP_THA			{0,0,0,0,0,0}					//目的MAC地址：ARP请求中该字段没有意义，设为0；ARP响应中为接收方的MAC地址
#define ARP_PAD			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define SPECIAL			0x70707070						//定义获得自己MAC地址的特殊源IP，112.112.112.112

#define ETH_HRD_DEFAULT	{BROADMAC, {0,0,0,0,0,0}, htons(EH_TYPE)}
#define ARP_HRD_DEFAULT	{htons(ARP_HRD), htons(ARP_PRO), ARP_HLN, ARP_PLN, htons(ARP_REQUEST), {0,0,0,0,0,0}, 0, ARP_THA, 0, ARP_PAD}

#define IPTOSBUFFERS 12

#define WM_PACKET	WM_USER + 105	
struct ethernet_head
{
	unsigned char dest_mac[6];									//目标主机MAC地址
	unsigned char source_mac[6];								//源端MAC地址
	unsigned short eh_type;										//以太网类型
};

struct arp_head
{
	unsigned short hardware_type;								//硬件类型：以太网接口类型为1
	unsigned short protocol_type;								//协议类型：IP协议类型为0X0800
	unsigned char add_len;										//硬件地址长度：MAC地址长度为6B
	unsigned char pro_len;										//协议地址长度：IP地址长度为4B
	unsigned short option;										//操作：ARP请求为1，ARP应答为2

	unsigned char sour_addr[6];									//源MAC地址：发送方的MAC地址
	unsigned long sour_ip;										//源IP地址：发送方的IP地址
	unsigned char dest_addr[6];									//目的MAC地址：ARP请求中该字段没有意义；ARP响应中为接收方的MAC地址
	unsigned long dest_ip;										//目的IP地址：ARP请求中为请求解析的IP地址；ARP响应中为接收方的IP地址
	unsigned char padding[18];
};

struct arp_packet										//最终arp包结构
{
	ethernet_head eth;									//以太网头部
	arp_head arp;										//arp数据包头部
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__D82421A2_CB27_4352_AE1B_69E5E3335D8B__INCLUDED_)
