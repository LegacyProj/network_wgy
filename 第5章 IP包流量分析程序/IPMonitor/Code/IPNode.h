// IPNode.h: interface for the CIPNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IPNODE_H__1366A568_424A_4BDF_8E76_9AF5BA10D449__INCLUDED_)
#define AFX_IPNODE_H__1366A568_424A_4BDF_8E76_9AF5BA10D449__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// 结点类CIPNode
class CIPNode  
{
private:
	unsigned long m_dwSourIPAddr;		// 源IP地址
	unsigned long m_dwDestIPAddr;		// 目的IP地址
	unsigned char m_chProtocol;			// IP包的协议类型
	unsigned long m_dwCouter;			// 数据包的数量

public:
	
	CIPNode * pNext;			// 指向下一类IP结点

	CIPNode();
	virtual ~CIPNode();

	// 构造函数
	CIPNode(unsigned long, unsigned long, unsigned char);

	// 增加数据包的数量
	void addCount();
	// 取得数据包数量
	unsigned long getCount();	
	// 取得源IP地址
	unsigned long getSourIPAddr();	
	// 取得目的IP地址
	unsigned long getDestIPAddr();
	// 取得协议类型
	unsigned char getProtocol();
	// 取得协议名称(TCP,UDP,ICMP...)
	char * getProtocol_String();
};

#endif // !defined(AFX_IPNODE_H__1366A568_424A_4BDF_8E76_9AF5BA10D449__INCLUDED_)
