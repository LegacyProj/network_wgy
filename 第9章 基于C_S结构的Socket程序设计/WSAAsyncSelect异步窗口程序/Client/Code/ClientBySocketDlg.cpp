#include "stdafx.h"
#include "ClientBySocket.h"
#include "ClientBySocketDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
ulint ulSeleIP = 0;
char* getUDP = "GET UDP PORT";
char* getTIME = "GET CUR TIME";
char rubsh[1024];

uchar gettime = 0;
uchar getudp = 0;
uchar gotudp = 0;
uchar connected = 0;
DWORD serv_ip;

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
// CClientBySocketDlg dialog

CClientBySocketDlg::CClientBySocketDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClientBySocketDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClientBySocketDlg)
	m_udptxt = _T("");
	m_catlog_txt = _T("");
	m_conn_to_port = 0;
	m_curlog = _T("");
	m_curstat = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientBySocketDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClientBySocketDlg)
	DDX_Control(pDX, IDC_IPCONN, m_ipconn);
	DDX_Control(pDX, IDC_CATLOG, m_catlog);
	DDX_Control(pDX, IDC_IPLIST, m_iplist);
	DDX_Text(pDX, IDC_UDPTXT, m_udptxt);
	DDV_MaxChars(pDX, m_udptxt, 100);
	DDX_Text(pDX, IDC_CATLOG, m_catlog_txt);
	DDX_Text(pDX, IDC_PORT, m_conn_to_port);
	DDX_Text(pDX, IDC_CURLOG, m_curlog);
	DDX_Text(pDX, IDC_CURSTAT, m_curstat);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClientBySocketDlg, CDialog)
	//{{AFX_MSG_MAP(CClientBySocketDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_UDPECHO, On_UDPECHO)
	ON_BN_CLICKED(IDC_TCPTIME, On_TCPTIME)
	ON_BN_CLICKED(IDC_UDPSEND, On_UDP_SEND_CLK)
	ON_BN_CLICKED(ID_CONN, On_CONN)
	ON_BN_CLICKED(IDC_DISCONN, On_Disconn)
	ON_MESSAGE(NETWORK_EVENT, OnNetEvent)
	ON_MESSAGE(NETWORK_LOG, OnLogDisp)
	ON_BN_CLICKED(IDCLOSE, On_Close)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClientBySocketDlg message handlers

BOOL CClientBySocketDlg::OnInitDialog()
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
	rmot_udp_addr_len = sizeof(rmot_udp_addr);
	memset(&rmot_udp_addr, 0, rmot_udp_addr_len);
	nSele = 0;
	InitNetwork(); //初始化网络
	while (pHost->h_addr_list[nSele++]) {
		m_iplist.AddString(ipTostr(*(ulint*)(pHost->h_addr_list[nSele - 1])));
	}
	m_iplist.SetCurSel(0);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CClientBySocketDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CClientBySocketDlg::OnPaint() 
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
HCURSOR CClientBySocketDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CClientBySocketDlg::On_UDPECHO() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_UDPTXT)->EnableWindow(TRUE);
	GetDlgItem(IDC_UDPSEND)->EnableWindow(TRUE);
}

void CClientBySocketDlg::On_TCPTIME() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_UDPTXT)->EnableWindow(FALSE);
	GetDlgItem(IDC_UDPSEND)->EnableWindow(FALSE);
	recv(Sock, rubsh, sizeof(rubsh), 0);//为保证兼容性清空SOCKET流剩余缓冲，因为只用了2个字节
	SendMessage(NETWORK_LOG, LOG_send_itm, LPARAM(getTIME));//写日志
	gettime = 1;
	send(Sock, getTIME, strlen(getTIME) + 1, 0);
}

BOOL CClientBySocketDlg::InitNetwork()
{
	//初始化winsock动态库
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0){
		MessageBox("无法初始化SOCKET!", "SOCKET", MB_OK);
		SendMessage(WM_CLOSE);
	}
	//初始化Raw Socket，在初始化时测试socket是否能正常建立
	if ((Sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == SOCKET_ERROR){
		MessageBox("建立SOCKET出错!", "SOCKET", MB_OK);
		closesocket(Sock);
		WSACleanup();
		SendMessage(WM_CLOSE);
	}
	closesocket(Sock);

	//获取本机名 
	if (gethostname((char*)LocalName, sizeof(LocalName)) == SOCKET_ERROR){
		MessageBox("无法取得本机名!", "SOCKET", MB_OK);
		SendMessage(WM_CLOSE);
	}
	//获取本地IP地址 
	if ((pHost = gethostbyname((char*)LocalName)) == NULL){
		MessageBox("无法取得IP地址!", "SOCKET", MB_OK);
		SendMessage(WM_CLOSE);
	}
	return TRUE;
}

void CClientBySocketDlg::On_UDP_SEND_CLK() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);//取回界面内容到内部变量
	SendMessage(NETWORK_LOG, LOG_send_str, LPARAM(&m_udptxt));//写日志
	sendto(Sock_U, m_udptxt, strlen(m_udptxt), 0, (PSOCKADDR)&serv_udp_addr, sizeof(serv_udp_addr));
}

void CClientBySocketDlg::On_CONN() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);	//取界面数据到变量
	gettime = 0;
	getudp = 0;
	gotudp = 0;
	connected = 0;
	GetDlgItem(ID_CONN)->EnableWindow(FALSE);
	GetDlgItem(IDC_IPLIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_IPCONN)->EnableWindow(FALSE);
	GetDlgItem(IDC_PORT)->EnableWindow(FALSE);
	
	Sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	nSele = m_iplist.GetCurSel();
	addr_in.sin_addr  = *(in_addr*)pHost->h_addr_list[nSele]; //IP
	ulSeleIP = *(ulint*)(pHost->h_addr_list[nSele]);
	addr_in.sin_family = AF_INET; 
	addr_in.sin_port  = 0;	//对客户端让系统自动分配空闲端口
	if (bind(Sock, (PSOCKADDR)&addr_in, sizeof(addr_in)) == SOCKET_ERROR){ 
		MessageBox("绑定网卡出错!", "SOCKET", MB_OK);
		closesocket(Sock);
		return;
	}
	if ((WSAGetLastError()) == 10013) {
		MessageBox("SOCKET建立失败!", "SOCKET", MB_OK);
		closesocket(Sock);
		return;
	}
	
	//将SeverSock设置为异步非阻塞模式，并为它注册各种网络异步事件
	//m_hWnd为应用程序的主对话框或主窗口的句柄
	m_ipconn.GetAddress(serv_ip);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(m_conn_to_port);
	serv_addr.sin_addr.S_un.S_addr = htonl(serv_ip);

	if(WSAAsyncSelect(Sock, m_hWnd, NETWORK_EVENT, FD_CONNECT) == SOCKET_ERROR){//先注册connect事件，处理连接，连上后再注册其余事件
		MessageBox("注册TCP网络异步事件失败!");
		closesocket(Sock);
		return;
	}
		
	SendMessage(NETWORK_LOG, LOG_conn_ing, htonl(serv_ip));
	
	connect(Sock, (LPSOCKADDR)&serv_addr, sizeof(serv_addr));//发送连接请求，事件将响应到OnConnect事件
}

void CClientBySocketDlg::OnNetEvent(WPARAM wParam, LPARAM lParam)
{
	//得到网络事件类型
	int iEvent = WSAGETSELECTEVENT(lParam);
	//得到发生此事件的客户端套接字
	SOCKET CurSock = (SOCKET)wParam;
	switch(iEvent)
	{
	case FD_CONNECT: //Connection or multi-point join operation initiated on socket completed
		OnConnect(CurSock, lParam != FD_CONNECT);
		break;
	case FD_CLOSE: //Connection identified by socket has been closed
		OnClose(CurSock);
		break;
	case FD_READ: //Socket ready for reading
		OnReceive(CurSock);
		break;
	case FD_WRITE: //Socket ready for writing
		OnSend(CurSock);
		break;
	default: break;
	}
}

void CClientBySocketDlg::OnClose(SOCKET CurSock)//对端Socket断开
{
	/* 结束与服务端的通信，释放相应资源 */
	UpdateData(TRUE); //因为后边要送字符到界面，所以先把界面内容取回变量
	closesocket(Sock);
	MessageBox("TCP连接已断开!");
	closesocket(Sock_U);
	m_curstat = "远端服务已断开";
	UpdateData(FALSE);
	SendMessage(NETWORK_LOG, LOG_conn_cls, 0);
	OnResum();
}

void CClientBySocketDlg::OnSend(SOCKET CurSock)//可以发送网络数据包了，即相应socket可写
{
	//在给服务端发数据时做相关预处理
	UpdateData(TRUE);//因为后边要送字符到界面，所以先把界面内容取回变量
	m_curstat = "发送数据";
	UpdateData(FALSE);
	if (!gotudp && CurSock == Sock && connected) OnGetudpPort(CurSock);//首先要获取udp端口号
	if ((CurSock == Sock_U) && gotudp) {
		recvfrom(CurSock, rubsh, sizeof(rubsh), 0, (PSOCKADDR)&rmot_udp_addr, &rmot_udp_addr_len);//将UDP队列清空，因为：有消息到事件尚未注册，所以在此清空是安全的
		WSAAsyncSelect(CurSock, m_hWnd, NETWORK_EVENT, FD_READ);//若宣告udp已可写，则才、仅注册消息到达事件
		GetDlgItem(IDC_UDPECHO)->EnableWindow(TRUE);//仅在获得udp端口号情况下才使按udp消息发送钮可用
	}
}

void CClientBySocketDlg::OnReceive(SOCKET CurSock)//有网络数据包到达，可以处理
{
	/* 读出网络缓冲区中的数据包 */
	UpdateData(TRUE); //因为后边要送字符到界面，所以先把界面内容取回变量
	m_curstat = "接收数据";
	if ((CurSock == Sock_U) && gotudp && connected) { //收UDP数据仅在远端UDP端口号获得后的情况下
		OnReceiveUDP(CurSock);
		m_curstat = "接收UDP数据";
	}
	if (CurSock == Sock && connected) {
		OnReceiveTCP(CurSock);
		recv(CurSock, rubsh, sizeof(rubsh), 0);//保证清空SOCKET流剩余缓冲
		m_curstat = "接收TCP数据";
	}
	UpdateData(FALSE);
}

void CClientBySocketDlg::OnConnect(SOCKET CurSock, int error)//客户端连接请求
{
	/* 建立与服务端的的通信，取得UDP端口号 */
	UpdateData(TRUE); //因为后边要送字符到界面，所以先把界面内容取回变量
	m_curstat = "连接失败";
	if (error) {
		closesocket(CurSock);
		SendMessage(NETWORK_LOG, LOG_conn_ing, 0);
		OnResum();
	}
	else {
		m_curstat = "连接成功";
		SendMessage(NETWORK_LOG, LOG_conn_suc, htonl(serv_ip));		
		SendMessage(NETWORK_LOG, LOG_send_itm, LPARAM(getUDP));//写日志
		GetDlgItem(IDC_TCPTIME)->EnableWindow(TRUE);//连接成功，则功能可用
		GetDlgItem(IDC_DISCONN)->EnableWindow(TRUE);
		SendMessage(NETWORK_LOG, 555, 0);
		connected = 1;
		WSAAsyncSelect(Sock, m_hWnd, NETWORK_EVENT, FD_CLOSE | FD_READ | FD_WRITE);//连接成功才注册：关闭，有消息到，写准备事件
	}
	UpdateData(FALSE);
}

void CClientBySocketDlg::OnLogDisp(WPARAM wParam, LPARAM lParam) //日志输出函数
{
	UpdateData(TRUE);
	int iEvent = int(wParam);
	string catlogAdd = "";
	char port[20];
	char* getITM = (char*)lParam;
	switch(iEvent)
	{
	case LOG_conn_ing: //尝试连接
		if (lParam) {
			catlogAdd += "正在连接到服务器：";
			catlogAdd += ipTostr(ulint(lParam));
			catlogAdd += " -> 端口：";
			itoa(usint(m_conn_to_port), port, 10);
			catlogAdd += port;
		}
		else {
			catlogAdd += "连接失败!请检查地址及端口号并确认存在指定服务";
			OnResum();
		}
		break;
	case LOG_conn_suc: //连接成功
		catlogAdd += "成功连接到服务器：";
		catlogAdd += ipTostr(ulint(lParam));
		catlogAdd += " -> 端口：";
		itoa(usint(m_conn_to_port), port, 10);
		catlogAdd += port;
		m_curstat = "已连接";
		UpdateData(FALSE);
		break;
	case LOG_send_itm:	//各类请求
		catlogAdd += "发送TCP请求：";
		catlogAdd += getITM;
		break;
	case LOG_send_str: //发送字符串
		catlogAdd += "发送UDP消息：";
		catlogAdd += *(CString*)lParam;
		break;
	case LOG_recv_upt: //获取UDP端口　
		catlogAdd += "通过TCP获得服务器UDP端口号：";
		itoa(usint(lParam), port, 10);
		catlogAdd += port;
		break;
	case LOG_recv_str://接收字符串
		catlogAdd += "收到消息";
		catlogAdd += *(CString*)lParam;
		break;
	case LOG_conn_cls: //关闭连接
		catlogAdd += "TCP连接已断开！服务器主动中止了此次连接";			
		break;
	case LOG_recv_tim: //同步时间
		catlogAdd += "通过TCP获得服务器时间：";
		catlogAdd += getITM;
		free(getITM); //赋完值后将前面申请的空间释放
		break;
	case LOG_recv_udp: //取得正确UDP回送
		itoa(ntohs(serv_udp_addr.sin_port), port, 10);
		catlogAdd += "通过UDP获得来自：";
		catlogAdd += ipTostr(serv_udp_addr.sin_addr.S_un.S_addr);
		catlogAdd += ":";
		catlogAdd += port;
		catlogAdd += " 回送的消息：";
		catlogAdd += getITM;
		break;
	default: break;
	}	
	m_curlog = catlogAdd.c_str();
	UpdateData(FALSE);
	catlogAdd += "\r\n";
	m_catlog.SetSel(m_catlog_txt.GetLength(), m_catlog_txt.GetLength(), 0);//光标选择最尾
	m_catlog.ReplaceSel(catlogAdd.c_str());
	UpdateData(TRUE);
	m_catlog.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);//自动滚动到日志最后
}

void CClientBySocketDlg::On_Disconn() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	closesocket(Sock);
	closesocket(Sock_U);
	m_curstat = "断开连接";
	UpdateData(FALSE);
	SendMessage(NETWORK_LOG, 555, 0);
	OnResum();
}

void CClientBySocketDlg::OnResum() //连接中断重置
{
	gettime = 0;
	getudp = 0;
	gotudp = 0;
	connected = 0;
	GetDlgItem(IDC_DISCONN)->EnableWindow(FALSE);
	GetDlgItem(IDC_TCPTIME)->EnableWindow(FALSE);
	GetDlgItem(IDC_UDPECHO)->EnableWindow(FALSE);
	GetDlgItem(IDC_UDPTXT)->EnableWindow(FALSE);
	GetDlgItem(IDC_UDPSEND)->EnableWindow(FALSE);
	GetDlgItem(ID_CONN)->EnableWindow(TRUE);
	GetDlgItem(IDC_IPLIST)->EnableWindow(TRUE);
	GetDlgItem(IDC_IPCONN)->EnableWindow(TRUE);
	GetDlgItem(IDC_PORT)->EnableWindow(TRUE);
}

void CClientBySocketDlg::On_Close() 
{
	// TODO: Add your control notification handler code here
	if (Sock) closesocket(Sock);
	if (Sock_U) closesocket(Sock_U);
	WSACleanup();
	SendMessage(WM_CLOSE);
}

void CClientBySocketDlg::OnGetudpPort(SOCKET s)//请求获得端口号
{
	recv(s, rubsh, sizeof(rubsh), 0); //清空SOCKET流剩余缓冲
	if (connected) {
		send(s, getUDP, strlen(getUDP) + 1, 0);
		getudp = 1;
	}
}

void CClientBySocketDlg::OnReceiveUDP(SOCKET s)//收UDP包
{
	memset(rubsh, 0, strlen(rubsh));
	CString err_msg = "";
	recvfrom(s, rubsh, sizeof(rubsh), 0, (PSOCKADDR)&rmot_udp_addr, &rmot_udp_addr_len);
	if ((rmot_udp_addr.sin_port == serv_udp_addr.sin_port) && (rmot_udp_addr.sin_addr.S_un.S_addr == serv_udp_addr.sin_addr.S_un.S_addr) && !(strcmp(m_udptxt, rubsh))){//判断是否从正确的源头回送的与发送一致的用户报
		SendMessage(NETWORK_LOG, LOG_recv_udp, LPARAM(rubsh));//写日志
	}
	else {
		err_msg = "(被动UDP)：";
		err_msg += rubsh;
		SendMessage(NETWORK_LOG, LOG_recv_str, LPARAM(&err_msg));//写日志
	}
}

void CClientBySocketDlg::OnReceiveTCP(SOCKET s)//收TCP数据
{
	if (getudp && !gotudp) OnGotudpPort(s);
	if (gettime) OnGetTime(s);
}

void CClientBySocketDlg::OnGetTime(SOCKET s)//收TCP时间
{
	gettime = 0;
	char* recvData = (char*)malloc(9);
	memset(recvData, 0, 9);
	recv(s, recvData, 8, 0);
	SendMessage(NETWORK_LOG, LOG_recv_tim, LPARAM(recvData));
}

void CClientBySocketDlg::OnGotudpPort(SOCKET s)//获知UDP端口号
{
	getudp = 0;
	char recvData[2];
	recv(s, recvData, sizeof(recvData), 0);
	memcpy(&servUDPort, &recvData, 2);
	SendMessage(NETWORK_LOG, LOG_recv_upt, ntohs(servUDPort));
	serv_udp_addr = serv_addr;
	serv_udp_addr.sin_port = servUDPort;
	rmot_udp_addr = serv_udp_addr;
	Sock_U = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	bind(Sock_U, (PSOCKADDR)&addr_in, sizeof(addr_in));
	if(WSAAsyncSelect(Sock_U, m_hWnd, NETWORK_EVENT, FD_WRITE) == SOCKET_ERROR){//对udp先注册：可写准备事件，再作其他处理
		MessageBox("注册UDP网络异步事件失败!");
		closesocket(Sock);
		closesocket(Sock_U);
		return;
	}
	gotudp = 1;
}