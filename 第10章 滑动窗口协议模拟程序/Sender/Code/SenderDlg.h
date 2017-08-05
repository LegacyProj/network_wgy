// SenderDlg.h : header file
//

#if !defined(AFX_SENDERDLG_H__F54D7945_DCD8_422C_A985_49386EB53259__INCLUDED_)
#define AFX_SENDERDLG_H__F54D7945_DCD8_422C_A985_49386EB53259__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSenderDlg dialog
#include "Protocol.h"

#define MAX_SEQ 15
#define NR_BUFS ((MAX_SEQ+1)/2)
//Timer ID
#define ID_SEND_TIMER 1
#define ID_TIMER_USER 2
//Event Type. 使用Windows消息模拟
typedef enum
{
	FRAME_ARRIVAL = (WM_USER+1),
	CKSUM_ERROR,
	TIMEOUT,
	NETWORK_LAYTER_READY,
	ACK_TIMEOUT,
} eventType;

class CSenderDlg : public CDialog
{
// Construction
public:
	CSenderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSenderDlg)
	enum { IDD = IDD_SENDER_DIALOG };
	CListBox	m_ListOutput;
	int		m_SendWndSize;
	int		m_SendInterval;
	int		m_ResendTime;
	CString	m_strChksumErr;
	CString	m_strFrameLost;
	int		m_ErrSetting;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSenderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSenderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnStopSend();
	virtual void OnCancel();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMenuCleanup();
	afx_msg void OnStartSend();
	//}}AFX_MSG
	afx_msg LRESULT OnNetworkLayerReady(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFrameArrival(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
		
private:
	static DWORD WINAPI UdpReceiveThread(LPVOID lpParam);	//UDP接收线程
	void UdpReceive();
	void SendFrame(frameKind fk, seqNum seq);
	void FromNetworkLayer(packet* p) {}
	void FromPhysicalLayer(char* p, char* pFromPhysical);
	void ToPhysicalLayer(char* pBuf, int iSize);
	void ReSendFrame(int framePos);
	void Inc(seqNum& seq) { (seq < MAX_SEQ) ? seq++ : seq=0; }
	BOOL Between(seqNum a, seqNum b, seqNum c)
	{
		//return true if a<=b<c, false otherwise
		return ((a<=b && b<c) || (c<a && a<=b) || (b<c && c<a));
	}

private:
	SOCKET m_UDPSndrSocket;
	SOCKADDR_IN m_UDPRcvrAddr;
	frame* m_pOutBuf;		 //数据输出缓冲区 = 滑动窗口大小×最大帧长
	int m_iBuffered;		 //当前滑动窗口大小
	seqNum m_ackExpected;	 //发送窗口左侧
	seqNum m_nextFrameToSend;//发送窗口右侧+1
	errMode m_errArray[MAX_SEQ+1]; //每个帧的错误模式
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDERDLG_H__F54D7945_DCD8_422C_A985_49386EB53259__INCLUDED_)
