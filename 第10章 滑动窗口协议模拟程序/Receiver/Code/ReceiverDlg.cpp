// ReceiverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Receiver.h"
#include "ReceiverDlg.h"

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
// CReceiverDlg dialog

CReceiverDlg::CReceiverDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReceiverDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReceiverDlg)
	m_RecvWndSize = NR_BUFS;
	m_RecvInterval = 1000;
	m_ErrSetting = 1;
	m_strAckLost = _T("");
	m_strAckChkErr = _T("");
	m_AuxiliaryTime = 2000;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pInBuf = NULL;		//数据接收缓冲区
	m_pArrived = NULL;		//数据接收缓冲区到达包位图
	m_bRecving = FALSE;
}

void CReceiverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReceiverDlg)
	DDX_Control(pDX, IDC_LIST_OUTPUT, m_ListOutput);
	DDX_Text(pDX, IDC_RECV_WND_SIZE, m_RecvWndSize);
	DDV_MinMaxInt(pDX, m_RecvWndSize, 1, NR_BUFS);
	DDX_Text(pDX, IDC_RECV_INTERVAL, m_RecvInterval);
	DDV_MinMaxInt(pDX, m_RecvInterval, 100, 60000);
	DDX_Radio(pDX, IDC_RANDOM_ERR, m_ErrSetting);
	DDX_Text(pDX, IDC_EDIT_LOST_ACK, m_strAckLost);
	DDX_Text(pDX, IDC_EDIT_ERR_ACK, m_strAckChkErr);
	DDX_Text(pDX, IDC_AUX_TIMER, m_AuxiliaryTime);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CReceiverDlg, CDialog)
	//{{AFX_MSG_MAP(CReceiverDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_START_RECV, OnStartRecv)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_STOP_RECV, OnStopRecv)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_MENU_CLEANUP, OnMenuCleanup)
	//}}AFX_MSG_MAP
	ON_MESSAGE(FRAME_ARRIVAL, OnFrameArrival)
	ON_MESSAGE(CKSUM_ERROR, OnCksumErr)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReceiverDlg message handlers

BOOL CReceiverDlg::OnInitDialog()
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

	//创建UDP Receiver Socket
	if ((m_UDPRcvrSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		AfxMessageBox("Failed to create UDPSocket");
		return FALSE;
	}

	//填充本地UDP Receiver Socket地址结构
	SOCKADDR_IN UDPRcvrAddr;
	memset(&UDPRcvrAddr, 0, sizeof(SOCKADDR_IN));
	UDPRcvrAddr.sin_family = AF_INET;
	UDPRcvrAddr.sin_port = htons(3074);
	UDPRcvrAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	//绑定Receiver UDP端口
	if (bind(m_UDPRcvrSocket, (sockaddr*)&UDPRcvrAddr, sizeof(UDPRcvrAddr)) == SOCKET_ERROR )
	{
		AfxMessageBox("Failed to bind UDPRcvrAddr");
		return FALSE;
	}

	//填充Sender UDP地址
	memset(&m_UDPSndrAddr, 0, sizeof(SOCKADDR_IN));
	m_UDPSndrAddr.sin_family = AF_INET;
	m_UDPSndrAddr.sin_port = htons(3073);
	m_UDPSndrAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	//创建UDP数据包接收线程
	DWORD dwThreadId;
	CreateThread(NULL, 0, UdpReceiveThread, this, 0, &dwThreadId);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CReceiverDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CReceiverDlg::OnPaint() 
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
HCURSOR CReceiverDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//UDP接收线程
DWORD CReceiverDlg::UdpReceiveThread(LPVOID lpParam)
{
	CReceiverDlg* pDlg = (CReceiverDlg*)lpParam;
	pDlg->UdpReceive();
	return 0;
}

void CReceiverDlg::UdpReceive()
{
	char buf[sizeof(frame)];
	int BytesReceived;
	int a=0;
	while (TRUE)
	{
		//接收UDP数据包
		if ((BytesReceived=recvfrom(m_UDPRcvrSocket, buf, sizeof(buf), 0, NULL, NULL)) == SOCKET_ERROR)
		{
			TRACE("Failed to recvfrom UDPRcvrSocket, ErrCode: %d\n", WSAGetLastError()); //10054错误是正常的
			//AfxMessageBox("Failed to recvfrom UDPSndrSocket");
			continue;
		}
		ASSERT(BytesReceived>=sizeof(frame_hdr) && BytesReceived<=sizeof(frame));
		if (BytesReceived>=sizeof(frame_hdr) && BytesReceived<=sizeof(frame) && m_bRecving)
		{
			frame* pFrame = (frame*)buf;
			if (pFrame->hdr.err == NO_ERR)
			{
				frame* p = new frame;
				memcpy(p, buf, BytesReceived);
				PostMessage(FRAME_ARRIVAL, (WPARAM)p);
				TRACE("Get Data%d\n", pFrame->hdr.seq);
			}
			else if (pFrame->hdr.err == CKSUM_ERR)
			{
				PostMessage(CKSUM_ERROR, pFrame->hdr.seq);
				TRACE("Get Data%d with cksumErr\n", pFrame->hdr.seq);
			}
			else //pFrame->hdr.err == LOST_ERR
			{
				//Do nothing!
				TRACE("Get Data%d with lostErr\n", pFrame->hdr.seq);
			}
		}
	}
}

void CReceiverDlg::OnStartRecv() 
{
	// TODO: Add your control notification handler code here
	//获取对话框数据
	if (!UpdateData(TRUE))
		return;

	//初始化所有参数
	if (m_pInBuf != NULL)
		delete []m_pInBuf;
	m_pInBuf = new frame[m_RecvWndSize];
	if (m_pArrived != NULL)
		delete []m_pArrived;
	m_pArrived = new BOOL[m_RecvWndSize];
	for (int i=0; i<m_RecvWndSize; i++)
		m_pArrived[i] = FALSE;
	m_frameExpected = 0;		//接收窗口左侧 
	m_tooFar = m_RecvWndSize;	//接收窗口右侧+1
	for (i=0; i<MAX_SEQ+1; i++) //错误模式初始均为NO_ERR
		m_errArray[i] = NO_ERR;
	m_iTokenCount = m_RecvWndSize;
	m_bNoNak = TRUE;
	m_bRecving = TRUE;

	//设置应答帧的错误模式，模拟传输错误。目前只支持手工设定
	char tmp[256];
	char *token;
	char seps[]   = " ";
	int iSeq;
	if (!m_strAckLost.IsEmpty())
	{
		strcpy(tmp, m_strAckLost);
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
	//启动网络层数据接收定时器，模拟接收速率限制
	SetTimer(ID_RECV_TIMER, m_RecvInterval, NULL);

	//窗口界面控制
	GetDlgItem(IDC_RECV_WND_SIZE)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECV_INTERVAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_AUX_TIMER)->EnableWindow(FALSE);
	GetDlgItem(IDC_RANDOM_ERR)->EnableWindow(FALSE);
	GetDlgItem(IDC_MANUAL_ERR)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_ERR_ACK)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_LOST_ACK)->EnableWindow(FALSE);
	GetDlgItem(ID_START_RECV)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP_RECV)->EnableWindow();
	CString strMsg;
	strMsg.Format("%d", m_iTokenCount);
	GetDlgItem(IDC_CUR_TOKEN)->SetWindowText(strMsg);
	strMsg.Format("%d", m_frameExpected);
	GetDlgItem(IDC_BOTTOM)->SetWindowText(strMsg);
	strMsg.Format("%d", m_tooFar);
	GetDlgItem(IDC_TOP)->SetWindowText(strMsg);
	GetDlgItem(IDC_CUR_RCVD)->SetWindowText("0");

}

void CReceiverDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	CString strMsg;
	switch(nIDEvent)
	{
	case ID_RECV_TIMER:
		if (m_iTokenCount < m_RecvWndSize) //令牌桶算法，最大令牌数为接收窗口大小
			m_iTokenCount++;
		//界面显示
		strMsg.Format("%d", m_iTokenCount);
		GetDlgItem(IDC_CUR_TOKEN)->SetWindowText(strMsg);
		break;

	case ID_ACK_TIMER:
		SendFrame(ACK, (m_frameExpected+MAX_SEQ)%(MAX_SEQ+1));
		break;
	default:
		ASSERT(0);
		break;
	}

	CDialog::OnTimer(nIDEvent);
}

LRESULT CReceiverDlg::OnFrameArrival(WPARAM wParam, LPARAM lParam)
{
	frame dataFrame;
	FromPhysicalLayer((char*)&dataFrame, (char*)wParam);
	ASSERT(dataFrame.hdr.kind == DATA);

	//界面显示
	TRACE("arrived Data%d normal\n", dataFrame.hdr.seq);
	char buf[32];
	_strtime(buf);
	CString strMsg;
	strMsg.Format("%s %5s %8s %7s %3d  token:%d", buf, "data", "arrived", "normal", dataFrame.hdr.seq, m_iTokenCount);
	m_ListOutput.AddString(strMsg);
	int iCount = m_ListOutput.GetCount();
	if (iCount > 0)
		m_ListOutput.SetCurSel(iCount-1);
	strMsg.Format("%d", dataFrame.hdr.seq);
	GetDlgItem(IDC_CUR_RCVD)->SetWindowText(strMsg);
	
	if ((dataFrame.hdr.seq != m_frameExpected) && m_bNoNak)
		SendFrame(NAK, m_frameExpected);
	else
		SetTimer(ID_ACK_TIMER, m_AuxiliaryTime, NULL); //ACK发送定时器

	int iPos = dataFrame.hdr.seq % m_RecvWndSize;
	if (Between(m_frameExpected, dataFrame.hdr.seq, m_tooFar) && 
		(m_pArrived[iPos] == FALSE)  && m_iTokenCount)
	{
		m_pArrived[iPos] = TRUE;
		memcpy(&m_pInBuf[iPos], &dataFrame, sizeof(frame));
		while (m_pArrived[m_frameExpected % m_RecvWndSize] && m_iTokenCount)
		{
			//ToNetworkLayer(&(m_pInBuf[m_frameExpected % m_RecvWndSize].info));
			ToNetworkLayer(m_frameExpected); //仅为了打印日志
			m_bNoNak = TRUE;
			m_pArrived[m_frameExpected % m_RecvWndSize] = FALSE;
			Inc(m_frameExpected);
			Inc(m_tooFar);
			SetTimer(ID_ACK_TIMER, m_AuxiliaryTime, NULL);
			m_iTokenCount--;
			
			//界面显示
			CString strMsg;
			strMsg.Format("%d", m_iTokenCount);
			GetDlgItem(IDC_CUR_TOKEN)->SetWindowText(strMsg);
			strMsg.Format("%d", m_frameExpected);
			GetDlgItem(IDC_BOTTOM)->SetWindowText(strMsg);
			strMsg.Format("%d", m_tooFar);
			GetDlgItem(IDC_TOP)->SetWindowText(strMsg);
		}
	}
	else
	{
		//界面显示
		TRACE("Discard Data%d\n", dataFrame.hdr.seq);
		char buf[32];
		_strtime(buf);
		CString strMsg;
		strMsg.Format("%s %5s %8s %7s %3d", buf, "data", "discard", "normal", dataFrame.hdr.seq);
		m_ListOutput.AddString(strMsg);
		int iCount = m_ListOutput.GetCount();
		if (iCount > 0)
			m_ListOutput.SetCurSel(iCount-1);
	}

	return 0;
}

void CReceiverDlg::ToNetworkLayer(int iSeqNo)
{
	char buf[32];
	_strtime(buf);
	CString strMsg;
	strMsg.Format("%s %5s %8s %7s %3d", buf, "data", "accepted", "normal", iSeqNo);
	m_ListOutput.AddString(strMsg);
	int iCount = m_ListOutput.GetCount();
	if (iCount > 0)
		m_ListOutput.SetCurSel(iCount-1);
}

void CReceiverDlg::FromPhysicalLayer(char* p, char* pFromPhysical)
{
	memcpy(p, pFromPhysical, sizeof(frame));
	delete (frame*)pFromPhysical;
}

void CReceiverDlg::SendFrame(frameKind fk, seqNum ack)
{
	frame ackFrame;
	ackFrame.hdr.kind = fk;
	ackFrame.hdr.ack = ack;
	ackFrame.hdr.err = m_errArray[ack]; //应答帧出错模拟只适用第一轮
	m_errArray[ack] = NO_ERR;
	if (fk == NAK)
		m_bNoNak = FALSE;
	ToPhysicalLayer((char*)&ackFrame, sizeof(ackFrame.hdr));
	KillTimer(ID_ACK_TIMER);

	//窗口显示格式
	//12:2:33 data arrived normal  0
	//12:2:34 ack     sent normal  0
	//12:2:35 ack     sent lost    1
	//12:2:36 nak     sent normal  2
	char buf[32];
	_strtime(buf);
	CString strMsg;
	strMsg.Format("%s %5s %8s %7s %3d", buf, (ackFrame.hdr.kind==ACK)?"ack":"nak", "sent", (ackFrame.hdr.err==NO_ERR)?"normal":"lost", ackFrame.hdr.ack);
	m_ListOutput.AddString(strMsg);
	int iCount = m_ListOutput.GetCount();
	if (iCount > 0)
		m_ListOutput.SetCurSel(iCount-1);
}

void CReceiverDlg::ToPhysicalLayer(char* pBuf, int iSize)
{
	sendto(m_UDPRcvrSocket, pBuf, iSize, 0, (sockaddr*)&m_UDPSndrAddr, sizeof(m_UDPSndrAddr));
}


LRESULT CReceiverDlg::OnCksumErr(WPARAM wParam, LPARAM lParam)
{
	//界面显示
	TRACE("arrived Data%d cksumErr\n", wParam);
	char buf[32];
	_strtime(buf);
	CString strMsg;
	strMsg.Format("%s %5s %8s %7s %3d", buf, "data", "arrived", "chkerr", wParam);
	m_ListOutput.AddString(strMsg);
	int iCount = m_ListOutput.GetCount();
	if (iCount > 0)
		m_ListOutput.SetCurSel(iCount-1);
	strMsg.Format("%d", wParam);
	GetDlgItem(IDC_CUR_RCVD)->SetWindowText(strMsg);

	if (m_bNoNak)
		SendFrame(NAK, m_frameExpected);
	return 0;
}

void CReceiverDlg::OnStopRecv() 
{
	// TODO: Add your control notification handler code here
	m_bRecving = FALSE;

	//窗口界面控制
	GetDlgItem(IDC_RECV_WND_SIZE)->EnableWindow();
	GetDlgItem(IDC_RECV_INTERVAL)->EnableWindow();
	GetDlgItem(IDC_AUX_TIMER)->EnableWindow();
	//GetDlgItem(IDC_RANDOM_ERR)->EnableWindow();
	GetDlgItem(IDC_MANUAL_ERR)->EnableWindow();
	//GetDlgItem(IDC_EDIT_ERR_ACK)->EnableWindow();
	GetDlgItem(IDC_EDIT_LOST_ACK)->EnableWindow();
	GetDlgItem(ID_START_RECV)->EnableWindow();
	GetDlgItem(IDC_STOP_RECV)->EnableWindow(FALSE);
}

void CReceiverDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	if (m_pInBuf != NULL)
		delete []m_pInBuf;
	m_pInBuf = NULL;
	if (m_pArrived != NULL)
		delete []m_pArrived;
	m_pArrived = NULL;
	
	CDialog::OnCancel();
}

void CReceiverDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
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

void CReceiverDlg::OnMenuCleanup() 
{
	// TODO: Add your command handler code here
	if (m_ListOutput.GetCount() > 0)
		m_ListOutput.ResetContent();

}
