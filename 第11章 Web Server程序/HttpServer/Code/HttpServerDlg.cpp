// HttpServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HttpServer.h"
#include "HttpServerDlg.h"
#include <process.h>

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
// CHttpServerDlg dialog

CHttpServerDlg::CHttpServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHttpServerDlg::IDD, pParent)
	, dwReceived(0)
	, dwTransferred(0)
{
	//{{AFX_DATA_INIT(CHttpServerDlg)
	m_nPort = 8000;
	m_strRootDir = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHttpServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHttpServerDlg)
	DDX_Text(pDX, IDC_PORT, m_nPort);
	DDV_MinMaxUInt(pDX, m_nPort, 0, 65535);
	DDX_Text(pDX, IDC_ROOTDIR, m_strRootDir);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_RECEIVED, dwReceived);
	DDX_Text(pDX, IDC_TRANSFERRED, dwTransferred);
}

BEGIN_MESSAGE_MAP(CHttpServerDlg, CDialog)
	//{{AFX_MSG_MAP(CHttpServerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START_STOP, OnStartStop)

	//}}AFX_MSG_MAP	
	ON_MESSAGE(LOG_MSG, AddLog)
	ON_MESSAGE(DATA_MSG, ShowData)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHttpServerDlg message handlers

BOOL CHttpServerDlg::OnInitDialog()
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
	m_AppKey = "HTTPSRVM";
	m_PortKey = "PORT";
	m_DirKey = "ROOTDIR";

	m_bStart = false;
	pHttpProtocol = NULL;

	char szText[256];
	// 读取端口号
	int port;
	port = GetProfileInt(m_AppKey, m_PortKey, 0);
	if(port)
	{
		m_nPort = port;
		UpdateData(false);
	}

	// 读取传输文件路径
	GetProfileString(m_AppKey, m_DirKey, "", szText, sizeof(szText));
	SetDlgItemText(IDC_ROOTDIR, szText);

	m_strRootDir.Format("%s", szText);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHttpServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CHttpServerDlg::OnPaint() 
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
HCURSOR CHttpServerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CHttpServerDlg::OnStartStop() 
{
	// TODO: Add your control notification handler code here

	CWnd* pWndButton = GetDlgItem(IDC_START_STOP);	
	char szText[256];

	//	保存端口号
	GetDlgItemText(IDC_PORT, szText, sizeof(szText));
	WriteProfileString(m_AppKey, m_PortKey, szText);

	//	保存传输文件路径
	GetDlgItemText(IDC_ROOTDIR, szText, sizeof(szText));
	WriteProfileString(m_AppKey, m_DirKey, szText);

	if ( !m_bStart )
	{     
    	// 读写注册表恢复上次退出时的设置 	
		m_nPort = GetProfileInt(m_AppKey, m_PortKey, 0);			// 端口号
		GetProfileString(m_AppKey, m_DirKey, "", szText, sizeof(szText));	// 传输文件	
		m_strRootDir.Format("%s", szText);

		pHttpProtocol = new CHttpProtocol;
		pHttpProtocol->m_strRootDir = m_strRootDir;
		pHttpProtocol->m_nPort = m_nPort;
		pHttpProtocol->m_hwndDlg = m_hWnd;

		if (pHttpProtocol->StartHttpSrv())
		{
			pWndButton->SetWindowText( "结束");
			m_bStart = true;
		}
		else
		{
			if(pHttpProtocol)
			{
				delete pHttpProtocol;
				pHttpProtocol = NULL;
			}
		}
	}
	else
	{
		pHttpProtocol->StopHttpSrv();	
		pWndButton->SetWindowText( "开始");
		if(pHttpProtocol)
		{
			delete pHttpProtocol;
			pHttpProtocol = NULL;
		}

		m_bStart = false;
	}
}

void CHttpServerDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
 	if (m_bStart)	
	{
		pHttpProtocol->StopHttpSrv();
	}
	if(pHttpProtocol)
	{
		delete pHttpProtocol;
		pHttpProtocol = NULL;
	}

	m_bStart = false;

	CDialog::OnCancel();
}

// 显示日志信息
void CHttpServerDlg::AddLog(WPARAM wParam, LPARAM lParam)
{
	char szBuf[284];
	CString *strTemp = (CString *)wParam; 
	CListBox* pListBox;
	pListBox = (CListBox*) GetDlgItem(IDC_LOG);
	SYSTEMTIME st;
	GetLocalTime(&st);
    wsprintf(szBuf,"%02d:%02d:%02d.%03d   %s", st.wHour, st.wMinute, st.wSecond, 
		st.wMilliseconds, *strTemp);
	pListBox->AddString(szBuf);
	delete strTemp;
	strTemp = NULL;
}

// 显示接收和发送的数据流量
void CHttpServerDlg::ShowData(WPARAM wParam, LPARAM lParam)
{
	PHTTPSTATS pStats = (PHTTPSTATS)wParam;
	dwReceived += pStats->dwRecv;
	dwTransferred += pStats->dwSend;

	TRACE1("Rev %d\n", pStats->dwRecv);
	TRACE1("Send %d\n", pStats->dwSend);
	TRACE1("Total Rev %d\n", dwReceived);
	TRACE1("Total Send %d\n", dwTransferred);

	UpdateData(false);
}
