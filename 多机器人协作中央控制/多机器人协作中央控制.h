
// 多机器人协作中央控制.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号
#include "robotAPI.h"
#include "zfun.h"
#include "IPClocation.h"

using namespace std;
// CMultiRobotApp: 
// 有关此类的实现，请参阅 多机器人协作中央控制.cpp
//

class CMultiRobotApp : public CWinApp
{
public:
	CMultiRobotApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现
	ROBOTServer robotServer;
	IPClocation visionLSys;//视觉定位系统
	vector<IPCobj> obj;//场地中的物体
	vector<vector<IPCobj>> everyIPCobj;

	//另外的一些传输信息
	vector<Mat> IPCshowImg;//从摄像头上提取出来的经过处理的画面，会实时更新。
	int seleteimshow = -2;//选择哪一个显示img，-1则是全显示,-2不显示
	int ThreadOn = 1; //=0代表退出线程
	
	//config
	FileStorage config;

	//标志位配置
	bool show2Dflag = false;//显示2D地图
	bool movecompFlag = true;//运动补偿

	Size showsize;//监控显示窗口的大小

	int robotlist_checkIndex;//存储目前选中了robot列表中的第几个机器人。



//加载logo
	ULONG_PTR m_gdiplusToken;
//任务！
	struct task
	{
		//1代表to point任务
		//2:倒退着走的任务，倒退着走到指定地点。
		//3:转到指定角度
		int taskname; 
		float x;
		float y;
		int lf;//摆正姿态的时候是向左转还是向右转，左转为1 右为-1
	};
	vector<vector<task>> taskqueue;//任务队列，移动有4个，按顺序对应着机器人列表中的机器人。

	vector<task> aimiTaskQueue; //任务队列，暂时是爱米家机器人的任务队列。
						   
	//爱米家机器人的控制任务
	HANDLE haimiTaskMutex;//用于多线程 互斥锁,对以上变量进行加锁。
	HANDLE haimiTaskrunThread;//任务执行线程线程句柄
	DWORD aimiTaskrunThreadID;//任务执行线程线程ID

	//小机器人控制任务
	HANDLE hTaskMutex;//用于多线程 互斥锁,上变量进行加锁。
	HANDLE hTaskrunThread[4];//任务执行线程线程句柄
	DWORD TaskrunThreadID[4];//任务执行线程线程ID

	//中央调度大任务
	int bigtask=0;//0:没有任务 1：物体分拣任务 2：机器人编队任务
	HANDLE MultiRobotControl_Mutex;//用于多线程 互斥锁,上变量进行加锁。
	HANDLE hMultiRobotControlThread;//任务执行线程线程句柄
	DWORD MultiRobotControlThreadID;//任务执行线程线程ID

	//完成抓取信号
	int finishGet_flag=0;

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CMultiRobotApp theApp;
