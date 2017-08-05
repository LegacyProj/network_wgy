// NodeList.cpp: implementation of the CNodeList class.
//
//////////////////////////////////////////////////////////////////////

#include "NodeList.h"

// Default constructor
CNodeList::CNodeList()
{
	pHead = pTail = NULL;
}

// Default destructor
CNodeList::~CNodeList()
{
	// 删除链表中所有结点
	if (pHead != NULL)
	{
		CIPNode * pTemp = pHead;
		pHead = pHead->pNext;
		delete pTemp;
	}
}

// 把新捕获的IP数据包加入链表
void CNodeList::addNode(unsigned long dwSourIP,unsigned long dwDestIP,unsigned char chPro)
{
	if (pHead == NULL)	// 链表空
	{
		pTail = new CIPNode(dwSourIP,dwDestIP,chPro);
		pHead = pTail;
		pTail->pNext = NULL;
	}
	
	else	// 链表不空时
	{
		CIPNode * pTemp;
		for(pTemp = pHead; pTemp; pTemp = pTemp->pNext)
		{
			// 如果链表中已存在该类型的IP包，则数据包个数加1
			if (pTemp->getSourIPAddr() == dwSourIP && 
				pTemp->getDestIPAddr() == dwDestIP && 
				pTemp->getProtocol() == chPro)
			{
				// 数据包个数加1
				pTemp->addCount();
				break;
			}
		}
		// 如果链表中不存在该类型的IP包，则创建新的结点加入链表
		if (pTemp == NULL)
		{
			pTail->pNext = new CIPNode(dwSourIP,dwDestIP,chPro);
			pTail = pTail->pNext;
			pTail->pNext = NULL;
		}
	}
}

// 输出链表
ostream& CNodeList::print(ostream & out)
{
	CIPNode * pTemp;
	if(pHead == NULL)
	{
		out << "没有捕获到IP数据包!" << endl;
	}
	else
	{
		out << "源地址  " << '\t' << "目的地址" << '\t' << "协议类型  " << "数据包数量" <<endl;
		for(pTemp = pHead; pTemp; pTemp = pTemp->pNext)
		{
			unsigned long dwSourTemp = pTemp->getSourIPAddr();
			unsigned long dwDestTemp = pTemp->getDestIPAddr();
			out << inet_ntoa(*(in_addr *)&(dwSourTemp)) << '\t';
			out << inet_ntoa(*(in_addr *)&(dwDestTemp)) << '\t';
			out << resetiosflags(ios::right) << setiosflags(ios::left)
				<< setfill(' ') << setw(10) << pTemp->getProtocol_String()
				<< resetiosflags(ios::left) << setiosflags(ios::right);
			out << pTemp->getCount() << endl;
		}
	}
	return out;
}
