#include "stdafx.h"
#include "HttpProtocol.h"

// 格林威治时间的星期转换
char *week[] = {		
	"Sun,",  
	"Mon,",
	"Tue,",
	"Wed,",
	"Thu,",
	"Fri,",
	"Sat,",
};
 
// 格林威治时间的月份转换
char *month[] = 
{	
	"Jan",  
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};


UINT CHttpProtocol::ClientNum = 0;
CCriticalSection CHttpProtocol::m_critSect;		// 排斥区初始化
HANDLE	CHttpProtocol::None = NULL;


CHttpProtocol::CHttpProtocol(void)
{
	m_pListenThread = NULL;	
	m_hwndDlg = NULL;
}

CHttpProtocol::~CHttpProtocol(void)
{
}

bool CHttpProtocol::StartHttpSrv()
{
	WORD wVersionRequested = WINSOCK_VERSION;
	WSADATA wsaData;
	int nRet;
	// 启动Winsock
	nRet = WSAStartup(wVersionRequested, &wsaData);		// 加载成功返回0
	if (nRet)
	{   
		// 错误处理
		AfxMessageBox("Initialize WinSock Failed");
		return false;
	}
	// 检测版本
	if (wsaData.wVersion != wVersionRequested)
	{    
		// 错误处理   
		AfxMessageBox("Wrong WinSock Version");
		return false;
	}
	
	m_hExit = CreateEvent(NULL, TRUE, FALSE, NULL);	
	if (m_hExit == NULL)
	{
		return false;
	}

	//创建套接字
	m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_listenSocket == INVALID_SOCKET)
	{
		// 异常处理
		CString *pStr = new CString;
		*pStr = "Could not create listen socket";
		SendMessage(m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);
		return false;
	}

	SOCKADDR_IN sockAddr;
	LPSERVENT	lpServEnt;
	if (m_nPort != 0)
	{
		// 从主机字节顺序转为网络字节顺序赋给sin_port
		sockAddr.sin_port = htons(m_nPort);
	}
	else
	{	
		// 获取已知http服务的端口，该服务在tcp协议下注册
		lpServEnt = getservbyname("http", "tcp");
		if (lpServEnt != NULL)
		{
			sockAddr.sin_port = lpServEnt->s_port;
		}
		else
		{
			sockAddr.sin_port = htons(HTTPPORT);	// 默认端口HTTPPORT＝80
		}
	}

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = INADDR_ANY;  // 指定端口号上面的默认IP接口 

	// 初始化content-type和文件后缀对应关系的map
    CreateTypeMap();


	// 套接字绑定
	nRet = bind(m_listenSocket, (LPSOCKADDR)&sockAddr, sizeof(struct sockaddr));
	if (nRet == SOCKET_ERROR)
	{  
		// 绑定发生错误
		CString *pStr = new CString;
		*pStr = "bind() error";
		SendMessage(m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);
		closesocket(m_listenSocket);	// 断开链接
		return false;
	}

    // 套接字监听。为客户连接创建等待队列,队列最大长度SOMAXCONN在windows sockets头文件中定义
	nRet = listen(m_listenSocket, SOMAXCONN);
	if (nRet == SOCKET_ERROR)
	{   
		// 异常处理
		CString *pStr = new CString;
		*pStr = "listen() error";
		SendMessage(m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);
		closesocket(m_listenSocket);	// 断开链接
		return false;
	}
	// 创建listening进程，接受客户机连接要求
	m_pListenThread = AfxBeginThread(ListenThread, this);
	
	if (!m_pListenThread)
	{
		// 线程创建失败
		CString *pStr = new CString;
		*pStr = "Could not create listening thread" ;
		SendMessage(m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);
		closesocket(m_listenSocket);	// 断开链接
		return false;
	}

	CString strIP, strTemp;
	char hostname[255];
	PHOSTENT hostinfo;
	// 获取计算机名
	if(gethostname(hostname, sizeof(hostname))==0)	// 得到主机名
	{
		// 由主机名得到主机的其他信息
		hostinfo = gethostbyname(hostname);
		if(hostinfo != NULL)
		{
			strIP = inet_ntoa(*(struct in_addr*)*(hostinfo->h_addr_list));
		}
	}
	
	// 显示web服务器正在启动
	CString *pStr = new CString;
	*pStr = "****** My WebServer is Starting now! *******";
	SendMessage(m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);

	// 显示web服务器的信息，包括主机名，IP以及端口号
	CString *pStr1 = new CString;
	pStr1->Format("%s", hostname); 
	*pStr1 = *pStr1 + "[" + strIP + "]" + "   Port ";
	strTemp.Format("%d", htons(sockAddr.sin_port));
	*pStr1 = *pStr1 + strTemp;
	SendMessage(m_hwndDlg, LOG_MSG, (UINT)pStr1, NULL);
	
	return true;

}


void CHttpProtocol::CreateTypeMap()
{
	// 初始化map
    m_typeMap[".doc"]	= "application/msword";
	m_typeMap[".bin"]	= "application/octet-stream";
	m_typeMap[".dll"]	= "application/octet-stream";
	m_typeMap[".exe"]	= "application/octet-stream";
	m_typeMap[".pdf"]	= "application/pdf";
	m_typeMap[".ai"]	= "application/postscript";
	m_typeMap[".eps"]	= "application/postscript";
	m_typeMap[".ps"]	= "application/postscript";
	m_typeMap[".rtf"]	= "application/rtf";
	m_typeMap[".fdf"]	= "application/vnd.fdf";
	m_typeMap[".arj"]	= "application/x-arj";
	m_typeMap[".gz"]	= "application/x-gzip";
	m_typeMap[".class"]	= "application/x-java-class";
	m_typeMap[".js"]	= "application/x-javascript";
	m_typeMap[".lzh"]	= "application/x-lzh";
	m_typeMap[".lnk"]	= "application/x-ms-shortcut";
	m_typeMap[".tar"]	= "application/x-tar";
	m_typeMap[".hlp"]	= "application/x-winhelp";
	m_typeMap[".cert"]	= "application/x-x509-ca-cert";
	m_typeMap[".zip"]	= "application/zip";
	m_typeMap[".cab"]	= "application/x-compressed";
	m_typeMap[".arj"]	= "application/x-compressed";
	m_typeMap[".aif"]	= "audio/aiff";
	m_typeMap[".aifc"]	= "audio/aiff";
	m_typeMap[".aiff"]	= "audio/aiff";
	m_typeMap[".au"]	= "audio/basic";
	m_typeMap[".snd"]	= "audio/basic";
	m_typeMap[".mid"]	= "audio/midi";
	m_typeMap[".rmi"]	= "audio/midi";
	m_typeMap[".mp3"]	= "audio/mpeg";
	m_typeMap[".vox"]	= "audio/voxware";
	m_typeMap[".wav"]	= "audio/wav";
	m_typeMap[".ra"]	= "audio/x-pn-realaudio";
	m_typeMap[".ram"]	= "audio/x-pn-realaudio";
	m_typeMap[".bmp"]	= "image/bmp";
	m_typeMap[".gif"]	= "image/gif";
	m_typeMap[".jpeg"]	= "image/jpeg";
	m_typeMap[".jpg"]	= "image/jpeg";
	m_typeMap[".tif"]	= "image/tiff";
	m_typeMap[".tiff"]	= "image/tiff";
	m_typeMap[".xbm"]	= "image/xbm";
	m_typeMap[".wrl"]	= "model/vrml";
	m_typeMap[".htm"]	= "text/html";
	m_typeMap[".html"]	= "text/html";
	m_typeMap[".c"]		= "text/plain";
	m_typeMap[".cpp"]	= "text/plain";
	m_typeMap[".def"]	= "text/plain";
	m_typeMap[".h"]		= "text/plain";
	m_typeMap[".txt"]	= "text/plain";
	m_typeMap[".rtx"]	= "text/richtext";
	m_typeMap[".rtf"]	= "text/richtext";
	m_typeMap[".java"]	= "text/x-java-source";
	m_typeMap[".css"]	= "text/css";
	m_typeMap[".mpeg"]	= "video/mpeg";
	m_typeMap[".mpg"]	= "video/mpeg";
	m_typeMap[".mpe"]	= "video/mpeg";
	m_typeMap[".avi"]	= "video/msvideo";
	m_typeMap[".mov"]	= "video/quicktime";
	m_typeMap[".qt"]	= "video/quicktime";
	m_typeMap[".shtml"]	= "wwwserver/html-ssi";
	m_typeMap[".asa"]	= "wwwserver/isapi";
	m_typeMap[".asp"]	= "wwwserver/isapi";
	m_typeMap[".cfm"]	= "wwwserver/isapi";
	m_typeMap[".dbm"]	= "wwwserver/isapi";
	m_typeMap[".isa"]	= "wwwserver/isapi";
	m_typeMap[".plx"]	= "wwwserver/isapi";
	m_typeMap[".url"]	= "wwwserver/isapi";
	m_typeMap[".cgi"]	= "wwwserver/isapi";
	m_typeMap[".php"]	= "wwwserver/isapi";
	m_typeMap[".wcgi"]	= "wwwserver/isapi";

}


UINT CHttpProtocol::ListenThread(LPVOID param)
{
	CHttpProtocol *pHttpProtocol = (CHttpProtocol *)param;

	SOCKET		socketClient;
	CWinThread*	pClientThread;
	SOCKADDR_IN	SockAddr;
	PREQUEST	pReq;
	int			nLen;
	DWORD		dwRet;

	// 初始化ClientNum，创建"no client"事件对象
	HANDLE		hNoClients;
	hNoClients = pHttpProtocol->InitClientCount();

	while(1)	// 循环等待,如有客户连接请求,则接受客户机连接要求
	{	
		nLen = sizeof(SOCKADDR_IN);		
		// 套接字等待链接,返回对应已接受的客户机连接的套接字
		socketClient = accept(pHttpProtocol->m_listenSocket, (LPSOCKADDR)&SockAddr, &nLen);
		if (socketClient == INVALID_SOCKET)
		{   
			break;
		}		
		// 将客户端网络地址转换为用点分割的IP地址
		CString *pstr = new CString;
		pstr->Format("%s Connecting on socket:%d", inet_ntoa(SockAddr.sin_addr), socketClient);
		SendMessage(pHttpProtocol->m_hwndDlg, LOG_MSG, (UINT)pstr, NULL);

        pReq = new REQUEST;
		if (pReq == NULL)
		{   
			// 处理错误
			CString *pStr = new CString;
			*pStr = "No memory for request";
			SendMessage(pHttpProtocol->m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);
			continue;
		}
		pReq->hExit  = pHttpProtocol->m_hExit;
		pReq->Socket = socketClient;
		pReq->hFile = INVALID_HANDLE_VALUE;
		pReq->dwRecv = 0;
		pReq->dwSend = 0;
		pReq->pHttpProtocol = pHttpProtocol;

	    // 创建client进程，处理request
		pClientThread = AfxBeginThread(ClientThread, pReq);
		if (!pClientThread)
		{  
			// 线程创建失败,错误处理
			CString *pStr = new CString;
			*pStr = "Couldn't start client thread";
			SendMessage(pHttpProtocol->m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);

			delete pReq;
		}
	} //while
	// 等待线程结束
	WaitForSingleObject((HANDLE)pHttpProtocol->m_hExit, INFINITE);
    // 等待所有client进程结束
	dwRet = WaitForSingleObject(hNoClients, 5000);
	if (dwRet == WAIT_TIMEOUT) 
	{  
		// 超时返回，并且同步对象未退出
		CString *pStr = new CString;
		*pStr = "One or more client threads did not exit";
		SendMessage(pHttpProtocol->m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);
	}
	pHttpProtocol->DeleteClientCount();

	return 0;
}

UINT CHttpProtocol::ClientThread(LPVOID param)
{
	int nRet;
	BYTE buf[1024];
	PREQUEST pReq = (PREQUEST)param;
	CHttpProtocol *pHttpProtocol = (CHttpProtocol *)pReq->pHttpProtocol;

	pHttpProtocol->CountUp();			// 记数
	
	// 接收request data
	 if (!pHttpProtocol->RecvRequest(pReq, buf, sizeof(buf)))
	{  
		pHttpProtocol->Disconnect(pReq);
		delete pReq;
		pHttpProtocol->CountDown();      
		return 0;  
	}
	
	// 分析request信息
	nRet = pHttpProtocol->Analyze(pReq, buf);
	if (nRet)
	{	
		// 处理错误
		CString *pStr = new CString;
		*pStr = "Error occurs when analyzing client request";
		SendMessage(pHttpProtocol->m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);

		pHttpProtocol->Disconnect(pReq);
		delete pReq;
		pHttpProtocol->CountDown();     
		return 0;
	}

	// 生成并返回头部
	pHttpProtocol->SendHeader(pReq);

	// 向client传送数据
	if(pReq->nMethod == METHOD_GET)
	{
		pHttpProtocol->SendFile(pReq);
	}

	pHttpProtocol->Disconnect(pReq);
	delete pReq;
	pHttpProtocol->CountDown();	// client数量减1

	return 0;
}

bool CHttpProtocol::RecvRequest(PREQUEST pReq, LPBYTE pBuf, DWORD dwBufSize)
{
	WSABUF			wsabuf;		// 发送/接收缓冲区结构
	WSAOVERLAPPED	over;		// 指向调用重叠操作时指定的WSAOVERLAPPED结构
	DWORD			dwRecv;
	DWORD			dwFlags;
	DWORD			dwRet;
	HANDLE			hEvents[2];
	bool			fPending;
	int				nRet;
	memset(pBuf, 0, dwBufSize);	// 初始化缓冲区
    wsabuf.buf  = (char *)pBuf;
	wsabuf.len  = dwBufSize;	// 缓冲区的长度
	over.hEvent = WSACreateEvent();	// 创建一个新的事件对象
	dwFlags = 0;
	fPending = FALSE;  
	// 接收数据
	nRet = WSARecv(pReq->Socket, &wsabuf, 1, &dwRecv, &dwFlags, &over, NULL);
    if (nRet != 0)  
	{
    	// 错误代码WSA_IO_PENDING表示重叠操作成功启动
		if (WSAGetLastError() != WSA_IO_PENDING)
		{   
			// 重叠操作未能成功
			CloseHandle(over.hEvent);
			return false;
		}
		else	
		{
			fPending = true;
		}
	}
	if (fPending)
	{	
		hEvents[0]  = over.hEvent;
		hEvents[1]  = pReq->hExit;
		dwRet = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		if (dwRet != 0)
		{
			CloseHandle(over.hEvent);                     
			return false;
		}
        // 重叠操作未完成
		if (!WSAGetOverlappedResult(pReq->Socket, &over, &dwRecv, FALSE, &dwFlags))
	{
			CloseHandle(over.hEvent);                    
		    return false;
		}
	}
	pReq->dwRecv += dwRecv;	// 统计接收数量
	CloseHandle(over.hEvent);                           
	return true;
}
	
// 分析request信息
int  CHttpProtocol::Analyze(PREQUEST pReq, LPBYTE pBuf)
{
	// 分析接收到的信息
	char szSeps[] = " \n";
	char *cpToken;
	// 防止非法请求
	if (strstr((const char *)pBuf, "..") != NULL)
	{
		strcpy(pReq->StatuCodeReason, HTTP_STATUS_BADREQUEST);
		return 1;
	}

	// 判断ruquest的mothed	
	cpToken = strtok((char *)pBuf, szSeps);	// 缓存中字符串分解为一组标记串。	
	if (!_stricmp(cpToken, "GET"))			// GET命令
	{
		pReq->nMethod = METHOD_GET;
	}
	else if (!_stricmp(cpToken, "HEAD"))	// HEAD命令
	{
		pReq->nMethod = METHOD_HEAD;  
	}
	else  
	{
        strcpy(pReq->StatuCodeReason, HTTP_STATUS_NOTIMPLEMENTED);
		return 1;
	}
    
	// 获取Request-URI
	cpToken = strtok(NULL, szSeps);
	if (cpToken == NULL)   
	{
		strcpy(pReq->StatuCodeReason, HTTP_STATUS_BADREQUEST);
		return 1;
	}

	strcpy(pReq->szFileName, m_strRootDir);
	if (strlen(cpToken) > 1)
	{
		strcat(pReq->szFileName, cpToken);	// 把该文件名添加到结尾处形成路径
	}
	else
	{
		strcat(pReq->szFileName, "/index.html");
	}
	return 0;
}

// 发送头部
void CHttpProtocol::SendHeader(PREQUEST pReq)
{
	
    int n = FileExist(pReq);
	if(!n)		// 文件不存在，则返回
	{
		return;
	}

	char Header[2048] = " ";
	char curTime[50] = " ";
	GetCurentTime((char*)curTime);
	// 取得文件长度
    DWORD length;
	length = GetFileSize(pReq->hFile, NULL);	
	// 取得文件的last-modified时间
	char last_modified[60] = " ";
	GetLastModified(pReq->hFile, (char*)last_modified);	
	// 取得文件的类型
	char ContenType[50] = " ";
 	GetContenType(pReq, (char*)ContenType);

	sprintf((char*)Header, "HTTP/1.0 %s\r\nDate: %s\r\nServer: %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nLast-Modified: %s\r\n\r\n",
			                    HTTP_STATUS_OK, 
								curTime,				// Date
								"My Http Server",       // Server
								ContenType,				// Content-Type
								length,					// Content-length
								last_modified);			// Last-Modified

    // 发送头部
	send(pReq->Socket, Header, strlen(Header), 0);	
}


int CHttpProtocol::FileExist(PREQUEST pReq)
{
	pReq->hFile = CreateFile(pReq->szFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// 如果文件不存在，则返回出错信息
	if (pReq->hFile == INVALID_HANDLE_VALUE)
	{
		strcpy(pReq->StatuCodeReason, HTTP_STATUS_NOTFOUND);
		return 0;
	}
	else 
	{
		return 1;
	}
}

// 发送文件
void CHttpProtocol::SendFile(PREQUEST pReq)
{
	int n = FileExist(pReq);
	if(!n)			// 文件不存在，则返回
	{		
		return;
	}

	CString *pStr = new CString;
	*pStr = *pStr + &pReq->szFileName[strlen(m_strRootDir)];
	SendMessage(m_hwndDlg, LOG_MSG, UINT(pStr), NULL);	
   

	static BYTE buf[2048];
    DWORD  dwRead;
    BOOL   fRet;
	int flag = 1;
    // 读写数据直到完成
    while(1)
	{	
		// 从file中读入到buffer中        
		fRet = ReadFile(pReq->hFile, buf, sizeof(buf), &dwRead, NULL);
		if (!fRet)
		{	
			
	    	static char szMsg[512];
		    wsprintf(szMsg, "%s", HTTP_STATUS_SERVERERROR);
        	// 向客户端发送出错信息
        	send(pReq->Socket, szMsg, strlen(szMsg), 0);	
	    	break;
		}
		// 完成
		if (dwRead == 0)
		{	
			break;
		}
		// 将buffer内容传送给client
		if (!SendBuffer(pReq, buf, dwRead))	
		{
			break;
		}
		pReq->dwSend += dwRead;
	}
    // 关闭文件
	if (CloseHandle(pReq->hFile))
	{
		pReq->hFile = INVALID_HANDLE_VALUE;
	}
	else
	{
		CString *pStr = new CString;
		*pStr = "Error occurs when closing file";
		SendMessage(m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);
	}
}

bool CHttpProtocol::SendBuffer(PREQUEST pReq, LPBYTE pBuf, DWORD dwBufSize)
{   
	// 发送缓存中的内容
	WSABUF			wsabuf;
	WSAOVERLAPPED	over;
	DWORD			dwRecv;
	DWORD			dwFlags;
	DWORD			dwRet;
	HANDLE			hEvents[2];
	BOOL			fPending;
	int				nRet;
	wsabuf.buf  = (char *)pBuf;
	wsabuf.len  = dwBufSize;
	over.hEvent = WSACreateEvent();
	fPending = false;
	
	// 发送数据 
	nRet = WSASend(pReq->Socket, &wsabuf, 1, &dwRecv, 0, &over, NULL);
	if (nRet != 0)
	{
		// 错误出理
		if (WSAGetLastError() == WSA_IO_PENDING)
		{
			fPending = true;
		}
		else
		{	
			CString *pStr = new CString;
			pStr->Format("WSASend() error: %d", WSAGetLastError() );
			SendMessage(m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);

			CloseHandle(over.hEvent);
			return false;
		}
	}
	if (fPending)	// i/o未完成
	{	
		hEvents[0]  = over.hEvent;
		hEvents[1]  = pReq->hExit;
		dwRet = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		if (dwRet != 0)
		{
			CloseHandle(over.hEvent);
			return false;
		}
		// 重叠操作未完成
		if (!WSAGetOverlappedResult(pReq->Socket, &over, &dwRecv, FALSE, &dwFlags))
		{
			// 错误处理
			CString *pStr = new CString;
			pStr->Format("WSAGetOverlappedResult() error: %d", WSAGetLastError() );
			SendMessage(m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);
			CloseHandle(over.hEvent);
			return false;
		}
	}
	CloseHandle(over.hEvent);
	return true;
}

void CHttpProtocol::Disconnect(PREQUEST pReq)
{
	// 关闭套接字：释放所占有的资源
	int	nRet;
	CString *pStr = new CString;
	pStr->Format("Closing socket: %d", pReq->Socket);
	SendMessage(m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);
	
	nRet = closesocket(pReq->Socket);
	if (nRet == SOCKET_ERROR)
	{
		// 处理错误
		CString *pStr1 = new CString;
		pStr1->Format("closesocket() error: %d", WSAGetLastError() );
		SendMessage(m_hwndDlg, LOG_MSG, (UINT)pStr1, NULL);
	}

	HTTPSTATS	stats;
	stats.dwRecv = pReq->dwRecv;
	stats.dwSend = pReq->dwSend;
	SendMessage(m_hwndDlg, DATA_MSG, (UINT)&stats, NULL);
}

HANDLE CHttpProtocol::InitClientCount()
{
	ClientNum = 0;
	// 创建"no client"事件对象
	None = CreateEvent(NULL, TRUE, TRUE, NULL);	
	return None;
}

void CHttpProtocol::CountUp()
{
	// 进入排斥区
	m_critSect.Lock();
	ClientNum++;
	// 离开排斥区
	m_critSect.Unlock();
	// 重置为无信号事件对象
	ResetEvent(None);
}

void CHttpProtocol::CountDown()
{	
	// 进入排斥区
	m_critSect.Lock();
	if(ClientNum > 0)
	{
		ClientNum--;
	} 
	// 离开排斥区
	m_critSect.Unlock();
	if(ClientNum < 1)
	{
		// 重置为有信号事件对象
		SetEvent(None);
	}
}

void CHttpProtocol::DeleteClientCount()
{
	CloseHandle(None);
}

// 活动本地时间
void CHttpProtocol::GetCurentTime(LPSTR lpszString)
{  
	// 活动本地时间
	SYSTEMTIME st;
	GetLocalTime(&st);
	// 时间格式化
    wsprintf(lpszString, "%s %02d %s %d %02d:%02d:%02d GMT",week[st.wDayOfWeek], st.wDay,month[st.wMonth-1],
     st.wYear, st.wHour, st.wMinute, st.wSecond);
}

bool CHttpProtocol::GetLastModified(HANDLE hFile, LPSTR lpszString)
{
	// 获得文件的last-modified 时间
	FILETIME ftCreate, ftAccess, ftWrite;
    SYSTEMTIME stCreate;
	FILETIME ftime;	
	// 获得文件的last-modified的UTC时间
    if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))   
		return false;
	FileTimeToLocalFileTime(&ftWrite,&ftime);
	// UTC时间转化成本地时间
    FileTimeToSystemTime(&ftime, &stCreate);	
	// 时间格式化
	wsprintf(lpszString, "%s %02d %s %d %02d:%02d:%02d GMT", week[stCreate.wDayOfWeek],
		stCreate.wDay, month[stCreate.wMonth-1], stCreate.wYear, stCreate.wHour,
		stCreate.wMinute, stCreate.wSecond);
}

bool CHttpProtocol::GetContenType(PREQUEST pReq, LPSTR type)
{   
	// 取得文件的类型
    CString cpToken;
    cpToken = strstr(pReq->szFileName, ".");
    strcpy(pReq->postfix, cpToken);
	// 遍历搜索该文件类型对应的content-type
	map<CString, char *>::iterator it = m_typeMap.find(pReq->postfix);
	if(it != m_typeMap.end()) 	
	{
		wsprintf(type,"%s",(*it).second);
	}
	return TRUE;
}

void CHttpProtocol::StopHttpSrv()
{

	int nRet;
	SetEvent(m_hExit);
	nRet = closesocket(m_listenSocket);
	nRet = WaitForSingleObject((HANDLE)m_pListenThread, 10000);
	if (nRet == WAIT_TIMEOUT)
	{
		CString *pStr = new CString;
		*pStr = "TIMEOUT waiting for ListenThread";
		SendMessage(m_hwndDlg, LOG_MSG, (UINT)pStr, NULL);
	}
	CloseHandle(m_hExit);

	CString *pStr1 = new CString;
	*pStr1 = "Server Stopped";
	SendMessage(m_hwndDlg, LOG_MSG, (UINT)pStr1, NULL);
}