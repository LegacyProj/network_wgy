// ServerBySocketDlg.h : header file
//

#if !defined(AFX_SERVERBYSOCKETDLG_H__5F4761E7_4D2E_4891_B785_27822568EF3C__INCLUDED_)
#define AFX_SERVERBYSOCKETDLG_H__5F4761E7_4D2E_4891_B785_27822568EF3C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <fstream>
using namespace std;
/////////////////////////////////////////////////////////////////////////////
// CServerBySocketDlg dialog
//定义网络事件
#define NETWORK_EVENT	(WM_USER + 0x0105)
#define NETWORK_LOG		(WM_USER + 0x0106)

#define LOG_conn_ing 0
#define LOG_conn_suc 1
#define LOG_reqe_tim 2
#define LOG_send_str 3
#define LOG_stop_svr 4
#define LOG_recv_str 5
#define LOG_conn_cls 6
#define LOG_send_tim 7
#define LOG_send_udp 8
#define LOG_reqe_udp 9

typedef unsigned char uchar;
typedef unsigned short usint;
typedef unsigned long ulint;

#define CLNT_MAX_NUM 9	//最大客户连接数
#define UDP_PORT 5555	//UDP服务端口号
#define MAX_QUEUE 5		//侦听队列最大长度

//将一个无符号长整型的机器IP地址转换为字符串类型的用户友好IP格式
inline char* ipTostr(ulint ulIP)
{
	in_addr ipAddr;
	ipAddr.s_addr = ulIP;
	return inet_ntoa(ipAddr);
}

//存储返回socket等对端信息的结构体
typedef struct client_sock {
	SOCKET Sock;
	SOCKADDR_IN addr;
	int addr_len;
	uchar inuse;
} client_sock;

class CServerBySocketDlg : public CDialog
{
// Construction
public:
	CServerBySocketDlg(CWnd* pParent = NULL);	// standard constructor

// App Data
	client_sock ClientSock[CLNT_MAX_NUM]; //存储由accept返回的与客户端通信的Socket等结构信息的数组
	client_sock ClientSock_U;
	client_sock ClientSock_Abandon;
	int total_conn;
	usint U_port;
	int FindFirstEmpty(client_sock* in); //查找首个空连接池
	int FindSockNo(client_sock* in, SOCKET s); //查找给定Socket在client数组中的编号
	void CloseClientSocket(client_sock* in); //关闭所有客户连接
	WSADATA WSAData;
	SOCKET ServerSock;
	SOCKET ServerSock_U;
	struct hostent *pHost;
	char LocalName[16];
	int nSele;
	SOCKADDR_IN addr_in;
	SOCKADDR_IN addr_in_udp;
	BOOL InitNetwork(); //初始化网络函数
	
	//各种网络异步事件的处理函数
	void OnClose(SOCKET CurSock);	//对端Socket断开
	void OnSend(SOCKET CurSock);	//发送网络数据包
	void OnReceive(SOCKET CurSock);	//网络数据包到达
	void OnAccept(SOCKET CurSock);	//客户端连接请求
	void OnNetEvent(WPARAM wParam, LPARAM lParam);//异步事件回调函数
	void OnLogDisp(WPARAM wParam, LPARAM lParam); //日志输出函数
	void OnResum();					//连接中断重置函数

// Dialog Data
	//{{AFX_DATA(CServerBySocketDlg)
	enum { IDD = IDD_SERVERBYSOCKET_DIALOG };
	CEdit	m_catlog;
	CComboBox	m_iplist;
	int		m_port;
	CString	m_connstat;
	CString	m_curlog;
	CString	m_catlog_txt;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerBySocketDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CServerBySocketDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void On_Stopserv();
	afx_msg void On_Startserv();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERBYSOCKETDLG_H__5F4761E7_4D2E_4891_B785_27822568EF3C__INCLUDED_)
