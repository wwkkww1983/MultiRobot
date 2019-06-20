/*
robotAPI
说  明：实现了基本通讯功能，操作机器人
制作人：邹智强
版  本：beta 0.9
更  新：
1、添加了对uArm机械臂的支持
*/


#pragma once
#include <opencv2\opencv.hpp>
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
#define ROBOT_CONNECTloseNUM 3 //允许错误的次数
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

 

/*******************************************************************************
* kobuki协议指令数据的结构体与类
*******************************************************************************/

typedef struct _StruRunControl
{
	unsigned char identifier;
	unsigned char lenth;
	unsigned short speed;
	unsigned short radius;
}StruRunControl;

typedef struct _StruSound
{
	unsigned char identifier;
	unsigned char lenth;
	unsigned short note;
	unsigned char duration;

}StruSound;


typedef struct _StruSoundSeq
{
	unsigned char identifier;
	unsigned char lenth;
	unsigned char SequenceNumber;

}StruSoundSeq;

typedef struct _StruPower
{
	unsigned char identifier;
	unsigned char lenth;
	unsigned short PowerFlags;

}StruPower;

typedef struct _StruExtraReq
{
	unsigned char identifier;
	unsigned char lenth;
	unsigned short ReqFlags;

}StruExtraReq;

typedef struct _StruNormolInput
{
	unsigned char identifier;
	unsigned char lenth;
	unsigned short Flags;

}StruNormolInput;

typedef struct _StruNetModel
{
	unsigned char identifier;
	unsigned char lenth;
	unsigned char STA[3];

}StruNetModel;

typedef struct _StruWifID
{
	unsigned char identifier;
	unsigned char lenth;
	unsigned char ids[100];

}StruWifID;

typedef struct _StruWifPWD
{
	unsigned char identifier;
	unsigned char lenth;
	unsigned char ids[100];

}StruWifPWD;

typedef struct _StruCommincateModel
{
	unsigned char identifier;
	unsigned char lenth;
	unsigned char date[4];

}StruCommincateModel;

typedef struct _StruFeedBackIndentifer
{
	unsigned short Timestamp;
	unsigned char Bumper;
	unsigned char WheelDrop;
	unsigned char Cliff;
	unsigned short LeftEncoder;
	unsigned short RightEncode;
	unsigned char LeftPWM;
	unsigned char RightPWM;
	unsigned char Button;
	unsigned char Charge;
	unsigned short int Battery;
	unsigned char OverCurrentFlags;

}StruFeedBackIndentifer;

typedef struct _StruInertialSensor
{
	short Angle;
	short AngleRate;
	unsigned char Unused[3];

}StruInertialSensor;

typedef struct _StruCliffSensor
{
	unsigned short RightCliff;
	unsigned short CentralCliff;
	unsigned short LeftCliff;

}StruCliffSensor;

typedef  struct _StruMotor
{
	short LeftMotor;
	short RightMotor;
}StruMotor;

typedef struct _StruHardVer
{
	unsigned char Patch;
	unsigned char Minor;
	unsigned char Major;
	unsigned char unused;
}StruHardVer;

typedef struct _StruFirmWare
{
	unsigned char Patch;
	unsigned char Minor;
	unsigned char Major;
	unsigned char unused;

}StruFirmWare;

typedef struct _Stru3DgyroScope
{
	unsigned char FrameID;
	unsigned char FollowLenth;
	struct {
		unsigned short x_axis;
		unsigned short y_axis;
		unsigned short z_axis;
	}RawGyro[100];
}Stru3DgyroScope;



typedef struct _StruUDID
{
	unsigned int UDID0;
	unsigned int UDID1;
	unsigned int UDID2;

}StruUDID;



typedef struct _StruUltrasonicData
{
	unsigned short DISL1;
	unsigned short DISL2;
	unsigned short DISL3;
	unsigned short DISL4;
	unsigned short DISL5;

}StruUltrasonicData;

typedef struct _StruRobotID
{
	unsigned char  id;
}StruRobotID;





//using namespace std;
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


/******************************************************************************
* //用于缓存线速度以及角速度的类，并且提供积分接口。
*******************************************************************************/
#define CasheQueue_MAXSIZE 20
class CasheQueuevw
{
public:
	/*
	@brief:初始化队列缓存。
	*/
	void init();

private:
	int lenth;
public://成员变量
	std::vector<float>  v;
	std::vector<float>  w;

public://接口
	   /*
	   @brief:线速度push。
	   */
	void vpush(float input);

	/*
	@brief:角速度push。
	*/
	void wpush(float input);

	/*
	@brief:返回线速度队列大小
	*/
	int vsize();

	/*
	@brief:返回角速度队列大小
	*/
	int wsize();

	/*
	@brief:返回线速度积分，积分下限为0.
	@param	detaT：时间微分，就是Ts/lenth
	@param	UPn：积分上限。
	*/
	float integralV(float detaT, int UPn = CasheQueue_MAXSIZE);

	/*
	@brief:返回角速度积分，积分下限为0.
	@param	detaT：时间微分，就是Ts/lenth
	@param	UPn：积分上限。
	*/
	float integralW(float detaT, int UPn = CasheQueue_MAXSIZE);

	/*
	@brief:返回延时时间内，补偿位移 △x
	@param	Ts：原定位系统延时时间
	@param	theta:n=0时的 绝对角度。
	@returns:返回补偿位移向量。
	*/
	cv::Point2f displace(float Ts, float theta);



};
/******************************************************************************
* 爱米家机器人返回数据类
*******************************************************************************/
class ClsRecvRobotInfo
{
public:
	StruFeedBackIndentifer feedBackIndentifer;
	StruInertialSensor InertialSensor;
	StruCliffSensor CliffSensor;
	StruMotor Motor;
	StruHardVer HardVer;
	StruFirmWare irmWare;
	Stru3DgyroScope DgyroScope;
	StruUDID UDID;
	StruUltrasonicData  UltrasonicData;
	StruRobotID RobotID;

	void init(char *buf, int length);
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
	//计数，如果有超过10次返回-1，则把connectStatusz置为异常
	int connectstatus_cout=0;

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
	float v = 0;//机器人线速度大小
	float w = 0;//机器人角速度大小

	//定位补偿估计的参数
	CasheQueuevw pvw;//线速度和角速度，这是要实时更新的队列缓存。

	/*  func   */
	
	INT8 move(float lin_val, float ang_val);
	void setTorque(char sta);
	float getVoltage(void);
	imu_msg getIMU();
	float getIllumination(void);
	uint8_t setLED(uint8_t lednum, uint8_t onoff);
	uint8_t getTorque();
	void initIMU(void);

};
/*******************************************************************************
* 定义aimijia机器人类
*******************************************************************************/
class AimiRobot
{
public:
	bool init(const char* ip, int port);
	int Connect();//阻塞连接

private:
	//电池电量
	int batteryVolt;



public:
	//专门为爱米家机器人开辟的线程
	HANDLE hThread;
	DWORD hThreadID;
	HANDLE hMutex;

	//网络连接状态
	robot_connectStatus_ret connectStatus;
	//机器人的ID号
	uint8_t robotID;
	//机器人的sock信息
	struct sockaddr_in socksin;
	//该机器人的通讯套接字
	SOCKET robotsock;
	//机器人状态，包括ID号
	ClsRecvRobotInfo robotInfo;


	//改变机器人运动
	float v = 0;//机器人线速度大小,在这里作为一个状态值，是可以通过外部改变的。
	float w = 0;//机器人角速度大小

	//定位补偿估计的参数
	CasheQueuevw pvw;//线速度和角速度，这是要实时更新的队列缓存。

	/*  func   */

	INT8 move();//通过vw变量，来改变机器人的运动速度。
	INT8 updateInfo();
	

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
	std::vector<robot> robotlist;//用于存储已经在通讯列表中的机器人，可以对这些机器人进行通讯控制
	AimiRobot aimirobot;//aimiplus机器人连接套接字
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



//-------------------------------------------------------
//		定义：uArmSocket::is_Open()的返回值的值域		-
//-------------------------------------------------------
enum uArmServerStatus_ret
{
	uArmSocket_BIND_SUCCESS = 1, //刚刚初始化bind成功

	uArmSocket_WSAERROR = -3, //初始化sock库版本号错误
	uArmSocket_INVALID_SOCKET = -2, //创建套接字失败
	uArmSocket_BINDERROR = -4, //bind失败
};


/*******************************************************************************
* 定义通讯套接字类:机械臂服务器
*******************************************************************************/
class uArmSocket
{
public:
	void init(int port);
	//查看ROBOTServer是否初始化成功，
	uArmServerStatus_ret is_Open();
	//监听
	int Listen(int maxacp);
	//接收
	int Accept();
	//获取本机IP地址
	bool GetLocalAddress(std::string& strAddress);
	//机械臂控制
	INT8 uArmControl(int x, int y, int z, int f);
	INT8 uArmControlsu();
	INT8 uArmControlsu(int x, int y, int z, int f);
	INT8 uArmqi(bool open);

private:
	uArmServerStatus_ret sock_Status; //存储本类是否创建成功

public:
	SOCKET ServerSock; //服务器的套接字，核心变量
	SOCKET uArmSock; //服务器的套接字，核心变量

	int uArmSock_status;


	//用于多线程 监听线程
	HANDLE hListenThread;
	DWORD ListenThreadID;
	//用于多线程 互斥锁
	HANDLE hMutex;

	//速度状态
	int su_x = 0, su_y = 0, su_z = 0;
	int su_f = 0;


};



