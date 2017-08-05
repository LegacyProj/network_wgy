#if !defined(AFX_CLIENTBYSOCKETDLG_H__BCD212ED_1E03_4319_A76D_8D4199E46364__INCLUDED_)
#define AFX_CLIENTBYSOCKETDLG_H__BCD212ED_1E03_4319_A76D_8D4199E46364__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <fstream>
using namespace std;
/////////////////////////////////////////////////////////////////////////////
// CClientBySocketDlg dialog
//定义网络事件
#define NETWORK_EVENT	(WM_USER + 0x0105)
#define NETWORK_LOG		(WM_USER + 0x0106)

#define LOG_conn_ing 0
#define LOG_conn_suc 1
#define LOG_send_itm 2
#define LOG_send_str 3
#define LOG_recv_upt 4
#define LOG_recv_str 5
#define LOG_conn_cls 6
#define LOG_recv_tim 7
#define LOG_recv_udp 8

typedef unsigned char uchar;
typedef unsigned short usint;
typedef unsigned long ulint;

//将一个无符号长整型的机器IP地址转换为字符串类型的用户友好IP格式
inline char* ipTostr(ulint ulIP)
{
	in_addr ipAddr;
	ipAddr.s_addr = ulIP;
	return inet_ntoa(ipAddr);
}

class CClientBySocketDlg : public CDialog
{
// Construction
public:
	CClientBySocketDlg(CWnd* pParent = NULL);	// standard constructor

// App Data
	WSADATA WSAData;
	SOCKET Sock;
	SOCKET Sock_U;
	struct hostent *pHost;
	SOCKADDR_IN serv_addr;		//目的地IP地址和端口的数据结构
	SOCKADDR_IN serv_udp_addr;	//目的地IP地址和端口的数据结构
	SOCKADDR_IN rmot_udp_addr;	//存放对端udp接收地址结构，重复使用，发时收时均用此结构
	int rmot_udp_addr_len;
	char LocalName[16];
	int nSele;
	SOCKADDR_IN addr_in;
	SOCKADDR_IN addr_in_udp;
	unsigned short int servUDPort;
	BOOL InitNetwork(); //初始化网络函数

	//各种网络异步事件的处理函数
	void OnClose(SOCKET CurSock);	//对端Socket断开
	void OnSend(SOCKET CurSock);	//发送网络数据包
	void OnReceive(SOCKET CurSock);	//网络数据包到达
	void OnConnect(SOCKET CurSock, int error);		//客户端连接请求
	void OnNetEvent(WPARAM wParam, LPARAM lParam);	//异步事件回调函数
	void OnLogDisp(WPARAM wParam, LPARAM lParam);	//日志输出函数
	void OnResum();					//连接中断重置函数
	void OnGetudpPort(SOCKET s);	//发送获知UDP口号请求
	void OnReceiveUDP(SOCKET s);	//接收UDP信息
	void OnReceiveTCP(SOCKET s);	//接收TCP数据
	void OnGetTime(SOCKET s);		//接收TCP时间
	void OnGotudpPort(SOCKET s);	//获知UDP端口号

// Dialog Data
	//{{AFX_DATA(CClientBySocketDlg)
	enum { IDD = IDD_CLIENTBYSOCKET_DIALOG };
	CIPAddressCtrl	m_ipconn;
	CEdit	m_catlog;
	CComboBox	m_iplist;
	CString	m_udptxt;
	CString	m_catlog_txt;
	int		m_conn_to_port;
	CString	m_curlog;
	CString	m_curstat;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClientBySocketDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CClientBySocketDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void On_UDPECHO();
	afx_msg void On_TCPTIME();
	afx_msg void On_UDP_SEND_CLK();
	afx_msg void On_CONN();
	afx_msg void On_Disconn();
	afx_msg void On_Close();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENTBYSOCKETDLG_H__BCD212ED_1E03_4319_A76D_8D4199E46364__INCLUDED_)
