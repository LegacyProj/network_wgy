#ifndef _HTTPPROTOCOL_H
#define _HTTPPROTOCOL_H


#pragma once

#include <winsock2.h>
#include <afxmt.h>

#define HTTPPORT 80
#define METHOD_GET 0
#define METHOD_HEAD 1


#define HTTP_STATUS_OK				"200 OK"
#define HTTP_STATUS_CREATED			"201 Created"
#define HTTP_STATUS_ACCEPTED		"202 Accepted"
#define HTTP_STATUS_NOCONTENT		"204 No Content"
#define HTTP_STATUS_MOVEDPERM		"301 Moved Permanently"
#define HTTP_STATUS_MOVEDTEMP		"302 Moved Temporarily"
#define HTTP_STATUS_NOTMODIFIED		"304 Not Modified"
#define HTTP_STATUS_BADREQUEST		"400 Bad Request"
#define HTTP_STATUS_UNAUTHORIZED	"401 Unauthorized"
#define HTTP_STATUS_FORBIDDEN		"403 Forbidden"
#define HTTP_STATUS_NOTFOUND		"404 File can not fonund!"
#define HTTP_STATUS_SERVERERROR		"500 Internal Server Error"
#define HTTP_STATUS_NOTIMPLEMENTED	"501 Not Implemented"
#define HTTP_STATUS_BADGATEWAY		"502 Bad Gateway"
#define HTTP_STATUS_UNAVAILABLE		"503 Service Unavailable"


// 连接的Client的信息
typedef struct REQUEST
{
	HANDLE		hExit;
	SOCKET		Socket;                // 请求的socket
	int			nMethod;               // 请求的使用方法：GET或HEAD
	DWORD		dwRecv;                // 收到的字节数
	DWORD		dwSend;                // 发送的字节数
	HANDLE		hFile;                 // 请求连接的文件
	char		szFileName[_MAX_PATH]; // 文件的相对路径
	char		postfix[10];           // 存储扩展名
	char        StatuCodeReason[100];  // 头部的status cod以及reason-phrase
	bool        permitted;             // 用户权限判断
	char *      authority;             // 用户提供的认证信息
	char        key[1024];             // 正确认证信息

	void* pHttpProtocol;			   // 指向类CHttpProtocol的指针
}REQUEST, *PREQUEST;

typedef struct HTTPSTATS
{
	DWORD	dwRecv;               // 收到字节数
	DWORD	dwSend;               // 发送字节数
}HTTPSTATS, *PHTTPSTATS;


#include <map>
#include <string>
using namespace std;


class CHttpProtocol
{
public:

	HWND m_hwndDlg;

	SOCKET m_listenSocket;

	map<CString, char *> m_typeMap;		// 保存content-type和文件后缀的对应关系map

	CWinThread* m_pListenThread;

	HANDLE m_hExit;

	static HANDLE	None;				// 标志是否有Client连接到Server
	static UINT ClientNum;				// 连接的Client数量
	static CCriticalSection m_critSect; // 互斥变量

	CString m_strRootDir;				// web的根目录
	UINT	m_nPort;					// http server的端口号


public:
	CHttpProtocol(void);

	void DeleteClientCount();
	void CountDown();
	void CountUp();
	HANDLE InitClientCount();
	
	void StopHttpSrv();
	bool StartHttpSrv();

	static UINT ListenThread(LPVOID param);
	static UINT ClientThread(LPVOID param);

	bool RecvRequest(PREQUEST pReq, LPBYTE pBuf, DWORD dwBufSize);
	int Analyze(PREQUEST pReq, LPBYTE pBuf);
	void Disconnect(PREQUEST pReq);
	void CreateTypeMap();
	void SendHeader(PREQUEST pReq);
	int FileExist(PREQUEST pReq);
	
	void GetCurentTime(LPSTR lpszString);
	bool GetLastModified(HANDLE hFile, LPSTR lpszString);
	bool GetContenType(PREQUEST pReq, LPSTR type);
	void SendFile(PREQUEST pReq);
	bool SendBuffer(PREQUEST pReq, LPBYTE pBuf, DWORD dwBufSize);


public:
	~CHttpProtocol(void);
};


#endif _HTTPPROTOCOL_H