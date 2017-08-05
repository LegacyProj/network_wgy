
string strSelDeviceName = "";
unsigned char* bLocalMac;
pcap_if_t* pDevGlobalHandle = 0;
int nThreadSignal = 0;
int GetMacSignal = 0;
char* IpToStr(unsigned long ulIP);	// 将一个无符号长整型的机器IP地址转换为字符串类型的用户友好IP格式
char* MacToStr(unsigned char* chMAC);//将MAC地址的数组变为带“-”的大写的字符可读形式
char* DelSpace(char* in);//去掉字符串中多余的空格
unsigned char* BuildArpRequestPacket(unsigned char* source_mac, unsigned char* arp_sha, unsigned long chLocalIP, unsigned long arp_tpa, int PackSize);//封装ARP请求包
unsigned char* GetSelfMac(char* pDevName, unsigned long chLocalIP);
void SendArpRequest(pcap_if_t* pDev, unsigned char* bLocalMac);


UINT StartArpScan(LPVOID mainClass);//发送ARP请求数据包的线程函数

UINT WaitForArpRepeatPacket(LPVOID mainClass);//接收ARP响应的线程函数
