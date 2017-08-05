#include<fstream.h>			// 用于文件操作
#include<stdlib.h>			// 用于程序流程控制


////////////////////////////////////////////////////////////////////////////////
// CRC校验，在上一轮校验的基础上继续作8位CRC校验
// 
//	输入参数：
//		chCurrByte	低8位数据有效，记录了上一次CRC校验的余数
//		chNextByte	低8位数据有效，记录了本次要继续校验的一个字节	
//
//	传出参数：
//		chCurrByte	低8位数据有效，记录了本次CRC校验的余数
////////////////////////////////////////////////////////////////////////////////

void checkCRC(int &chCurrByte, int chNextByte)
{
	// CRC循环：每次调用进行8次循环，处理一个字节的数据。
	for (int nMask = 0x80; nMask > 0; nMask >>= 1)
	{
		if ((chCurrByte & 0x80) != 0)		// 首位为1：移位，并进行异或运算		
		{	
			chCurrByte <<= 1;				// 移一位
			if ( (chNextByte & nMask) != 0)	// 补一位
			{
				chCurrByte |= 1;
			}
			chCurrByte ^= 7;				// 首位已经移出，仅对低8位进行异或运算，7的二进制为0000,0111
		}
		else								// 首位为0，只移位，不进行异或运算
		{		
			chCurrByte <<= 1;				// 移一位
			if ( (chNextByte & nMask) != 0)	// 补一位
			{
				chCurrByte |= 1;
			}
		}
	}
}


void main(int argc, char* argv[])		
{
	// 检测命令行参数的正确性
	if (argc != 2)
	{
		cout << "请以帧封装包文件为参数重新执行程序" << endl;
		exit(0);
	}

	// 检测输入文件是否存在，并可以按所需的权限和方式打开
	ifstream file(argv[1], ios::in|ios::binary|ios::nocreate);
	if (!file.is_open())
	{
		cout << "无法打开帧封装包文件，请检查文件是否存在并且未损坏" << endl;
		exit(0);
	}
	

	// 变量声明及初始化
	int nSN = 1;						// 帧序号
	int nCheck = 0;						// 校验码
	int nCurrDataOffset = 22;			// 帧头偏移量
	int nCurrDataLength = 0;			// 数据字段长度
	bool bParseCont = true;				// 是否继续对输入文件进行解析
	int nFileEnd = 0;					// 输入文件的长度
	
	// 计算输入文件的长度
	file.seekg(0, ios::end);			// 把文件指针移到文件的末尾
	nFileEnd = file.tellg();			// 取得输入文件的长度
	file.seekg(0, ios::beg);			// 文件指针位置初始化

	cout.fill('0');						// 显示初始化
	cout.setf(ios::uppercase);			// 以大写字母输出

	// 定位到输入文件中的第一个有效帧
	// 从文件头开始，找到第一个连续的“AA-AA-AA-AA-AA-AA-AA-AB”
	while ( true )
	{		
		for (int j = 0; j < 7; j++)				// 找7个连续的0xaa
		{			
			if (file.tellg() >= nFileEnd)		// 安全性检测
			{
				cout<<"没有找到合法的帧"<<endl;
				file.close();
				exit(0);
			}
			// 看当前字符是不是0xaa，如果不是，则重新寻找7个连续的0xaa
			if (file.get() != 0xaa)				
			{
				j = -1;
			}
		}
		
		if (file.tellg() >= nFileEnd)			// 安全性检测
		{
			cout<<"没有找到合法的帧"<<endl;
			file.close();
			exit(0);
		}
		
		if (file.get() == 0xab)					// 判断7个连续的0xaa之后是否为0xab
		{
			break;
		}
	}

	// 将数据字段偏移量定位在上述二进制串之后14字节处，并准备进入解析阶段
	nCurrDataOffset = file.tellg() + 14;
	file.seekg(-8,ios::cur);


	// 主控循环
	while ( bParseCont ) // 当仍然可以继续解析输入文件时,继续解析
	{

		// 检测剩余文件是否可能包含完整帧头
		if (file.tellg() + 14 > nFileEnd)
		{
			cout<<endl<<"没有找到完整帧头，解析终止"<<endl;
			file.close();
			exit(0);
		}

		int c;						// 读入字节
		int i = 0;					// 循环控制变量					
		int EtherType = 0;			// 由帧中读出的类型字段
		bool bAccept = true;		// 是否接受该帧


		// 输出帧的序号
		cout << endl << "序号：\t\t" << nSN;
		
		// 输出前导码，只输出，不校验
		cout << endl << "前导码：\t";			
		for (i = 0; i < 7; i++)					// 输出格式为：AA AA AA AA AA AA AA
		{
			cout.width(2);
			cout << hex << file.get() << dec << " ";
		}
		// 输出帧前定界符，只输出，不校验
		cout << endl << "帧前定界符：\t";		
		cout.width(2);							// 输出格式为：AB
		cout << hex << file.get();

		// 输出目的地址，并校验
		cout << endl << "目的地址：\t";	
		for (i = 0; i < 6; i++)					// 输出格式为：xx-xx-xx-xx-xx-xx
		{
			c = file.get();
			cout.width(2);
			cout<< hex << c << dec << (i==5 ? "" : "-");
			if (i == 0)							// 第一个字节，作为“余数”等待下一个bit
			{
				nCheck = c;
			}
			else								// 开始校验
			{
				checkCRC(nCheck, c);
			}
		}
		
		// 输出源地址，并校验
		cout << endl << "源地址：\t";
		for (i = 0; i < 6; i++)					// 输出格式为：xx-xx-xx-xx-xx-xx
		{
			c = file.get();
			cout.width(2);
			cout<< hex << c << dec << (i==5 ? "" : "-");
			checkCRC(nCheck, c);				// 继续校验
		}

		// 输出类型字段，并校验
		cout<<endl<<"类型字段：\t";
		cout.width(2);							
		// 输出类型字段的高8位
		c = file.get();
		cout<< hex << c << dec << " ";
		checkCRC(nCheck, c);					// CRC校验
		EtherType = c;
		// 输出类型字段的低8位
		c = file.get();						
		cout.width(2);
		cout<< hex << c;
		checkCRC(nCheck,c);						// CRC校验
		EtherType <<= 8;						// 转换成主机格式
		EtherType |= c;

		// 定位下一个帧，以确定当前帧的结束位置
		while ( bParseCont )
		{

			for (int i = 0; i < 7; i++)					//找下一个连续的7个0xaa
			{				
				if (file.tellg() >= nFileEnd)			//到文件末尾，退出循环
				{
					bParseCont = false;
					break;
				}
				// 看当前字符是不是0xaa，如果不是，则重新寻找7个连续的0xaa
				if (file.get() != 0xaa)
				{
					i = -1;
				}
			}
			
			// 如果直到文件结束仍没找到上述比特串，将终止主控循环的标记bParseCont置为true
			bParseCont = bParseCont && (file.tellg() < nFileEnd);													

			// 判断7个连续的0xaa之后是否为0xab
			if (bParseCont && file.get() == 0xab)		
			{
				break;
			}
		}

		// 计算数据字段的长度
		nCurrDataLength =								
			bParseCont ?								// 是否到达文件末尾
			(file.tellg() - 8 - 1 - nCurrDataOffset) :	// 没到文件末尾：下一帧头位置 - 前导码和定界符长度 - CRC校验码长度 - 数据字段起始位置
			(file.tellg() - 1 - nCurrDataOffset);		// 已到达文件末尾：文件末尾位置 - CRC校验码长度 - 数据字段起始位置

		
		// 以文本格式数据字段，并校验
		cout << endl << "数据字段：\t";	
		unsigned char* pData = new unsigned char[nCurrDataLength];	// 创建缓冲区
		file.seekg(bParseCont ? (-8 - 1 -nCurrDataLength) : ( -1 - nCurrDataLength), ios::cur);
		file.read(pData, nCurrDataLength);				// 读入数据字段
		
		int nCount = 50;								// 每行的基本字符数量
		for (i = 0; i < nCurrDataLength; i++)			// 输出数据字段文本					
		{
			nCount--;
			cout << pData[i];							// 字符输出
			checkCRC(nCheck, (int)pData[i]);			// CRC校验
			
			if ( nCount < 0)							// 换行处理
			{
				// 将行尾的单词写完整
				if ( pData[i] == ' ' )					
				{
					cout << endl << "\t\t";
					nCount = 50;
				}
				// 处理过长的行尾单词：换行并使用连字符
				if ( nCount < -10)						
				{
					cout<< "-" << endl << "\t\t";
					nCount = 50;
				}
			}
		}
		delete[] pData;									//释放缓冲区空间

		
		// 输出CRC校验码，如果CRC校验有误，则输出正确的CRC校验码
		cout << endl <<"CRC校验";
		c = file.get();								// 读入CRC校验码
		int nTmpCRC = nCheck;
		checkCRC(nCheck, c);						// 最后一步校验

		if ((nCheck & 0xff) == 0)					// CRC校验无误
		{
			cout.width(2);
			cout<<"(正确)：\t"<< hex << c;
		}
		else										// CRC校验有误
		{
			cout.width(2);
			cout<< "(错误)：\t" << hex << c;	
			checkCRC(nTmpCRC, 0);					// 计算正确的CRC校验码
			cout<< "\t应为：" << hex << (nTmpCRC & 0xff);
			bAccept = false;						// 将帧的接收标记置为false
		}
	
		//	如果数据字段长度不足46字节或数据字段长度超过1500字节，则将帧的接收标记置为false	
		if (nCurrDataLength < 46 ||	nCurrDataLength > 1500 )							
		{
			bAccept = false;
		}

		// 输出帧的接收状态
		cout<< endl << "状态：\t\t" << (bAccept ? "Accept" : "Discard") << endl <<endl;

		nSN++;									// 帧序号加1
		nCurrDataOffset = file.tellg() + 22;	// 将数据字段偏移量更新为下一帧的帧头结束位置

	}

	// 关闭输入文件
	file.close();

}