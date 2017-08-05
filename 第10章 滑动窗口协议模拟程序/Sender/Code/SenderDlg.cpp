// SenderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Sender.h"
#include "SenderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSenderDlg dialog

CSenderDlg::CSenderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSenderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSenderDlg)
	m_SendWndSize = 8;
	m_SendInterval = 1000;
	m_ResendTime = 10000;
	m_strChksumErr = _T("");
	m_strFrameLost = _T("");
	m_ErrSetting = 1;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pOutBuf = NULL;		//数据输出缓冲区
}

void CSenderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSenderDlg)
	DDX_Control(pDX, IDC_LIST_OUTPUT, m_ListOutput);
	DDX_Text(pDX, IDC_SEND_WND_SIZE, m_SendWndSize);
	DDV_MinMaxInt(pDX, m_SendWndSize, 1, MAX_SEQ);
	DDX_Text(pDX, IDC_SEND_INTERVAL, m_SendInterval);
	DDV_MinMaxInt(pDX, m_SendInterval, 100, 60000);
	DDX_Text(pDX, IDC_RESEND_TIMER, m_ResendTime);
	DDV_MinMaxInt(pDX, m_ResendTime, 100, 60000);
	DDX_Text(pDX, IDC_CHKSUM_ERR, m_strChksumErr);
	DDX_Text(pDX, IDC_FRAME_LOST, m_strFrameLost);
	DDX_Radio(pDX, IDC_RANDOM_ERR, m_ErrSetting);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSenderDlg, CDialog)
	//{{AFX_MSG_MAP(CSenderDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_STOP_SEND, OnStopSend)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_MENU_CLEANUP, OnMenuCleanup)
	ON_BN_CLICKED(IDC_START_SEND, OnStartSend)
	//}}AFX_MSG_MAP
	ON_MESSAGE(NETWORK_LAYTER_READY, OnNetworkLayerReady)
	ON_MESSAGE(FRAME_ARRIVAL, OnFrameArrival)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSenderDlg message handlers

BOOL CSenderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	//初始化WinSock
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		AfxMessageBox("Failed to initialize the winsock 2 stack");
		return FALSE;
	}

	//创建UDP Sender Socket
	if ((m_UDPSndrSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		AfxMessageBox("Failed to create UDPSocket");
		return FALSE;
	}

	//填充本地UDP Sender Socket地址结构
	SOCKADDR_IN UDPSndrAddr;
	memset(&UDPSndrAddr, 0, sizeof(SOCKADDR_IN));
	UDPSndrAddr.sin_family = AF_INET;
	UDPSndrAddr.sin_port = htons(3073);
	UDPSndrAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	//绑定Sender UDP端口
	if (bind(m_UDPSndrSocket, (sockaddr*)&UDPSndrAddr, sizeof(UDPSndrAddr)) == SOCKET_ERROR )
	{
		AfxMessageBox("Failed to bind UDPSndrAddr");
		return FALSE;
	}

	//填充Receiver UDP地址
	memset(&m_UDPRcvrAddr, 0, sizeof(SOCKADDR_IN));
	m_UDPRcvrAddr.sin_family = AF_INET;
	m_UDPRcvrAddr.sin_port = htons(3074);
	m_UDPRcvrAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	//创建UDP数据包接收线程
	DWORD dwThreadId;
	CreateThread(NULL, 0, UdpReceiveThread, this, 0, &dwThreadId);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSenderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSenderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSenderDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSenderDlg::OnStartSend() 
{
	// TODO: Add your control notification handler code here
	//获取对话框数据
	if (!UpdateData(TRUE))
		return;

	//初始化所有参数
	if (m_pOutBuf != NULL)
		delete []m_pOutBuf;
	m_pOutBuf = new frame[m_SendWndSize];
	m_iBuffered = 0;		//当前滑动窗口大小
	m_ackExpected = 0;		//发送窗口左侧 
	m_nextFrameToSend = 0;	//发送窗口右侧+1
	for (int i=0; i<MAX_SEQ+1; i++) //错误模式初始均为NO_ERR
		m_errArray[i] = NO_ERR;

	//设置帧的错误模式，模拟传输错误。目前只支持手工设定
	char tmp[256];
	char *token;
	char seps[]   = " ";
	int iSeq;
	if (!m_strFrameLost.IsEmpty())
	{
		strcpy(tmp, m_strFrameLost);
		token = strtok(tmp, seps);
		while (token != NULL)
		{
			iSeq = atoi(token);
			token = strtok(NULL, seps);
			if (iSeq < 0 || iSeq > MAX_SEQ)
			{
				AfxMessageBox("Invalid seqno in LostFrame box, ignore it");
				continue;
			}
			m_errArray[iSeq] = LOST_ERR;
		}

	}
	if (!m_strChksumErr.IsEmpty())
	{
		strcpy(tmp, m_strChksumErr);
		token = strtok(tmp, seps);
		while (token != NULL)
		{
			iSeq = atoi(token);
			token = strtok(NULL, seps);
			if (iSeq < 0 || iSeq > MAX_SEQ)
			{
				AfxMessageBox("Invalid seqno in ChksumErr box, ignore it");
				continue;
			}
			m_errArray[iSeq] = CKSUM_ERR;
		}

	}
	//启动网络层数据发送定时器
	SetTimer(ID_SEND_TIMER, m_SendInterval, NULL);

	//窗口界面控制
	GetDlgItem(IDC_SEND_WND_SIZE)->EnableWindow(FALSE);
	GetDlgItem(IDC_SEND_INTERVAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_RESEND_TIMER)->EnableWindow(FALSE);
	GetDlgItem(IDC_RANDOM_ERR)->EnableWindow(FALSE);
	GetDlgItem(IDC_MANUAL_ERR)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHKSUM_ERR)->EnableWindow(FALSE);
	GetDlgItem(IDC_FRAME_LOST)->EnableWindow(FALSE);
	GetDlgItem(IDC_START_SEND)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP_SEND)->EnableWindow();
	CString strMsg;
	strMsg.Format("%d", m_ackExpected);
	GetDlgItem(IDC_BOTTOM)->SetWindowText(strMsg);
	strMsg.Format("%d", m_nextFrameToSend);
	GetDlgItem(IDC_TOP)->SetWindowText(strMsg);
	GetDlgItem(IDC_CUR_FRAME)->SetWindowText("0");
}

void CSenderDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	switch(nIDEvent)
	{
	case ID_SEND_TIMER:
		if (m_iBuffered < m_SendWndSize)
			PostMessage(NETWORK_LAYTER_READY);
		break;

	default: //case timeout
		int framePos = nIDEvent - ID_TIMER_USER;
		ASSERT(framePos>=0 && framePos<m_SendWndSize);
		ReSendFrame(framePos);
		break;
	}
	
	CDialog::OnTimer(nIDEvent);
}

void CSenderDlg::ReSendFrame(int framePos)
{
	frame* pFrame = &m_pOutBuf[framePos];
	pFrame->hdr.err = NO_ERR;	//重发时去掉出错模拟
	ToPhysicalLayer((char*)pFrame, sizeof(pFrame->hdr));
	SetTimer(framePos + ID_TIMER_USER, m_ResendTime, NULL); //重发定时器

	//窗口显示
	char buf[32];
	_strtime(buf);
	CString strMsg;
	strMsg.Format("%s %5s %8s %7s %3d", buf, "data", "re-sent", "normal", pFrame->hdr.seq);
	m_ListOutput.AddString(strMsg);
	int iCount = m_ListOutput.GetCount();
	if (iCount > 0)
		m_ListOutput.SetCurSel(iCount-1);
}

//UDP接收线程
DWORD CSenderDlg::UdpReceiveThread(LPVOID lpParam)
{
	CSenderDlg* pDlg = (CSenderDlg*)lpParam;
	pDlg->UdpReceive();
	return 0;
}

void CSenderDlg::UdpReceive()
{
	char buf[sizeof(frame)];
	int BytesReceived;
	while (TRUE)
	{
		//接收UDP应答包
		if ((BytesReceived=recvfrom(m_UDPSndrSocket, buf, sizeof(buf), 0, NULL, NULL)) == SOCKET_ERROR)
		{
			TRACE("Failed to recvfrom UDPSndrSocket, ErrCode: %d\n", WSAGetLastError()); //10054错误是正常的
			//AfxMessageBox("Failed to recvfrom UDPSndrSocket");
			continue;
		}
		ASSERT(BytesReceived>=sizeof(frame_hdr) && BytesReceived<=sizeof(frame));
		if (BytesReceived>=sizeof(frame_hdr) && BytesReceived<=sizeof(frame))
		{
			frame* pFrame = (frame*)buf;
			TRACE("Get %s%d, errMode=%d\n", (pFrame->hdr.kind==ACK)?"Ack":"Nak", pFrame->hdr.ack, pFrame->hdr.err);
			if (pFrame->hdr.err == NO_ERR)
			{
				frame* p = new frame;
				memcpy(p, buf, BytesReceived);
				PostMessage(FRAME_ARRIVAL, (WPARAM)p);
			}
		}
	}
}

LRESULT CSenderDlg::OnNetworkLayerReady(WPARAM wParam, LPARAM lParam)
{
	m_iBuffered++;	//扩展滑动窗口大小
	FromNetworkLayer(&m_pOutBuf[m_nextFrameToSend % m_SendWndSize].info); //从网络层取得数据
	SendFrame(DATA, m_nextFrameToSend); //发送数据帧
	Inc(m_nextFrameToSend); //扩展滑动窗口右沿

	//窗口显示
	CString strMsg;
	strMsg.Format("%d", m_ackExpected);
	GetDlgItem(IDC_BOTTOM)->SetWindowText(strMsg);
	strMsg.Format("%d", m_nextFrameToSend);
	GetDlgItem(IDC_TOP)->SetWindowText(strMsg);
	return 0;
}

void CSenderDlg::SendFrame(frameKind fk, seqNum seq)
{
	frame* pFrame = &m_pOutBuf[seq % m_SendWndSize];
	pFrame->hdr.kind = fk;
	pFrame->hdr.seq = seq;
	pFrame->hdr.err = m_errArray[seq];
	ToPhysicalLayer((char*)pFrame, sizeof(pFrame->hdr));
	SetTimer(seq % m_SendWndSize + ID_TIMER_USER, m_ResendTime, NULL); //重发定时器

	//窗口显示格式
	//12:2:34 data    sent normal  0
	//12:2:35 data    sent lost    1
	//12:2:36 data    sent chkerr  2
	//12:2:38 data re-sent normal  6
	//12:2:39 data re-sent normal  7
	//12:2:40  ack arrived normal  0
	char buf[32];
	_strtime(buf);
	CString strErrmode;
	if (pFrame->hdr.err == NO_ERR)
		strErrmode = "normal";
	else if (pFrame->hdr.err == CKSUM_ERR)
		strErrmode = "chkerr";
	else
		strErrmode = "lost";
	CString strMsg;
	strMsg.Format("%s %5s %8s %7s %3d", buf, "data", "sent", strErrmode, pFrame->hdr.seq);
	m_ListOutput.AddString(strMsg);
	int iCount = m_ListOutput.GetCount();
	if (iCount > 0)
		m_ListOutput.SetCurSel(iCount-1);
}

void CSenderDlg::ToPhysicalLayer(char* pBuf, int iSize)
{
	sendto(m_UDPSndrSocket, pBuf, iSize, 0, (sockaddr*)&m_UDPRcvrAddr, sizeof(m_UDPRcvrAddr));

	//窗口显示
	CString strMsg;
	strMsg.Format("%d", ((frame_hdr*)pBuf)->seq);
	GetDlgItem(IDC_CUR_FRAME)->SetWindowText(strMsg);
}

LRESULT CSenderDlg::OnFrameArrival(WPARAM wParam, LPARAM lParam)
{
	frame ackFrame;
	FromPhysicalLayer((char*)&ackFrame, (char*)wParam);

	//窗口显示
	TRACE("arrived %s%d\n", (ackFrame.hdr.kind==ACK)?"Ack":"Nak", ackFrame.hdr.ack);
	char buf[32];
	_strtime(buf);
	CString strMsg;
	strMsg.Format("%s %5s %8s %7s %3d", buf, (ackFrame.hdr.kind==ACK)?"ack":"nak", "arrived", "normal", ackFrame.hdr.ack);
	m_ListOutput.AddString(strMsg);
	int iCount = m_ListOutput.GetCount();
	if (iCount > 0)
		m_ListOutput.SetCurSel(iCount-1);
	
	if (ackFrame.hdr.kind==NAK && Between(m_ackExpected, ackFrame.hdr.ack, m_nextFrameToSend))
	{
		ReSendFrame(ackFrame.hdr.ack % m_SendWndSize);
	}

	seqNum ackedSeq = (ackFrame.hdr.kind==ACK)? ackFrame.hdr.ack : (ackFrame.hdr.ack+MAX_SEQ)%(MAX_SEQ+1);
	while (Between(m_ackExpected, ackedSeq, m_nextFrameToSend))
	{
		m_iBuffered--;
		KillTimer(m_ackExpected % m_SendWndSize + ID_TIMER_USER);
		Inc(m_ackExpected); //扩展滑动窗口左沿

		//窗口显示
		CString strMsg;
		strMsg.Format("%d", m_ackExpected);
		GetDlgItem(IDC_BOTTOM)->SetWindowText(strMsg);
		strMsg.Format("%d", m_nextFrameToSend);
		GetDlgItem(IDC_TOP)->SetWindowText(strMsg);
	}

	return 0;
}

void CSenderDlg::FromPhysicalLayer(char* p, char* pFromPhysical)
{
	memcpy(p, pFromPhysical, sizeof(frame));
	delete (frame*)pFromPhysical;
}


void CSenderDlg::OnStopSend() 
{
	// TODO: Add your control notification handler code here
	//停止所有定时器
	for (int i=1; i<m_SendWndSize+ID_TIMER_USER; i++)
		KillTimer(i);

	//窗口界面控制
	GetDlgItem(IDC_SEND_WND_SIZE)->EnableWindow();
	GetDlgItem(IDC_SEND_INTERVAL)->EnableWindow();
	GetDlgItem(IDC_RESEND_TIMER)->EnableWindow();
	//GetDlgItem(IDC_RANDOM_ERR)->EnableWindow();
	GetDlgItem(IDC_MANUAL_ERR)->EnableWindow();
	GetDlgItem(IDC_CHKSUM_ERR)->EnableWindow();
	GetDlgItem(IDC_FRAME_LOST)->EnableWindow();
	GetDlgItem(IDC_START_SEND)->EnableWindow();
	GetDlgItem(IDC_STOP_SEND)->EnableWindow(FALSE);
}


void CSenderDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	if (m_pOutBuf != NULL)
		delete []m_pOutBuf;
	m_pOutBuf = NULL;

	CDialog::OnCancel();
}


void CSenderDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	CWnd* pListBox = GetDlgItem(IDC_LIST_OUTPUT);
	if (pWnd == pListBox) //等于 if (pWnd == &m_ListOutput)
	{
		CMenu PopupMenu;
		PopupMenu.LoadMenu(IDR_MENU_POPUP);
		PopupMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, this);
	}	
}

void CSenderDlg::OnMenuCleanup() 
{
	// TODO: Add your command handler code here
	if (m_ListOutput.GetCount() > 0)
		m_ListOutput.ResetContent();
}

