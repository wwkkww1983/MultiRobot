/*
robotAPI
说  明：实现了基本通讯功能，操作机器人
制作人：邹智强
版  本：beta 0.2
更  新：
1、添加了：socket监听套接字的超时设置
2、添加了：得到本地服务器IP地址的接口
*/


#pragma once
#include <stdio.h>
#include <Winsock2.h>
#include <cstring>
#include <iostream>
#include <WS2tcpip.h>
#include <time.h>
#include <windows.h>
#include <vector>


#pragma comment(lib,"ws2_32.lib")
#define ServerIPport 6000
#define SEND_LENGHT 128 //一个数据包的包长
#define ROBOT_MAXCONNECT_NUM 5 //机器人最大连接个数
//-------------------------------------------------------
//		定义：ROBOTServer::is_Open()的返回值的值域		-
//-------------------------------------------------------
enum ServerStatus_ret
{
	ROBOTServer_BIND_SUCCESS=1, //刚刚初始化bind成功

	ROBOTServer_WSAERROR=-3, //初始化sock库版本号错误
	ROBOTServer_INVALID_SOCKET=-2, //创建套接字失败
	ROBOTServer_BINDERROR=-4, //bind失败
};

//-------------------------------------------------------
//		定义：getVoltage的返回值的值域		            -
//-------------------------------------------------------
#define getVoltage_SOCKET_ERROR -3 //表示套接字有错误
#define getVoltage_SOCKET_CLOSE -2 //表示套接字已经关闭
#define getVoltage_FORMAT_ERROR -1 //表示接收到的数据格式有误，或者不是电压值


//-------------------------------------------------------
//		定义：robot::connectStatus的值域,网络连接状态	-
//-------------------------------------------------------
enum robot_connectStatus_ret
{
	ROBOT_connectStatus_CONNECTERROR=-1, //异常，一开始就连接异常，就是说在创建robot的时候，再次检查ID号，出现错误。
	ROBOT_connectStatus_CHECKERROR =-2,   //检查时网络中断（getVoltage函数中返回异常）
	ROBOT_connectStatus_COMMUNICATEERROR= -3, //在执行通讯函数中，网络中断

	ROBOT_connectStatus_1Level= 1, //网络连接正常，通讯质量比较差。
	ROBOT_connectStatus_2Level= 2, //网络连接正常，通讯质量一般。
	ROBOT_connectStatus_3Level= 3, //网络连接正常，通讯质量很好。
};

 






using namespace std;
/*******************************************************************************
* 定义机器人指令数据的结构体
*******************************************************************************/
struct rebotATmsg
{
	/* data */
	uint8_t header;//包头标识
	int8_t ret;//返回正确值，
	uint8_t cmd;
	uint8_t parmNum;
	uint8_t callret;
	uint8_t parmdata[8][64];
};
struct imu_msg
{
	//角速度
	float angular_velocity_x;
	float angular_velocity_y;
	float angular_velocity_z;
	//线性加速度
	float linear_acceleration_x;
	float linear_acceleration_y;
	float linear_acceleration_z;
	//姿态 四元数
	float orientation_w;
	float orientation_x;
	float orientation_y;
	float orientation_z;
};
/*******************************************************************************
* 定义数据转化的公用体,用于数据转换
*******************************************************************************/
union float_to_char
{
	/* data */
	float fval;
	uint8_t bval[4];
};
union int32_to_char
{
	/* data */
	int32_t intval;
	uint8_t bval[4];
};

union imumsg_to_char
{
	/* data */
	imu_msg imuval;
	uint8_t bval[40];
};

/*******************************************************************************
* 定义机器人类
*******************************************************************************/
class robot
{
public:
	robot(SOCKET socket, struct sockaddr_in sin);

	
private:
	//电池电量
	int batteryVolt;
	/*  func   */
	rebotATmsg resolverMsg(char rbuf[SEND_LENGHT]);
	uint8_t getID(void);

public:
	//网络连接状态
	robot_connectStatus_ret connectStatus;
	//机器人的ID号
	uint8_t robotID;
	//机器人的sock信息
	struct sockaddr_in socksin;
	//该机器人的通讯套接字
	SOCKET robotsock;
	//
	float Voltage;

	/*  func   */
	
	INT8 move(float lin_val, float ang_val);
	void setTorque(char sta);
	float getVoltage(void);
	imu_msg getIMU();
	float getIllumination(void);
	uint8_t setLED(uint8_t lednum, uint8_t onoff);
	uint8_t getTorque();


};

/*******************************************************************************
* 定义通讯套接字类:监听服务器
* 机器人入队列，多机器人通讯管理
*******************************************************************************/
class ROBOTServer
{
public:
	//ROBOTServer();
	void init(int port);

private:
	ServerStatus_ret sock_Status; //存储本类是否创建成功
	

public:
	SOCKET ServerSock; //服务器的套接字，核心变量
	vector<robot> robotlist;//用于存储已经在通讯列表中的机器人，可以对这些机器人进行通讯控制
	//用于多线程 监听线程
	HANDLE hListenThread;
	DWORD ListenThreadID;
	//用于多线程 互斥锁
	HANDLE hMutex;

	//查看ROBOTServer是否初始化成功，
	ServerStatus_ret is_Open();
	//监听
	int Listen(int maxacp);
	//接收
	int Accept();
	//得到robotlist的连接个数
	int getRobotListNum();
	//得到robotlist中第几个robot的IP地址
	sockaddr_in getRobotIP(int robotlistCont);
	//在robotlist中查找ID号并返回引索，-1表示没有
	int findID(uint8_t robotid);
	//获取本机IP地址
	bool GetLocalAddress(std::string& strAddress);

	
};





