    // NodeList.h: interface for the CNodeList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODELIST_H__9781C411_82DF_47F7_A449_3054B18A550E__INCLUDED_)
#define AFX_NODELIST_H__9781C411_82DF_47F7_A449_3054B18A550E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include<iostream.h>
#include<fstream.h>
#include<iomanip.h>
#include<winsock2.h>
#include "IPNode.h"

// 结点链表类CNodeList
class CNodeList  
{
private:
	CIPNode * pHead;	// 链表头
	CIPNode * pTail;	// 链表尾
	
public:
	// Default constructor
	CNodeList();
	// Default destructor
	virtual ~CNodeList();

	// 把新捕获的IP数据包加入链表
	void addNode(unsigned long, unsigned long, unsigned char);
	// 输出链表
	ostream& print(ostream&);
};

#endif // !defined(AFX_NODELIST_H__9781C411_82DF_47F7_A449_3054B18A550E__INCLUDED_)
