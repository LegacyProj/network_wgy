#include<iostream.h>
#include<fstream.h>
#include<winsock.h>			// 用于使用网络顺序显示：htons
#pragma comment(lib, "WS2_32.LIB")



/**************************************************************************
 * 计算给定数据的校验和
 *
 *		输入参数：
 *			pBuffer		指向需要校验的数据缓冲区
 *			nSize		需要校验的数据的大小，以字节为单位
 *
 *		返回值：
 *			16位的校验结果
 *
 **************************************************************************/
unsigned short checksum_calculating(unsigned short *pBuffer, int nSize)
{
	unsigned long dwCksum = 0;		// 32位累加和

	// 以两字节为单位反复累加
	while(nSize > 1)
	{
		dwCksum += *pBuffer++;
		nSize -= sizeof(unsigned short);
	}
	// 如果总字节数为奇数则加上最后一个字节
	if (nSize)
	{
		dwCksum += *(unsigned char*) pBuffer;
	}
	// 将32位累加和的高16位与低16位第一次相加
	dwCksum = (dwCksum >> 16) + (dwCksum & 0xffff);
	// 将上一步可能产生的高16位进位再次与低16位累加
	dwCksum += (dwCksum >> 16);

	// 返回16位校验和
	return (unsigned short) (~dwCksum);
}


void main(int argc, char * argv[])
{
	// 判断输入的命令行格式是否正确
	if (argc != 3)
	{
		cout << "请按以下格式输入命令行: Checksum inputfile outputfile" <<endl;
		return;
	}
	
	// 创建输入文件流
	ifstream fInfile;
	// 创建输出文件流
	fstream fOutfile;

	// 以2进制方式打开指定的输入文件
	fInfile.open(argv[1], ios::in|ios::binary);

	// 把文件指针移到文件末尾
	fInfile.seekg(0, ios::end);

	// 取得输入文件的长度
	unsigned short wLen = (unsigned short)fInfile.tellg();

	// 文件指针位置初始化
	fInfile.seekg(0, ios::beg);
	
	// 定义数据报缓冲区，缓冲区大小为4+wLen ，其中4为数据报类型字段、长度字段
	// 以及校验和字段的长度和，wLen为数据字段长度，即输入文件长度（以字节为单位）
	unsigned char * pBuf = new unsigned char[4 + wLen];

	pBuf[0] = unsigned char(0xab);		// 给数据报类型字段赋值
	pBuf[1] = unsigned char(wLen);		// 给数据报长度字段赋值
	*(unsigned short *)(pBuf + 2) = 0;	// 计算校验和之前，校验和字段先置为0
	
	fInfile.read(pBuf+4, wLen);			// 根据输入文件填充数据报的数据字段

	// 计算校验和并把结果填入到数据报的校验和字段
	*(unsigned short *)(pBuf+2) = checksum_calculating((unsigned short *)pBuf,4+wLen);
		
	// 屏幕输出校验和计算结果
	cout.width(4);					
	cout << "校验和为：0x" << hex << htons( *(unsigned short *)(pBuf+2) )
		 << "	(以网络顺序显示)"<< endl;
	
	// 以2进制方式打开输出文件
	fOutfile.open(argv[2],ios::in|ios::out|ios::binary|ios::trunc);

	// 将pBuf中的数据报写入输出文件
	fOutfile.write((char *)pBuf, wLen+4);

	cout<< "数据报已成功保存在" << argv[2] << "文件中!" << endl;

	delete pBuf;		// 释放数据报缓冲区

	fInfile.close();	// 关闭输入文件流
	fOutfile.close();	// 关闭输出文件流
}

