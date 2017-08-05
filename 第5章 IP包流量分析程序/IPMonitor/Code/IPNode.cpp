// IPNode.cpp: implementation of the CIPNode class.
//
//////////////////////////////////////////////////////////////////////

#include "IPNode.h"

// Default constructor
CIPNode::CIPNode()
{

}

// Default destructor
CIPNode::~CIPNode()
{

}

CIPNode::CIPNode(unsigned long dwSourIP,unsigned long dwDestIP,unsigned char chPro)
{
	m_dwSourIPAddr = dwSourIP;
	m_dwDestIPAddr = dwDestIP;
	m_chProtocol = chPro;
	m_dwCouter = 1;			// 初始化数据包个数为1
}

// 增加数据包的数量
void CIPNode::addCount()
{
	m_dwCouter++;
}

// 取得数据包数量
unsigned long CIPNode::getCount()
{
	return m_dwCouter;
}

// 取得源IP地址
unsigned long CIPNode::getSourIPAddr()
{
	return m_dwSourIPAddr;
}

// 取得目的IP地址
unsigned long CIPNode::getDestIPAddr()
{
	return m_dwDestIPAddr;
}

// 取得协议类型
unsigned char CIPNode::getProtocol()
{
	return m_chProtocol;
}

// 取得协议名称
char * CIPNode::getProtocol_String()
{
	switch(m_chProtocol)
	{
	case 1:
		return "ICMP";
		break;
	case 2:
		return "IGMP";
		break;
	case 4:
		return "IP in IP";
		break;
	case 6:
		return "TCP";
		break;
	case 8:
		return "EGP";
		break;
	case 17:
		return "UDP";
		break;
	case 41:
		return "IPv6";
		break;
	case 46:
		return "RSVP";
		break;
	case 89:
		return "OSPF";
		break;
	default:
		return "UNKNOWN";
	}
}
