/*
robotAPI
说  明：实现了基本通讯功能，操作机器人
制作人：邹智强
版  本：beta 0.2
更  新：
	1、添加了：socket监听套接字的超时设置
	2、添加了：得到本地服务器IP地址的接口
*/


#include "stdafx.h"
#include "robotAPI.h"


/*-----------------------------------------------------------------------------
						robot函数接口定义
							 class
------------------------------------------------------------------------------*/
robot::robot(SOCKET socket, struct sockaddr_in sin)
{ 
	robotsock = socket;
	socksin = sin;
	//刷新机器人固件信息
	robotID = getID();
	if (robotID == 0)
	{
		connectStatus = ROBOT_connectStatus_CONNECTERROR;
	}
	else
	{
		connectStatus = ROBOT_connectStatus_3Level;
	}

}



/*******************************************************************************
* 说明：返回机器人ID号
*******************************************************************************/
uint8_t robot::getID(void)
{
	char outbuf[SEND_LENGHT] = { 0xD8, 0x00, 0x00, 0x04, 0xC7 };
	send(robotsock, outbuf, sizeof(outbuf), 0);

	char recvbuf[SEND_LENGHT];
	int recvLen=recv(robotsock, recvbuf, sizeof(recvbuf), 0);
	if (recvLen == 0)
	{
		return 0;
	}
	else if (recvLen == SOCKET_ERROR)
	{
		return 0;
	}

	rebotATmsg atmsg = resolverMsg(recvbuf);

	if (atmsg.ret > 0 && atmsg.callret == 1 && atmsg.cmd == 0x04)
	{
		return atmsg.parmdata[0][0];
	}
	else
	{
		return 0;
	}

}
/*******************************************************************************
* 机器人移动命令
说明：给定机器人移动速度与转弯角度来控制机器人的移动，
输入	lin_val		移动速度，0-0.2为正常值，这个参数对基于两个电机转速
		ang_val		这个参数会基于两个电机一个反相值一般为0-1.8
输出	1/-1		1正确  -1错误值
*******************************************************************************/
INT8 robot::move(float lin_val, float ang_val)
{
	float_to_char lin;
	float_to_char ang;
	
	lin.fval = lin_val;
	ang.fval = ang_val;

	char outbuf[SEND_LENGHT] = { 0xd8,0x02,0x00,0x02,0x04,0,0,0,0,0x04,0,0,0,0 ,0xc7};
	for (int i = 0; i < 4; i++)
	{
		outbuf[5 + i] = lin.bval[i];
		outbuf[10 + i] = ang.bval[i];
	}
	int sendret=send(robotsock, outbuf, sizeof(outbuf), 0);

	if (sendret <= 0)
	{
		connectStatus = ROBOT_connectStatus_COMMUNICATEERROR;
		return -1;
	}
	else
	{
		return 1;
	}

	
}

/*******************************************************************************
* 电机强制停转
说明：电机开启关闭：1是开启，0是关闭（同时会初始化），关闭之后如果要重新运动的话，
	  应该先1开启再执行move
输入	sta	1是开启，0是关闭
*******************************************************************************/
void robot::setTorque(char sta)
{
	char outbuf[SEND_LENGHT] = { 0xd8,0x01,0x00,0x03,0x01,0x01,0xc7 };
	outbuf[5] = sta;
	int sendret = send(robotsock, outbuf, sizeof(outbuf), 0);

	if (sendret <= 0)
	{
		connectStatus = ROBOT_connectStatus_COMMUNICATEERROR;
		
	}

}

/*******************************************************************************
* 解释接收到的函数命令，并且释放内存。
*******************************************************************************/
rebotATmsg robot::resolverMsg(char rbuf[SEND_LENGHT])
{
	rebotATmsg retmsgz;
	int readi = 0;
	uint8_t header0 = rbuf[readi]; readi++;
	if (header0 == 0xD8 || header0 == 0xD9)
	{
		retmsgz.header = header0;
		retmsgz.parmNum = rbuf[readi]; readi++;
		retmsgz.callret = rbuf[readi]; readi++;
		retmsgz.cmd = rbuf[readi]; readi++;
		//读参数数据
		for (int i = 0; i<retmsgz.parmNum; i++)
		{
			uint8_t inum = rbuf[readi]; readi++;
			for (int j = 0; j < inum; j++)
			{
				retmsgz.parmdata[i][j]= rbuf[readi]; readi++;
			}
		}
		//判断包尾是否正确
		if ((UINT8)rbuf[readi] == 0xc7) { retmsgz.ret = 1; }
		else retmsgz.ret = -1;

		return retmsgz;
	}
	else
	{
		retmsgz.ret = -1;
		return retmsgz;
	}
}

/*******************************************************************************
* 得到电池电压，会阻塞(非阻塞用updata)
*******************************************************************************/
imu_msg robot::getIMU()
{
	
	imumsg_to_char imubuf;
	char outbuf[SEND_LENGHT] = { 0xD8, 0x00, 0x00, 0x07, 0xC7 };
	send(robotsock, outbuf, sizeof(outbuf), 0);

	
	char recvbuf[SEND_LENGHT];
	int recvLen = recv(robotsock, recvbuf, sizeof(recvbuf), 0);
	if (recvLen<=0)
	{
		connectStatus = ROBOT_connectStatus_COMMUNICATEERROR;
	}
	
	
	rebotATmsg atmsg = resolverMsg(recvbuf);
	if (atmsg.ret > 0 && atmsg.callret == 1 && atmsg.cmd == 0x07)
	{
		for (int i = 0; i<40; i++)
		{
			imubuf.bval[i] = atmsg.parmdata[0][i];
		}
	}
	
	return imubuf.imuval;
}

/*******************************************************************************
* 得到电池电压，会阻塞(非阻塞用updata),检查网络特性
*******************************************************************************/
float robot::getVoltage(void)
{
	DWORD starttime = GetTickCount();
	char outbuf[SEND_LENGHT] = { 0xD8, 0x00, 0x00, 0x05, 0xC7 };
	send(robotsock, outbuf, sizeof(outbuf), 0);

	char recvbuf[SEND_LENGHT];
	int recvLen=recv(robotsock, recvbuf, sizeof(recvbuf), 0);
	//检查网络质量
	if (recvLen == 0)
	{
		connectStatus = ROBOT_connectStatus_CHECKERROR;
		return -2;
	}
	else if(recvLen==SOCKET_ERROR)
	{
		connectStatus = ROBOT_connectStatus_CHECKERROR;
		return -3;
	}
	else
	{
		DWORD pingms = GetTickCount()- starttime;
		if(pingms<1) connectStatus = ROBOT_connectStatus_3Level;
		else if(pingms < 10) connectStatus = ROBOT_connectStatus_2Level;
		else connectStatus = ROBOT_connectStatus_1Level;
	}
	

	rebotATmsg atmsg = resolverMsg(recvbuf);
	if (atmsg.ret > 0 && atmsg.callret == 1 && atmsg.cmd == 0x05)
	{
		//获取参数
		float_to_char volt;
		for (int i = 0; i<4; i++)
		{
			volt.bval[i] = atmsg.parmdata[0][i];
		}
		return volt.fval;
	}
	else
	{
		return -1;
	}

}
/*******************************************************************************
* 得到光照强度，会阻塞(非阻塞用updata)（与getVoltage公用返回域）
*******************************************************************************/
float robot::getIllumination(void)
{
	char outbuf[SEND_LENGHT] = { 0xD8, 0x00, 0x00, 0x09, 0xC7 };
	send(robotsock, outbuf, sizeof(outbuf), 0);

	char recvbuf[SEND_LENGHT];
	int recvLen = recv(robotsock, recvbuf, sizeof(recvbuf), 0);
	if (recvLen == 0)
	{
		connectStatus = ROBOT_connectStatus_CHECKERROR;
		return -2;
	}
	else if (recvLen == SOCKET_ERROR)
	{
		connectStatus = ROBOT_connectStatus_CHECKERROR;
		return -3;
	}


	rebotATmsg atmsg = resolverMsg(recvbuf);
	if (atmsg.ret > 0 && atmsg.callret == 1 && atmsg.cmd == 0x09)
	{
		//获取参数
		float_to_char Illumi;
		for (int i = 0; i<4; i++)
		{
			Illumi.bval[i] = atmsg.parmdata[0][i];
		}
		return Illumi.fval;
	}
	else
	{
		return -1;
	}
}


/*******************************************************************************
* 控制LED灯
输入	lednum	led编号（1-4）
		Onoff	开灯/关灯（1/0）
返回	Ret	通讯是否成功（send返回值）
*******************************************************************************/
uint8_t robot::setLED(uint8_t lednum, uint8_t onoff)
{
	char outbuf[SEND_LENGHT] = { 0xD8,0x02,0x00,0x08,0x01,0,0x01,0,0xC7 };
	outbuf[5] = lednum;
	outbuf[7] = onoff;

	int sendret=send(robotsock, outbuf, sizeof(outbuf), 0);
	if (sendret <= 0)
	{
		connectStatus = ROBOT_connectStatus_COMMUNICATEERROR;
		return -1;
	}
	else
	{
		return 1;
	}
}

/*******************************************************************************
* 返回机器人电机使能信息
输入	
返回	使能信息,1是开启 0是关断
*******************************************************************************/
uint8_t robot::getTorque()
{
	char outbuf[SEND_LENGHT] = { 0xD8,0x00,0x00,0x10,0xC7 };
	send(robotsock, outbuf, sizeof(outbuf), 0);

	char recvbuf[SEND_LENGHT];
	int recvLen = recv(robotsock, recvbuf, sizeof(recvbuf), 0);
	if (recvLen <= 0)
	{
		connectStatus = ROBOT_connectStatus_COMMUNICATEERROR;
	}


	rebotATmsg atmsg = resolverMsg(recvbuf);
	if (atmsg.ret > 0 && atmsg.callret == 1 && atmsg.cmd == 0x10)
	{
		return atmsg.parmdata[0][0];
	}
	else
	{
		return -1;
	}

	 

}


/*-----------------------------------------------------------------------------
						 ROBOTServer函数接口定义
								class
------------------------------------------------------------------------------*/

void ROBOTServer::init(int port)
{

	//初始化Socket
	WORD socket_version = MAKEWORD(2, 2);
	WSADATA wsadata;
	if (WSAStartup(socket_version, &wsadata) != 0)
	{
		sock_Status = ROBOTServer_WSAERROR;
		return;
	}


	ServerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//创建socket 并判断是否创建成功
	if (ServerSock == INVALID_SOCKET)
	{
		sock_Status = ROBOTServer_INVALID_SOCKET;
		return;
	}


	//设置服务器Socket地址
	struct sockaddr_in s_sin; //用于存储本地创建socket的基本信息
	s_sin.sin_family = AF_INET;  //定义协议族为IPV4
	s_sin.sin_port = htons(port);//规定端口号
	s_sin.sin_addr.S_un.S_addr = INADDR_ANY;

	//绑定Socket Server到本地地址
	if (bind(ServerSock, (LPSOCKADDR)&s_sin, sizeof(s_sin)) == SOCKET_ERROR)//绑定
	{
		sock_Status = ROBOTServer_BINDERROR;
		return;
	}
	//创建bind成功
	sock_Status = ROBOTServer_BIND_SUCCESS;

	//设置超时
	int nNetTimeout = 2000;
	if (SOCKET_ERROR == setsockopt(ServerSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int)))
	{
		printf("Set Ser_RecTIMEO error !\r\n");
	}

	return;
}




ServerStatus_ret ROBOTServer::is_Open()
{
	return sock_Status;
}
int ROBOTServer::Listen(int maxacp)
{
	return listen(ServerSock, maxacp);
}

//阻塞式接收机器人的连接
int ROBOTServer::Accept()
{
	SOCKET socket_of_client;  //客户端（远程）的socket
	struct sockaddr_in c_sin; //用于存储已连接的客户端的socket基本信息
	int    c_sin_len;         //函数accept的第三个参数，c_sin的大小。

	c_sin_len = sizeof(c_sin);

	socket_of_client = accept(ServerSock, (SOCKADDR *)&c_sin, &c_sin_len);

	//接收新用户
	if (socket_of_client == INVALID_SOCKET)
	{
		return -1;//Accept error
	}
	else
	{
		robot robotnew(socket_of_client, c_sin);
		robotlist.push_back(robotnew);

	}
}

//得到robotlist的连接个数
int ROBOTServer::getRobotListNum()
{
	return robotlist.size();
}
//得到robotlist中第几个robot的IP地址
sockaddr_in ROBOTServer::getRobotIP(int robotlistCont)
{
	sockaddr_in ipport;
	if (robotlistCont >= getRobotListNum())
	{
		perror("getRobotIP不存在编号");
	}

	return robotlist[robotlistCont].socksin;
}

int ROBOTServer::findID(uint8_t robotid)
{
	for (size_t i = 0; i < getRobotListNum(); i++)
	{
		if (robotid == robotlist[i].robotID)
		{
			return i;
		}
	}
	return -1;
}

bool ROBOTServer::GetLocalAddress(std::string& strAddress)
{
	char strHost[30] = { 0 };
	
	// get host name, if fail, SetLastError is called  
	if(SOCKET_ERROR!=gethostname(strHost, sizeof(strHost)))
	{
		struct hostent* hp;
		hp = gethostbyname(strHost);
		int i = 0;
		// IPv4: Address is four bytes (32-bit)  
		if (hp->h_length < 4)
			return false;

		while (hp != NULL&&hp->h_addr_list[i] != NULL)
		{
			//如果网段小于10（就是路由器的局域网一般的范围）
			
			if((UINT)(((PBYTE)hp->h_addr_list[i])[2])<10)
			{
				// Convert address to . format  
				strHost[0] = 0;

				// IPv4: Create Address string  
				sprintf(strHost, "%u.%u.%u.%u",
					(UINT)(((PBYTE)hp->h_addr_list[i])[0]),
					(UINT)(((PBYTE)hp->h_addr_list[i])[1]),
					(UINT)(((PBYTE)hp->h_addr_list[i])[2]),
					(UINT)(((PBYTE)hp->h_addr_list[i])[3]));

				strAddress = strHost;
				return true;
			}
			i++;
		}
		return false;
		
	}
	else
		SetLastError(ERROR_INVALID_PARAMETER);
	return false;
}



