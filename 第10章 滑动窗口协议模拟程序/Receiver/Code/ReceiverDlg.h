// ReceiverDlg.h : header file
//

#if !defined(AFX_RECEIVERDLG_H__5AA98424_35AD_4D6B_A28F_64DD0F8CE0DD__INCLUDED_)
#define AFX_RECEIVERDLG_H__5AA98424_35AD_4D6B_A28F_64DD0F8CE0DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CReceiverDlg dialog
#include "Protocol.h"

#define MAX_SEQ 15
#define NR_BUFS ((MAX_SEQ+1)/2)
//Timer ID
#define ID_RECV_TIMER 1
#define ID_ACK_TIMER  2
//Event Type. 使用Windows消息模拟
typedef enum
{
	FRAME_ARRIVAL = (WM_USER+1),
	CKSUM_ERROR,
	TIMEOUT,
	NETWORK_LAYTER_READY,
	ACK_TIMEOUT,
} eventType;

class CReceiverDlg : public CDialog
{
// Construction
public:
	CReceiverDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CReceiverDlg)
	enum { IDD = IDD_RECEIVER_DIALOG };
	CListBox	m_ListOutput;
	int		m_RecvWndSize;
	int		m_RecvInterval;
	int		m_ErrSetting;
	CString	m_strAckLost;
	CString	m_strAckChkErr;
	int		m_AuxiliaryTime;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReceiverDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CReceiverDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnStartRecv();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnStopRecv();
	virtual void OnCancel();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMenuCleanup();
	//}}AFX_MSG
	afx_msg LRESULT OnFrameArrival(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCksumErr(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	static DWORD WINAPI UdpReceiveThread(LPVOID lpParam);	//UDP接收线程
	void UdpReceive();
	void SendFrame(frameKind fk, seqNum ack);
	void ToNetworkLayer(packet* p) {}
	void ToNetworkLayer(int iSeqNo); //仅为了输出模拟过程日志信息
	void FromPhysicalLayer(char* p, char* pFromPhysical);
	void ToPhysicalLayer(char* pBuf, int iSize);
	void Inc(seqNum& seq) { (seq < MAX_SEQ) ? seq++ : seq=0; }
	BOOL Between(seqNum a, seqNum b, seqNum c)
	{
		//return true if a<=b<c, false otherwise
		return ((a<=b && b<c) || (c<a && a<=b) || (b<c && c<a));
	}

private:
	SOCKET m_UDPRcvrSocket;
	SOCKADDR_IN m_UDPSndrAddr;
	frame* m_pInBuf;		//数据接收缓冲区 = 滑动窗口大小×最大帧长
	BOOL* m_pArrived;		//接受窗口内数据包是否到达的位图
	seqNum m_frameExpected;	//接收窗口左侧
	seqNum m_tooFar;		//接收窗口右侧+1
	errMode m_errArray[MAX_SEQ+1]; //应答帧的错误模式
	int m_iTokenCount;		//令牌计数器，最大为接收窗口大小
	BOOL m_bNoNak;			//标志还未发送过nak
	BOOL m_bRecving;		//标志是否开始接收数据包
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECEIVERDLG_H__5AA98424_35AD_4D6B_A28F_64DD0F8CE0DD__INCLUDED_)
