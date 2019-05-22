
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
	int seleteimshow = -1;//选择哪一个显示img，-1则是全显示
	int ThreadOn = 1; //=0代表退出线程

	//config
	FileStorage config;

	//标志位配置
	bool show2Dflag = false;//显示2D地图
	bool movecompFlag = false;//运动补偿

	DECLARE_MESSAGE_MAP()
};

extern CMultiRobotApp theApp;
