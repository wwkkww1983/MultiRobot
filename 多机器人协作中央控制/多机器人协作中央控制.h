
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
	uArmSocket uArm;//机械臂控制
	ROBOTServer robotServer;
	IPClocation visionLSys;//视觉定位系统
	vector<IPCobj> obj;//场地中的物体
	vector<vector<IPCobj>> everyIPCobj;

	//另外的一些传输信息
	HANDLE IPCshowImgMutex;//专门为IPCshowImg定制的互斥锁。
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
		//1:代表to point任务
		//2:倒退着走的任务，倒退着走到指定地点。
		//3:转到指定角度
		//4:机械臂独有指令，让机械臂运动到某个位置，抓取再放到指定位置。x,y,z  to  x1,y1,z1，但是不放下物体
		//5:机械臂独有指令，让机械臂放下物体。
		int taskname; 
		float x;
		float y;
		int lf;//摆正姿态的时候是向左转还是向右转，左转为1 右为-1 ,以最短距离转圈为0
		float z;
		float x1;
		float y1;
		float z1;
	};
	
	vector<vector<task>> taskqueue;//任务队列，移动有4个，按顺序对应着机器人列表中的机器人。
	void taskqueue_push(int index,int taskn,Point2f toP,int lf=1);

	vector<task> aimiTaskQueue; //任务队列，暂时是爱米家机器人的任务队列。

	vector<task> uArmTaskQueue; //任务队列，机械臂的任务队列
	void uArmTaskQueue_push(int taskn, Vec3f p1, Vec3f p2);

						   
	//爱米家机器人的控制任务
	HANDLE haimiTaskMutex;//用于多线程 互斥锁,对任务队列进行加锁。
	HANDLE haimiTaskrunThread;//任务执行线程线程句柄
	DWORD aimiTaskrunThreadID;//任务执行线程线程ID

	//小机器人控制任务
	HANDLE hTaskMutex;//用于多线程 互斥锁,任务队列进行加锁。
	HANDLE hTaskrunThread[4];//任务执行线程线程句柄
	DWORD TaskrunThreadID[4];//任务执行线程线程ID

	//机械臂的控制任务
	HANDLE huArmTaskMutex;//用于多线程 互斥锁,对任务队列进行加锁。
	HANDLE huArmTaskrunThread;//任务执行线程线程句柄
	DWORD uArmTaskrunThreadID;//任务执行线程线程ID

	//中央调度大任务
	int bigtask=0;//0:没有任务 1：物体分拣任务 2：机器人编队任务
	HANDLE MultiRobotControl_Mutex;//用于多线程 互斥锁,上变量进行加锁。
	HANDLE hMultiRobotControlThread;//任务执行线程线程句柄
	DWORD MultiRobotControlThreadID;//任务执行线程线程ID

	//用于打印在mfc界面的输出框，跨线程操作
	CString printd_str;
	HANDLE printd_str_Mutex;//printd_str变量的互斥锁。
	void printd(CString cout);//输出字符串信息
	void printd(string cout);//输出字符串信息
	void printd(int cout);
	void printd(float cout);

	//识别到的物体位置
	vector<IPCobj> rgbobj;
	HANDLE rgbobj_Mutex;//rgbobj变量的互斥锁。


	//完成抓取信号
	int finishGet_flag=0;

	//定位轨迹记录:空间坐标加时间坐标（x,y,z,t）
	vector<Vec4f> track_data;
	bool track_data_flag;

	//判断识别到的物体是否在机械臂可抓取范围内
	INT8 judgeGrabRange(vector<IPCobj> inputobj);//负数代表抓不到，0代表抓得到。

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CMultiRobotApp theApp;
