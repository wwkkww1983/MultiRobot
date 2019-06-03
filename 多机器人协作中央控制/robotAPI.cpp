/*
robotAPI
说  明：实现了基本通讯功能，操作机器人
制作人：邹智强
版  本：beta 0.7
更  新：
	 1、改进爱米家机器人的运动控制的参数解析
*/


#include "stdafx.h"
#include "robotAPI.h"


/*-----------------------------------------------------------------------------
CasheQueuevw函数接口定义
class
------------------------------------------------------------------------------*/
void CasheQueuevw::init()
{
	for (size_t i = 0; i < CasheQueue_MAXSIZE; i++)
	{
		v.push_back(0);
		w.push_back(0);
	}
}

void CasheQueuevw::vpush(float input)
{
	v.push_back(input);
	std::vector<float>::iterator iter = v.begin();
	v.erase(iter);
}
void CasheQueuevw::wpush(float input)
{
	w.push_back(input);
	std::vector<float>::iterator iter = w.begin();
	w.erase(iter);
}

int CasheQueuevw::vsize()
{
	return v.size();
}
int CasheQueuevw::wsize()
{
	return w.size();
}

float CasheQueuevw::integralV(float detaT, int UPn)
{
	float sum = 0;
	for (size_t i = 0; i < UPn; i++)
	{
		sum = sum + v[i];
	}
	return sum*detaT;
}
float CasheQueuevw::integralW(float detaT, int UPn)
{
	float sum = 0;
	for (size_t i = 0; i < UPn; i++)
	{
		sum = sum + w[i];
	}
	return sum*detaT;
}
/*
@brief:返回延时时间内，补偿位移 △x
*/

cv::Point2f CasheQueuevw::displace(float Ts,float theta)
{
	float detaT = Ts / CasheQueue_MAXSIZE;
	cv::Point2f sumv; sumv.x = 0; sumv.y = 0;
	for (size_t i = 0; i < CasheQueue_MAXSIZE; i++)
	{
		float thetai = integralW(detaT, i)+ theta;
		sumv.x = sumv.x + v[i]*0.8 * cos(thetai);
		sumv.y = sumv.y + v[i] * 0.8 * sin(thetai);
	}
	sumv.x = sumv.x*detaT;
	sumv.y = sumv.y*detaT;
	return sumv;
}

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

	pvw.init();

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
输入	lin_val		线速度大小 单位m/s
		ang_val		角速度大小 单位弧度/s
输出	1/-1		1正确  -1错误值
*******************************************************************************/
INT8 robot::move(float lin_val, float ang_val)
{
	float_to_char lin;
	float_to_char ang;
	
	lin.fval = -lin_val;
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
		v = lin_val;
		w = ang_val;
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

void robot::initIMU(void)
{
	char outbuf[SEND_LENGHT] = { 0xD8,0x00,0x00,0x11,0xC7 };
	int sendret = send(robotsock, outbuf, sizeof(outbuf), 0);

	if (sendret <= 0)
	{
		connectStatus = ROBOT_connectStatus_COMMUNICATEERROR;
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
		int timeout = 500; //3s
		setsockopt(socket_of_client, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
		setsockopt(socket_of_client, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
	
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


bool AimiRobot::init(const char* ip, int port)
{
	//初始化WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}
	//创建套接字
	robotsock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (robotsock == INVALID_SOCKET)
	{
		printf("invalid socket !");
		return 0;
	}

	//绑定IP和端口
	socksin.sin_family = AF_INET;
	socksin.sin_port = htons(port);
	socksin.sin_addr.S_un.S_addr = inet_addr(ip);

	//设置超时时间
	int nNetTimeout = 500; //1秒
	setsockopt(robotsock, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout, sizeof(int));
	setsockopt(robotsock, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));
	return 1;
}

int AimiRobot::Connect()
{
	while(connect(robotsock, (struct sockaddr *)&socksin, sizeof(socksin)) == SOCKET_ERROR)
	{
		printf("connect error !");
	}
	connectStatus = ROBOT_connectStatus_2Level;
	return 1;
}

INT8 AimiRobot::move()
{
	short speed = -v * 1000;
	short radius;
	if (w == 0)
	{
		radius = 0;
	}
	else
	{
		radius = speed / w;
	}

	if (v == 0)
	{
		radius = 1;
		speed = w;
	}

	std::vector<unsigned char> sendBytes;

	unsigned char Header0 = 0xAA;
	unsigned char Header1 = 0x55;
	unsigned char length = 0x00;
	unsigned char Checksun = 0x00;

	std::vector<unsigned char>  RunBytes;
	RunBytes.push_back(0x01);
	RunBytes.push_back(0x04);

	char buff[3] = { 0 };
	memcpy(buff, &speed, 2);
	RunBytes.push_back(buff[0]);
	RunBytes.push_back(buff[1]);

	memset(buff, 0, 3);
	memcpy(buff, &radius, 2);
	RunBytes.push_back(buff[0]);
	RunBytes.push_back(buff[1]);

	length = RunBytes.size();
	RunBytes.insert(RunBytes.begin(), length);

	for (int i = 0; i < RunBytes.size(); i++)
	{
		Checksun ^= RunBytes.at(i);
	}

	sendBytes.push_back(Header0);
	sendBytes.push_back(Header1);
	sendBytes.insert(sendBytes.end(), RunBytes.begin(), RunBytes.end());
	sendBytes.push_back(Checksun);


	char* sendData = new char[sendBytes.size() + 1];
	for (size_t i = 0; i < sendBytes.size(); i++)
	{
		sendData[i] = sendBytes[i];
	}
	sendData[sendBytes.size()] = '\n';
	int sendret = send(robotsock, sendData, 10, 0);

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

void ClsRecvRobotInfo::init(char *buf, int length)
{
	int ct = 0;
	while (ct<length)
	{
		if (buf[ct] == 1)
		{
			memcpy(&feedBackIndentifer, buf + ct + 2, buf[ct + 1]);
			ct = ct + buf[ct + 1] + 2;
		}
		else if (buf[ct] == 4)
		{
			memcpy(&InertialSensor, buf + ct + 2, buf[ct + 1]);
			ct = ct + buf[ct + 1] + 2;
		}
		else if (buf[ct] == 5)
		{
			memcpy(&CliffSensor, buf + ct + 2, buf[ct + 1]);
			ct = ct + buf[ct + 1] + 2;
		}
		else if (buf[ct] == 6)
		{
			memcpy(&Motor, buf + ct + 2, buf[ct + 1]);
			ct = ct + buf[ct + 1] + 2;
		}
		else if (buf[ct] == 10)
		{
			memcpy(&HardVer, buf + ct + 2, buf[ct + 1]);
			ct = ct + buf[ct + 1] + 2;
		}
		else if (buf[ct] == 11)
		{
			memcpy(&irmWare, buf + ct + 2, buf[ct + 1]);
			ct = ct + buf[ct + 1] + 2;
		}
		else if (buf[ct] == 13)
		{
			memcpy(&DgyroScope, buf + ct + 2, buf[ct + 1]);
			ct = ct + buf[ct + 1] + 2;
		}
		else if (buf[ct] == 19)
		{
			memcpy(&UDID, buf + ct + 2, buf[ct + 1]);
			ct = ct + buf[ct + 1] + 2;
		}
		else if (buf[ct] == 21)
		{
			memcpy(&UltrasonicData, buf + ct + 2, buf[ct + 1]);
			ct = ct + buf[ct + 1] + 2;
		}
		else if (buf[ct] == 23)
		{
			memcpy(&RobotID, buf + ct + 2, buf[ct + 1]);
			ct = ct + buf[ct + 1] + 2;
		}


	}
}

INT8 AimiRobot::updateInfo()
{
	char recData[255];
	int ret = recv(robotsock, recData, 255, 0);
	if (ret <= 0)
	{
		connectStatus = ROBOT_connectStatus_COMMUNICATEERROR;
		return -1;
	}

	unsigned char Header0 = 0;
	unsigned char Header1 = 0;
	unsigned char length = 0x00;
	unsigned char Checksun = 0x00;

	Header0 = recData[0]; Header1 = recData[1];
	length = recData[2];
	Checksun = recData[length + 3];
	char* databytes = new char[length];

	unsigned char cs = 0;
	for (unsigned int i = 2; i < length + 3; i++)
	{
		cs ^= recData[i];
	}
	if (cs == Checksun && Header0 == 0xaa && Header1 == 0x55)
	{
		memcpy(databytes, recData + 3, length);
		robotInfo.init(databytes, length);
	}


	return 1;
}