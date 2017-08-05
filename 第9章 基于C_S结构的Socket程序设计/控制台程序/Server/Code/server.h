

#define MAX_CLIENT  10						//同时服务的client数目上限
#define MAX_BUF_SIZE 65535					//缓存区的大小

const u_short UDPSrvPort	= 2345;			//Server的UDP端口
const char START_CMD[]		= "START";
const char GETCURTIME_CMD[]	= "GET CUR TIME";

//传递给TCP线程的结构化参数
struct TcpThreadParam
{
	SOCKET socket;
	sockaddr_in addr;
};

DWORD WINAPI TcpServeThread(LPVOID lpParam);	//TCP线程的线程函数
DWORD WINAPI UdpServer(LPVOID lpParam);	//UDP服务器线程


