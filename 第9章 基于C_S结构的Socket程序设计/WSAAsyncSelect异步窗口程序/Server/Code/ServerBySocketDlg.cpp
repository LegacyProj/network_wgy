#include "stdafx.h"
#include "ServerBySocket.h"
#include "ServerBySocketDlg.h"

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
char recvbuff[1024];
char timestr[9];

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
// CServerBySocketDlg dialog

CServerBySocketDlg::CServerBySocketDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerBySocketDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServerBySocketDlg)
	m_port = 0;
	m_connstat = _T("");
	m_curlog = _T("");
	m_catlog_txt = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerBySocketDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServerBySocketDlg)
	DDX_Control(pDX, IDC_CATLOG, m_catlog);
	DDX_Control(pDX, IDC_IPlist, m_iplist);
	DDX_Text(pDX, IDC_PORT, m_port);
	DDV_MinMaxInt(pDX, m_port, 1024, 65535);
	DDX_Text(pDX, IDC_CONNSTAT, m_connstat);
	DDX_Text(pDX, IDC_LOGSTAT, m_curlog);
	DDX_Text(pDX, IDC_CATLOG, m_catlog_txt);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CServerBySocketDlg, CDialog)
	//{{AFX_MSG_MAP(CServerBySocketDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCLOSE, OnClose)
	ON_BN_CLICKED(IDC_STOPSERV, On_Stopserv)
	ON_BN_CLICKED(IDC_STARTSERV, On_Startserv)
	ON_MESSAGE(NETWORK_EVENT, OnNetEvent)
	ON_MESSAGE(NETWORK_LOG, OnLogDisp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerBySocketDlg message handlers

BOOL CServerBySocketDlg::OnInitDialog()
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
	memset(&ClientSock, 0, sizeof(ClientSock));		//将连接客户SOCK的结构体置0
	memset(&ClientSock_U, 0, sizeof(ClientSock_U));	//将UDP对端返回结构体置0
	U_port = htons(UDP_PORT);
	nSele = 0;
	InitNetwork(); //初始化网络
	while (pHost->h_addr_list[nSele++]) {
		m_iplist.AddString(ipTostr(*(ulint*)(pHost->h_addr_list[nSele - 1])));
	}
	m_iplist.SetCurSel(0);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CServerBySocketDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CServerBySocketDlg::OnPaint() 
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
HCURSOR CServerBySocketDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CServerBySocketDlg::InitNetwork()
{
	//初始化winsock动态库
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0){
		MessageBox("无法初始化SOCKET!", "SOCKET", MB_OK);
		SendMessage(WM_CLOSE);
	}
	//初始化Raw Socket，在初始化时测试socket是否能正常建立
	if ((ServerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == SOCKET_ERROR){
		MessageBox("建立SOCKET出错!", "SOCKET", MB_OK);
		closesocket(ServerSock);
		WSACleanup();
		SendMessage(WM_CLOSE);
	}
	closesocket(ServerSock);

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

void CServerBySocketDlg::OnClose() 
{
	// TODO: Add your control notification handler code here
	if (ServerSock) closesocket(ServerSock);
	if (ServerSock_U) closesocket(ServerSock_U);
	WSACleanup();
	SendMessage(WM_CLOSE);	
}

void CServerBySocketDlg::OnNetEvent(WPARAM wParam, LPARAM lParam)
{
	//得到网络事件类型
	int iEvent = WSAGETSELECTEVENT(lParam);
	//得到发生此事件的客户端套接字
	SOCKET CurSock = (SOCKET)wParam;
	switch(iEvent)
	{
	case FD_ACCEPT: //Connection or multi-point join operation initiated on socket completed
		OnAccept(CurSock);
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

void CServerBySocketDlg::OnClose(SOCKET CurSock)
{
	UpdateData(TRUE);
	//结束服务端的通信，释放相应资源
	if (CurSock == ServerSock) {
		CloseClientSocket(ClientSock);
		closesocket(CurSock);
		closesocket(ServerSock_U);
		m_connstat = "服务已停止";
		UpdateData(FALSE);
		SendMessage(NETWORK_LOG, LOG_stop_svr, 5);
		OnResum();
		return;
	}
	//结束与CurSock相应的客户端的通信，释放资源
	ClientSock[FindSockNo(ClientSock, CurSock)].inuse = 0;
	closesocket(CurSock);//关闭相应的CurSock
	total_conn--;
	m_connstat = char(total_conn + 48);
	m_connstat += "个活动连接";
	UpdateData(FALSE);
	SendMessage(NETWORK_LOG, LOG_conn_cls, FindSockNo(ClientSock, CurSock));
}

void CServerBySocketDlg::OnSend(SOCKET CurSock)//发送网络数据包
{
}

void CServerBySocketDlg::OnReceive(SOCKET CurSock)//有网络数据包到达，进行处理
{
	memset(&recvbuff, 0, sizeof(recvbuff));//接收缓冲先清空
	if ((CurSock != ServerSock_U) && (CurSock != ServerSock)) {//tcp响应
		recv(CurSock, recvbuff, sizeof(recvbuff), 0);
		if (!strcmp(recvbuff, getUDP)) {//回送UDP端口
			SendMessage(NETWORK_LOG, LOG_reqe_udp, FindSockNo(ClientSock, CurSock));
			send(CurSock, (const char*)(&U_port), sizeof(U_port), 0);
			SendMessage(NETWORK_LOG, LOG_send_udp, FindSockNo(ClientSock, CurSock));
		}
		if (!strcmp(recvbuff, getTIME)) {//回送服务器时间
			_strtime(timestr);
			SendMessage(NETWORK_LOG, LOG_reqe_tim, FindSockNo(ClientSock, CurSock));
			send(CurSock, timestr, sizeof(timestr), 0);
			SendMessage(NETWORK_LOG, LOG_send_tim, FindSockNo(ClientSock, CurSock));
		}
	}
	else if (CurSock == ServerSock_U) {//从udp接收的原样回送
		recvfrom(CurSock, recvbuff, sizeof(recvbuff), 0, (LPSOCKADDR)&(ClientSock_U.addr), &(ClientSock_U.addr_len));//收UDP包
		SendMessage(NETWORK_LOG, LOG_recv_str, LPARAM(recvbuff));
		sendto(CurSock, recvbuff, sizeof(recvbuff), 0, (LPSOCKADDR)&(ClientSock_U.addr), ClientSock_U.addr_len);//送回文
		SendMessage(NETWORK_LOG, LOG_send_str, LPARAM(recvbuff));
	}
}

void CServerBySocketDlg::OnAccept(SOCKET CurSock)//处理客户端的连接请求
{
	int temp = 0;
	int CurConn = FindFirstEmpty(ClientSock);
	if ((CurConn < CLNT_MAX_NUM) && (total_conn <= CLNT_MAX_NUM)) {
		ClientSock[CurConn].Sock = 0xFFFFFFFF;
		ClientSock[CurConn].Sock = accept(ServerSock, (LPSOCKADDR)&(ClientSock[CurConn].addr), &(ClientSock[CurConn].addr_len));
		temp = WSAGetLastError();
		ClientSock[CurConn].inuse = 1;
		total_conn++;//连接总数加1
		if(WSAAsyncSelect(ClientSock[CurConn].Sock, m_hWnd, NETWORK_EVENT, FD_CLOSE | FD_READ | FD_WRITE) == SOCKET_ERROR){
			MessageBox("注册网络异步事件失败!");
			closesocket(ServerSock);
			closesocket(ServerSock_U);
			OnResum();
			return;
		}
		m_connstat = char(total_conn + 48);
		m_connstat += "个活动连接";
		UpdateData(FALSE);
		SendMessage(NETWORK_LOG, LOG_conn_suc, CurConn);
		return;
	}
	//若已满，则先连（其实不得不连，因为在listen队列里已经接受了），再gracfully地关闭
	SOCKET abandon = accept(ServerSock, (LPSOCKADDR)&(ClientSock_Abandon.addr), &(ClientSock_Abandon.addr_len));
	SendMessage(NETWORK_LOG, LOG_conn_ing, 0);
	closesocket(abandon);
}

void CServerBySocketDlg::OnLogDisp(WPARAM wParam, LPARAM lParam) //日志输出函数
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
			_strtime(timestr);
			catlogAdd += "-*-*-*-*-*-服务器自：";
			catlogAdd += timestr;
			catlogAdd += " 开始服务-*-*-*-*-*-";
		}
		else {
			catlogAdd += "建立来自<-*-";
			catlogAdd += ipTostr(ClientSock_Abandon.addr.sin_addr.S_un.S_addr);
			catlogAdd += ":";
			itoa(ntohs(ClientSock_Abandon.addr.sin_port), port, 10);
			catlogAdd += port;
			catlogAdd += "-*->的连接失败!超过最大连接数!";
		}
		break;
	case LOG_conn_suc: //连接成功
		catlogAdd += "接受来自<-*-";
		catlogAdd += ipTostr(ClientSock[lParam].addr.sin_addr.S_un.S_addr);
		catlogAdd += ":";
		itoa(ntohs(ClientSock[lParam].addr.sin_port), port, 10);
		catlogAdd += port;
		catlogAdd += "-*->的连接请求!";
		break;
	case LOG_reqe_tim:	//时间请求
		catlogAdd += "收到来自<-*-";
		catlogAdd += ipTostr(ClientSock[lParam].addr.sin_addr.S_un.S_addr);
		catlogAdd += ":";
		itoa(ntohs(ClientSock[lParam].addr.sin_port), port, 10);
		catlogAdd += port;
		catlogAdd += "-*->的服务器时间同步请求!";
		break;
	case LOG_send_str: //发送字符串
		catlogAdd += "回送消息<-*-";
		catlogAdd += ipTostr(ClientSock_U.addr.sin_addr.S_un.S_addr);
		catlogAdd += ":";
		itoa(ntohs(ClientSock_U.addr.sin_port), port, 10);
		catlogAdd += port;
		catlogAdd += "-*->(UDP)：";
		catlogAdd += getITM;
		break;
		break;
	case LOG_stop_svr://停止服务
			_strtime(timestr);
			catlogAdd += "-*-*-*-*-*-服务器于：";
			catlogAdd += timestr;
			catlogAdd += " 停止服务-*-*-*-*-*-";
		break;
	case LOG_recv_str://接收字符串
		catlogAdd += "收到来自<-*-";
		catlogAdd += ipTostr(ClientSock_U.addr.sin_addr.S_un.S_addr);
		catlogAdd += ":";
		itoa(ntohs(ClientSock_U.addr.sin_port), port, 10);
		catlogAdd += port;
		catlogAdd += "-*->的UDP消息：";
		catlogAdd += getITM;
		break;
	case LOG_conn_cls: //关闭连接
		catlogAdd += "客户端：<-*-";
		catlogAdd += ipTostr(ClientSock[lParam].addr.sin_addr.S_un.S_addr);
		catlogAdd += ":";
		itoa(ntohs(ClientSock[lParam].addr.sin_port), port, 10);
		catlogAdd += port;
		catlogAdd += "-*->的连接已断开！";
		break;
	case LOG_send_tim: //同步时间
		catlogAdd += "向客户端<-*-";
		catlogAdd += ipTostr(ClientSock[lParam].addr.sin_addr.S_un.S_addr);
		catlogAdd += ":";
		itoa(ntohs(ClientSock[lParam].addr.sin_port), port, 10);
		catlogAdd += port;
		catlogAdd += "-*->发送当前时间:";
		catlogAdd += timestr;
		break;
	case LOG_send_udp: //发送UDP端口　
		catlogAdd += "向客户端<-*-";
		catlogAdd += ipTostr(ClientSock[lParam].addr.sin_addr.S_un.S_addr);
		catlogAdd += ":";
		itoa(ntohs(ClientSock[lParam].addr.sin_port), port, 10);
		catlogAdd += port;
		itoa(UDP_PORT, port, 10);
		catlogAdd += "-*->发送UDP服务端口号:";
		catlogAdd += port;
		break;
	case LOG_reqe_udp: //请求UDP回送
		catlogAdd += "收到来自<-*-";
		catlogAdd += ipTostr(ClientSock[lParam].addr.sin_addr.S_un.S_addr);
		catlogAdd += ":";
		itoa(ntohs(ClientSock[lParam].addr.sin_port), port, 10);
		catlogAdd += port;
		catlogAdd += "-*->的UDP服务端口号获知请求!";
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

void CServerBySocketDlg::On_Stopserv() //停止服务的处理
{
	// TODO: Add your control notification handler code here
	if (ClientSock_U.addr.sin_port) {
		strcpy(recvbuff, "我要停止服务了:)\0");
		closesocket(ServerSock_U);
		sendto(ClientSock_U.Sock, recvbuff, sizeof(recvbuff), 0, (LPSOCKADDR)&(ClientSock_U.addr), ClientSock_U.addr_len);//送中止消息20051123
		Sleep(1);
	}
	shutdown(ServerSock, 2); //优雅中止连接
	SendMessage(NETWORK_EVENT, ServerSock, FD_CLOSE);
}

void CServerBySocketDlg::OnResum()//连接中断重置
{
	GetDlgItem(IDC_STOPSERV)->EnableWindow(FALSE);
	GetDlgItem(IDC_IPlist)->EnableWindow(TRUE);
	GetDlgItem(IDC_PORT)->EnableWindow(TRUE);
	GetDlgItem(IDC_STARTSERV)->EnableWindow(TRUE);
}

void CServerBySocketDlg::On_Startserv() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE); //取界面数据到变量
	if ((m_port < 1024) || (m_port > 65535)) return; //端口不合法则返回
	GetDlgItem(IDC_STARTSERV)->EnableWindow(FALSE);
	GetDlgItem(IDC_IPlist)->EnableWindow(FALSE);
	GetDlgItem(IDC_PORT)->EnableWindow(FALSE);
	
	//建立TCP socket
	ServerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	nSele = m_iplist.GetCurSel();
	addr_in.sin_addr  = *(in_addr*)pHost->h_addr_list[nSele]; //IP
	ulSeleIP = *(ulint*)(pHost->h_addr_list[nSele]);
	addr_in.sin_family = AF_INET; 
	addr_in.sin_port = htons(m_port); //服务器端口号手工指定
	for (int i = 0; i < CLNT_MAX_NUM; i++){
		ClientSock[i].addr_len = sizeof(ClientSock[i].addr);
		ClientSock[i].addr = addr_in;
	}
	total_conn = 0;

	if (bind(ServerSock, (PSOCKADDR)&addr_in, sizeof(addr_in)) == SOCKET_ERROR){ 
		MessageBox("绑定网卡出错!", "SOCKET", MB_OK);
		closesocket(ServerSock);
		OnResum();
		return;
	}
	if ((WSAGetLastError()) == 10013) {
		MessageBox("SOCKET建立失败!", "SOCKET_TCP", MB_OK);
		closesocket(ServerSock);
		return;
	}
	
	//建立UDP socket
	addr_in_udp = addr_in; //地址都是本机，所以直接拿来使用
	addr_in_udp.sin_port = htons(UDP_PORT); //端口

	ServerSock_U = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (bind(ServerSock_U, (PSOCKADDR)&addr_in_udp, sizeof(addr_in_udp)) == SOCKET_ERROR){ 
		MessageBox("绑定网卡出错!", "SOCKET_UDP", MB_OK);
		closesocket(ServerSock);
		closesocket(ServerSock_U);
		OnResum();
		return;
	}

	ClientSock_U.addr_len = sizeof(ClientSock_U.addr);
	ClientSock_U.addr = addr_in_udp;
	ClientSock_U.addr.sin_port = 0;
	ClientSock_Abandon = ClientSock_U;

	//准备工作完成，下面注册TCP的异步响应事件
	if(WSAAsyncSelect(ServerSock, m_hWnd, NETWORK_EVENT, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR){
		MessageBox("注册网络异步事件失败!");
		closesocket(ServerSock);
		closesocket(ServerSock_U);
		OnResum();
		return;
	}

	//注册UDP的异步响应事件
	if(WSAAsyncSelect(ServerSock_U, m_hWnd, NETWORK_EVENT, FD_READ | FD_WRITE) == SOCKET_ERROR){
		MessageBox("注册网络异步事件失败!");
		closesocket(ServerSock);
		closesocket(ServerSock_U);
		OnResum();
		return;
	}

	//开始在TCP上进行侦听
	if (listen(ServerSock, MAX_QUEUE) == SOCKET_ERROR) {
		MessageBox("无法初始化TCP侦听!");
		closesocket(ServerSock);
		closesocket(ServerSock_U);
		OnResum();
		return;		
	}
	
	m_connstat = "正在侦听.....";
	UpdateData(FALSE);
	ClientSock_U.Sock = ServerSock_U;
	GetDlgItem(IDC_STOPSERV)->EnableWindow(TRUE);
	SendMessage(NETWORK_LOG, LOG_conn_ing, 5);
	ShowWindow(SW_MINIMIZE);//最小化到任务栏
}


int CServerBySocketDlg::FindFirstEmpty(client_sock* in)
{
	for (int i = 0; i < CLNT_MAX_NUM; i++) {
		if (!in[i].inuse) return i;
	}
	return CLNT_MAX_NUM;
}

int CServerBySocketDlg::FindSockNo(client_sock* in, SOCKET s)
{
	for (int i = 0; i < CLNT_MAX_NUM; i++) {
		if (in[i].Sock == s) return i;
	}
	return CLNT_MAX_NUM;
}

void CServerBySocketDlg::CloseClientSocket(client_sock* in)
{
	for (int i = 0; i < CLNT_MAX_NUM; i++) {
		if (in[i].inuse) {
			closesocket(in[i].Sock);
			in[i].inuse = 0;
		}
	}
}
