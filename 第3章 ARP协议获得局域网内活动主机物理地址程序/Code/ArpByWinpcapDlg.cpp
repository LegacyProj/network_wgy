
#include "stdafx.h"
#include "ArpByWinpcap.h"
#include "ArpByWinpcapDlg.h"
#include "arp.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
// CArpByWinpcapDlg dialog

CArpByWinpcapDlg::CArpByWinpcapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CArpByWinpcapDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CArpByWinpcapDlg)
	m_Dev_No = 0;
	m_sending = _T("");
	m_count = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CArpByWinpcapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CArpByWinpcapDlg)
	DDX_Control(pDX, IDC_MAC_LIST, m_Mac_list);
	DDX_Control(pDX, IDC_DEV_LIST, m_Dev);
	DDX_Text(pDX, IDC_EDIT_DEVNo, m_Dev_No);
	DDV_MinMaxInt(pDX, m_Dev_No, 0, 9);
	DDX_Text(pDX, IDC_SEND_ARP, m_sending);
	DDX_Text(pDX, IDC_COUNT, m_count);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CArpByWinpcapDlg, CDialog)
	//{{AFX_MSG_MAP(CArpByWinpcapDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCLOSE, OnClose)
	ON_BN_CLICKED(IDC_ANALYSIS, OnGetDev)
	ON_BN_CLICKED(IDC_GET_MAC, OnGetMac)
	ON_LBN_SELCHANGE(IDC_DEV_LIST, OnSelChangeDevList)
	ON_BN_CLICKED(IDC_STOP_CAP, OnStopCap)
	ON_BN_CLICKED(IDC_ANALYSIS, OnGetDev)
	ON_MESSAGE(WM_PACKET, OnPacket)		//进行消息映射
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CArpByWinpcapDlg message handlers

BOOL CArpByWinpcapDlg::OnInitDialog()
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

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_Dev_No = 0;
	UpdateData(FALSE);  
	GetDlgItem(IDC_GET_MAC)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP_CAP)->EnableWindow(FALSE);
	return TRUE;  
}

void CArpByWinpcapDlg::OnSysCommand(UINT nID, LPARAM lParam)
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


void CArpByWinpcapDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


HCURSOR CArpByWinpcapDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CArpByWinpcapDlg::OnClose() 
{
	SendMessage( WM_CLOSE );
}

void CArpByWinpcapDlg::OnGetDev() 
{
	GetDlgItem(IDC_ANALYSIS)->EnableWindow(FALSE);
	
	int i = 1;
	string strDev = "";
	m_Dev.AddString("经分析，本系统网络适配器列表如下：");
	pcap_if_t* alldevs = 0; 
	pcap_if_t* pDev = 0;
	pcap_addr_t* pAdr = 0;
	char errbuf[PCAP_ERRBUF_SIZE+1]; 
	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{ 								// 获得设备列表 
		MessageBox(errbuf, NULL, MB_OK | MB_ICONINFORMATION);					// 若没有设备则弹出警告
		exit(1);
	} 
	for(pDev = alldevs; pDev; pDev = pDev->next)
	{								// 遍历所有成员
		if (pDev->description)
		{
			strDev = char(i + 48);
			strDev += ". ";
			strDev += DelSpace(pDev->description);								//去掉网卡描述过多的空格
			pAdr = pDev->addresses;
			if (pAdr!=NULL)
			{		
				if (pAdr->addr->sa_family == AF_INET)
				{							//pAdr->addr是否IP地址类型
					strDev += " -> ";
					strDev += IpToStr(((struct sockaddr_in *)pAdr->addr)->sin_addr.s_addr);
					if(IpToStr(((struct sockaddr_in *)pAdr->addr)->sin_addr.s_addr)[0] != '0')
					{
						m_Dev_No = i;
						UpdateData(FALSE);										//传递变量值去界面
					}
					strDev += " & [";
					strDev += IpToStr(((struct sockaddr_in *)pAdr->netmask)->sin_addr.s_addr);
					strDev += "]";
					GetDlgItem(IDC_GET_MAC)->EnableWindow(TRUE);				//若网卡有IP地址，则使抓包按钮可用
				}
			}
			m_Dev.InsertString(i++, strDev.c_str());
		}
	}
	pcap_freealldevs(alldevs);													//不再需要列表, 释放
}

void CArpByWinpcapDlg::OnGetMac()
{
	GetDlgItem(IDC_GET_MAC)->EnableWindow(FALSE);								//使不能重复按下，以确保线程正确性
	UpdateData(TRUE);															//传递界面改变进变量
	int nDevNo = m_Dev_No;														//此时取得操作对象序号
	
	int i = 0;
	int nDev = 0;
	unsigned long chLocalIp = 0;														//存放本地ip地址
	pcap_if_t* alldevs = 0; 
	pcap_if_t* pDev = 0;
	pcap_addr_t* pAdr = 0;
	char errbuf[PCAP_ERRBUF_SIZE+1]; 
	pcap_findalldevs(&alldevs, errbuf);											// 获得设备列表 
	for(pDev = alldevs; pDev; pDev = pDev->next)
	{
		nDev++;							// 取得网卡总数
	}
	if ((nDevNo < 1) || (nDevNo > nDev))
	{
		MessageBox("您输入的序号越界!", "Get", MB_OK);
		pcap_freealldevs(alldevs);												// 释放设备列表
		GetDlgItem(IDC_GET_MAC)->EnableWindow(TRUE);							//使按钮可再按并返回
		return;
	}
	for(pDev = alldevs, i = 0; i < nDevNo - 1; pDev = pDev->next, i++);			// 通过指针转到上步所选择的设备
	pAdr = pDev->addresses;
	if(!pAdr)
	{																	//若没有绑定IP地址，则退出
		MessageBox("该适配器没有绑定IP地址!", "Get.Note", MB_OK);
		pcap_freealldevs(alldevs);
		GetDlgItem(IDC_GET_MAC)->EnableWindow(TRUE);							//使按钮可再按并返回
		return;
	}
	chLocalIp = ((struct sockaddr_in *)pAdr->addr)->sin_addr.s_addr;			//得到本地ip
	if(IpToStr(chLocalIp)[0] == '0')
	{
		MessageBox("请确定该适配器网线连接正常!", "Get.Note", MB_OK);
		pcap_freealldevs(alldevs);
		GetDlgItem(IDC_GET_MAC)->EnableWindow(TRUE);							//使按钮可再按并返回
		return;
	}
	if (!GetMacSignal)
	{
		bLocalMac = GetSelfMac(pDev->name, chLocalIp);
	}
	if (!GetMacSignal)
	{
		MessageBox("请确定该适配器工作正常!", "Get.Note", MB_OK);
		pcap_freealldevs(alldevs);
		GetDlgItem(IDC_GET_MAC)->EnableWindow(TRUE);							//使按钮可再按并返回
		return;
	}
	pDevGlobalHandle = pDev;
	strSelDeviceName = pDev->name;
	nThreadSignal = 1;
	GetDlgItem(IDC_STOP_CAP)->EnableWindow(TRUE);
	AfxBeginThread(WaitForArpRepeatPacket, this);
	Sleep(100);																	//让守候线程有时间完成初始化：）20061025
	AfxBeginThread(StartArpScan, this);
}

void CArpByWinpcapDlg::OnSelChangeDevList() 
{
	// TODO: Add your control notification handler code here
	m_Dev_No = m_Dev.GetCurSel();
	UpdateData(FALSE);
}


void CArpByWinpcapDlg::OnStopCap() 
{
	nThreadSignal = 0;
	GetDlgItem(IDC_GET_MAC)->EnableWindow(TRUE);
	GetDlgItem(IDC_STOP_CAP)->EnableWindow(FALSE);
	m_sending = "用户终止!";
	UpdateData(FALSE);
	return;
}

LRESULT CArpByWinpcapDlg::OnPacket(WPARAM wParam, LPARAM lParam)
{
	string* transPack = (string*)wParam;
	//处理捕获到的数据包
	if (lParam == 0){
		m_Mac_list.AddString(transPack->c_str());
		m_count = "发现  ";
		char buffer[5];
		itoa(m_Mac_list.GetCount(), buffer, 10);	//将数量转化为10进制字符串；
		m_count += buffer;
		m_count += "  台活动主机";
	}
	else if (lParam == 1)
	{
		m_sending = "正在发送ARP请求包!";
	}
	else if (lParam == 2)
	{
		if (nThreadSignal)
		{
			m_sending = "全部ARP请求包发送完毕!";	//判断是自行发送完毕还是用户终止的？
		};
	}
	UpdateData(FALSE);
	return 0;
}

