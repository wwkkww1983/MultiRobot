
// MultiRobotDlg.cpp : 实现文件
//


#include "stdafx.h"
#include "多机器人协作中央控制.h"
#include "MultiRobotDlg.h"
#include "afxdialogex.h"
#include "directionDlg.h"
#include "addipcDlg.h"





#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMultiRobotDlg 对话框



CMultiRobotDlg::CMultiRobotDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MY_DIALOG, pParent)
	, m_voltage(0)
	, m_angular_velocity_x(0)
	, m_angular_velocity_y(0)
	, m_angular_velocity_z(0)
	, m_linear_acceleration_x(0)
	, m_linear_acceleration_y(0)
	, m_linear_acceleration_z(0)
	, m_roll(0)
	, m_pitch(0)
	, m_yaw(0)
	, m_movelin_display(0)
	, m_moveang_display(0)
	, m_rtsp(_T(""))
	, m_direction(0)
	, m_delaytime(0)
	, m_showimg(FALSE)
	, m_aimibotID(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMultiRobotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Control(pDX, IDC_MFCVSLISTBOX2, m_RobotList);
	DDX_Control(pDX, IDC_LIST4, m_RobotList);
	DDX_Text(pDX, IDC_EDIT1, m_voltage);
	DDX_Text(pDX, IDC_EDIT2, m_angular_velocity_x);
	DDX_Text(pDX, IDC_EDIT3, m_angular_velocity_y);
	DDX_Text(pDX, IDC_EDIT4, m_angular_velocity_z);
	DDX_Text(pDX, IDC_EDIT5, m_linear_acceleration_x);
	DDX_Text(pDX, IDC_EDIT6, m_linear_acceleration_y);
	DDX_Text(pDX, IDC_EDIT7, m_linear_acceleration_z);
	DDX_Text(pDX, IDC_EDIT8, m_roll);
	DDX_Text(pDX, IDC_EDIT9, m_pitch);
	DDX_Text(pDX, IDC_EDIT10, m_yaw);
	DDX_Control(pDX, IDC_CHECK1, m_moveEnable);
	DDX_Control(pDX, IDC_SLIDER1, m_movelin);
	DDX_Control(pDX, IDC_SLIDER2, m_moveang);
	DDX_Text(pDX, IDC_EDIT11, m_movelin_display);
	DDV_MinMaxInt(pDX, m_movelin_display, -200, 200);
	//  DDX_Control(pDX, IDC_EDIT12, m_moveang_display);
	DDX_Text(pDX, IDC_EDIT12, m_moveang_display);
	DDV_MinMaxInt(pDX, m_moveang_display, -180, 180);
	DDX_Control(pDX, IDC_CHECK2, m_wsadFlag);
	DDX_Control(pDX, IDC_EDIT13, m_disIPaddr);
	DDX_Control(pDX, IDC_LIST1, m_IPClist);
	DDX_Text(pDX, IDC_EDIT14, m_rtsp);
	DDX_Text(pDX, IDC_EDIT15, m_direction);
	DDX_Text(pDX, IDC_EDIT16, m_delaytime);
	DDX_Control(pDX, IDC_EDIT17, m_printout);
	DDX_Check(pDX, IDC_CHECK3, m_showimg);
	DDX_Text(pDX, IDC_EDIT18, m_aimibotID);
}

BEGIN_MESSAGE_MAP(CMultiRobotDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK1, &CMultiRobotDlg::OnBnClickedCheck1)
//	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER1, &CMultiRobotDlg::OnTRBNThumbPosChangingSlider1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CMultiRobotDlg::OnNMCustomdrawSlider1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, &CMultiRobotDlg::OnNMCustomdrawSlider2)
	ON_BN_CLICKED(IDC_BUTTON1, &CMultiRobotDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMultiRobotDlg::OnBnClickedButton2)
	ON_COMMAND(ID_32775, &CMultiRobotDlg::On32775)
	ON_LBN_SELCHANGE(IDC_LIST1, &CMultiRobotDlg::OnLbnSelchangeList1)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON3, &CMultiRobotDlg::OnBnClickedButton3)
	ON_COMMAND(ID_32777, &CMultiRobotDlg::On32777)
	ON_COMMAND(ID_32778, &CMultiRobotDlg::On32778)
	ON_COMMAND(ID_32779, &CMultiRobotDlg::Onshow2Donoff)
	ON_COMMAND(ID_32771, &CMultiRobotDlg::Onvision)
	ON_COMMAND(ID_32780, &CMultiRobotDlg::OnAddIPC)
	ON_LBN_DBLCLK(IDC_LIST1, &CMultiRobotDlg::OnLbnDblclkList1)
	ON_BN_CLICKED(IDC_CHECK3, &CMultiRobotDlg::OnBnClickedCheck3)
	ON_COMMAND(ID_32782, &CMultiRobotDlg::OnTest_toPoint)
	ON_COMMAND(ID_32784, &CMultiRobotDlg::OnsetTask)
	ON_COMMAND(ID_32787, &CMultiRobotDlg::OnfinishGet_flag)
	ON_EN_CHANGE(IDC_EDIT14, &CMultiRobotDlg::OnEnChangeEdit14)
	ON_COMMAND(ID_32789, &CMultiRobotDlg::OnSaveData)
END_MESSAGE_MAP()


// CMultiRobotDlg 消息处理程序

BOOL CMultiRobotDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	//***************************************** TODO: 在此添加额外的初始化代码
	
	
	//初始化菜单
	m_Menu.LoadMenu(IDR_MENU1);
	SetMenu(&m_Menu);
	//读取json 设置文件
	theApp.config.open("config.json", cv::FileStorage::READ);
	theApp.config["trackData"] >> theApp.track_data_flag;
	//zzq测试代码
	int ipport;
	theApp.config["IPport"]>> ipport;

	theApp.robotServer.init(ipport);
	if (theApp.robotServer.is_Open() < 0)
	{
		AfxMessageBox(_T("服务器初始化失败"));
	}
	theApp.robotServer.hMutex = CreateMutex(NULL, FALSE, NULL);
	//监控显示的大小
	if (theApp.config["imshowSize"] == "1280x720")
	{
		theApp.showsize = Size(1280, 720);
	}
	else if(theApp.config["imshowSize"] == "1920x1080")
	{
		theApp.showsize = Size(1920, 1080);
	}
	theApp.config["movecompFlag"] >> theApp.movecompFlag;
	theApp.config["estimation_Algorithm"] >> theApp.visionLSys.estimation_Algorithm;


	//开启监听线程
	theApp.robotServer.hListenThread = CreateThread(NULL, 0, ListenAcceptThreadFun, NULL, 0, &theApp.robotServer.ListenThreadID);
	//设置对话框刷新时间
	SetTimer(1, 500, NULL);
	//定时刷新机器的线速度角速度。
	SetTimer(2, theApp.visionLSys.delayTime / CasheQueue_MAXSIZE, NULL);

	m_voltage = 0;
	//初始化运动控制滑动条
	m_movelin.SetRange(-50, 50);
	m_movelin.SetTicFreq(5);
	m_movelin.SetPos(0);
	//m_movelin.SetLineSize(5);//一行的大小，对应键盘的方向键
	m_moveang.SetRange(-180, 180);
	m_moveang.SetTicFreq(5);
	m_moveang.SetPos(0);
	//显示本机IP
	string strip;
	theApp.robotServer.GetLocalAddress(strip);
	CString cstrip(strip.c_str());
	CString ipportstr;
	ipportstr.Format(_T("%d"), ipport);
	cstrip = cstrip + ":" + ipportstr;
	m_disIPaddr.ReplaceSel(cstrip);
	//初始化IPC
	theApp.visionLSys.bindxml("IPCxml.xml");
	theApp.visionLSys.initMap("map.jpg");
	theApp.visionLSys.hMutex = CreateMutex(NULL, FALSE, NULL);
	theApp.visionLSys.hThread = CreateThread(NULL, 0, IPCvisionLocationSystemThreadFun, NULL, 0, &theApp.visionLSys.ThreadID);
	for (size_t i = 0; i < theApp.visionLSys.IPC.size(); i++)
	{
		CString IPCname;
		IPCname.Format(_T("IPC:%d"), i);
		m_IPClist.AddString(IPCname);
	}
	theApp.IPCshowImgMutex = CreateMutex(NULL, FALSE, NULL);
	//
	printd("初始化成功");
	printd("开始运作");

	//开启爱米家机器人任务执行线程
	theApp.haimiTaskMutex = CreateMutex(NULL, FALSE, NULL);
	theApp.haimiTaskrunThread = CreateThread(NULL, 0, aimiTaskrun_ThreadFun, NULL, 0, &theApp.aimiTaskrunThreadID);


	//开启爱米家机器人线程
	theApp.robotServer.aimirobot.hMutex= CreateMutex(NULL, FALSE, NULL);
	theApp.robotServer.aimirobot.hThread = CreateThread(NULL, 0, aimipuls_ThreadFun, NULL, 0, &theApp.robotServer.aimirobot.hThreadID);

	//开启小机器人任务执行线程
	theApp.hTaskMutex= CreateMutex(NULL, FALSE, NULL);
	for (int threadi = 0; threadi < 4; threadi++)
	{
		vector<CMultiRobotApp::task> newtaskq;
		theApp.taskqueue.push_back(newtaskq);
		theApp.hTaskrunThread[threadi]= CreateThread(NULL, 0, Taskrun_ThreadFun, (LPVOID)threadi, 0, &theApp.TaskrunThreadID[threadi]);
	}

	//开启中央调度线程
	theApp.MultiRobotControl_Mutex = CreateMutex(NULL, FALSE, NULL);
	theApp.hMultiRobotControlThread= CreateThread(NULL, 0, MultiRobotControl_ThreadFun, NULL,0,&theApp.MultiRobotControlThreadID);


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMultiRobotDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMultiRobotDlg::OnPaint()
{

	//------绘制logo
	//获取到DC
	CDC* pDC = GetDC();
	//你需要绘制的目标
	CRect clientRect;
	GetClientRect(&clientRect);
	//载入你的图片
	Gdiplus::Bitmap * lpBmp = Gdiplus::Bitmap::FromFile(_T("logo1.png"));;
	//绑定你 的DC
	Gdiplus::Graphics graph(pDC->m_hDC);
	//开始绘制操作
	graph.DrawImage(lpBmp,
		//目标位置
		Gdiplus::Rect(500, 280, lpBmp->GetWidth()/4, lpBmp->GetHeight()/4),
		//图片的剪切
		0, 0, lpBmp->GetWidth(), lpBmp->GetHeight(),
		Gdiplus::UnitPixel
	);
	graph.ReleaseHDC(pDC->m_hDC);
	delete lpBmp;

	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);

		
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMultiRobotDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}





/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
						多线程函数定义
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*---------------------socket监听线程-----------------------*/
DWORD WINAPI ListenAcceptThreadFun(LPVOID p)
{
	//用于多线程 监听线程
	vector<HANDLE> hUpdataRobotThread;
	vector<DWORD> UpdataRobotThreadID;
	//监听
	if (theApp.robotServer.Listen(ROBOT_MAXCONNECT_NUM) == SOCKET_ERROR)//监听
	{
		AfxMessageBox(_T("服务监听错误"));
		return -1;
	}
	
	while (theApp.ThreadOn)
	{
		int acptret = theApp.robotServer.Accept();

		//接收新用户
		if (acptret == INVALID_SOCKET)
		{
			//printf("accept error\n");
			continue; //继续等待下一次连接
		}
		else
		{
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
			HANDLE h1; DWORD id1;
			//uint8_t therobotid = theApp.robotServer.robotlist.back().robotID;
			h1 = CreateThread(NULL, 0, updataRobotStatusThreadFun, &theApp.robotServer.robotlist.back().robotID, 0, &id1);
			hUpdataRobotThread.push_back(h1);
			UpdataRobotThreadID.push_back(id1);
			theApp.robotServer.robotlist.back().setTorque(1);
			//解锁
			ReleaseMutex(theApp.robotServer.hMutex);
		}

	}

	

	return 0;


}

/*---------------------socket子线程-----------------------*/
//刷新机器人状态 如果掉线直接从robotServer.robotlist中pop掉，并且关闭线程
DWORD WINAPI updataRobotStatusThreadFun(LPVOID p)
{
	uint8_t robotid = *(uint8_t*)p;
	int robotindex = theApp.robotServer.findID(robotid);

	//判断网络连接 断开则跳出循环
	while (theApp.robotServer.robotlist[robotindex].connectStatus>0 && theApp.ThreadOn)
	{
		//updata 电压值
		WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
		robotindex = theApp.robotServer.findID(robotid);
		theApp.robotServer.robotlist[robotindex].Voltage = theApp.robotServer.robotlist[robotindex].getVoltage();
		//解锁
		ReleaseMutex(theApp.robotServer.hMutex);

		

		Sleep(1000);
	}
	
	//删除该id的进程 从robotServer.robotlist中pop掉,并且关闭socket
	WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
	robotindex = theApp.robotServer.findID(robotid);
	//关闭套接字。
	closesocket(theApp.robotServer.robotlist[robotindex].robotsock);
	vector<robot>::iterator it = theApp.robotServer.robotlist.begin() + robotindex;
	theApp.robotServer.robotlist.erase(it);
	//解锁
	ReleaseMutex(theApp.robotServer.hMutex);
	return 0;
}

/*--------------------IPC视觉处理定位线程-----------------------*/
DWORD WINAPI IPCvisionLocationSystemThreadFun(LPVOID p) 
{
	vector<IPCobj> lastobj;

	//读IPC，分别为每个IPC建立线程
	WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
	for (int i = 0; i < theApp.visionLSys.getIPCNum(); i++)
	{
		DWORD id1;
		HANDLE h1 = CreateThread(NULL, 0, IPCvisionLocationSonThreadFun, (LPVOID)i, 0, &id1);
		//为这个IPC创立互斥锁
		theApp.visionLSys.IPC[i].hMutexcap= CreateMutex(NULL, FALSE, NULL);
		Mat img=Mat(720,1280, CV_8UC3);
		theApp.IPCshowImg.push_back(img);
		vector<IPCobj> newereyobj;
		theApp.everyIPCobj.push_back(newereyobj);
	}
	ReleaseMutex(theApp.visionLSys.hMutex);//解锁
	//建立显示线程
	DWORD Threadshowid;
	HANDLE Threadshowh = CreateThread(NULL, 0, IPCvisionLocationSon_ShowThreadFun, NULL, 0, &Threadshowid);


	////测试用，要删掉
	//AllocConsole();
	//freopen("CONOUT$", "w", stdout);

	//定义用于运动学姿态估计的变量
	int whilect=0;		//循环计数
	vector<IPCobj> lastobj_estim;//用于姿态估计的，比lastobj早10倍
	int move_flag = 1;
	float dir_now = 0;   // 0-2pi
	float dir_st = 0;
	//用于计时
	DWORD starttime = GetTickCount();

	while (theApp.ThreadOn)
	{
		vector<vector<IPCobj>> inputobj;
		// 识别处理每一个图像中的obj
		//查看每一个IPC的图片
		for (size_t i = 0; i < theApp.IPCshowImg.size(); i++)
		{
			WaitForSingleObject(theApp.IPCshowImgMutex, INFINITE);//锁挂
			Mat img = theApp.IPCshowImg[i];
			ReleaseMutex(theApp.IPCshowImgMutex);//解锁
			vector<IPCobj> objection;
			if (theApp.visionLSys.Algorithm == 1)//判断用哪个算法
			{
				objection = theApp.visionLSys.locationMat(img, i);
			}
			else if(theApp.visionLSys.Algorithm == 0)
			{
				objection = theApp.visionLSys.location(img, i);
			}

			inputobj.push_back(objection);
		}

		//（1）整合obj
		WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
		theApp.everyIPCobj= inputobj;
		ReleaseMutex(theApp.visionLSys.hMutex);//解锁

		vector<IPCobj> casheobj = theApp.visionLSys.calculateAllObjection(inputobj);
		//（2）如果是运动学估计算法，来估计姿态位置.
		//刷新方向
		whilect++;
		if (theApp.visionLSys.estimation_Algorithm == 1 && whilect>2)
		{
			whilect = 0;
			WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
			for (size_t i = 0; i < theApp.robotServer.getRobotListNum(); i++)
			{
				Point2f ds;
				float dsl;
				int objindex_last = theApp.visionLSys.findVecterElm(lastobj_estim, theApp.robotServer.robotlist[i].robotID);
				int objindex_new = theApp.visionLSys.findVecterElm(casheobj, theApp.robotServer.robotlist[i].robotID);
				if (objindex_last >= 0 && objindex_new >= 0)
				{
					ds.x = casheobj[objindex_new].coordinate3D[0] - lastobj_estim[objindex_last].coordinate3D[0];
					ds.y = casheobj[objindex_new].coordinate3D[1] - lastobj_estim[objindex_last].coordinate3D[1];
					dsl = sqrt(ds.x*ds.x + ds.y*ds.y);
					//开始运动学姿态估计算法
					float robotv=theApp.robotServer.robotlist[i].pvw.v[CasheQueue_MAXSIZE-1];
					if (move_flag==0 && robotv>=0.1)
					{
						move_flag = 1;
						//theApp.robotServer.robotlist[i].initIMU();
						//dir_st = dir_now;
					}
					else if(move_flag==1 && robotv <0.1)
					{
						move_flag = 0;
						theApp.robotServer.robotlist[i].initIMU();
						dir_st = dir_now;
					}

					if (move_flag == 0)
					{
						//刷新IMU
						imu_msg imudata;
						imudata = theApp.robotServer.robotlist[i].getIMU();
						//四元组转RPY
						Eigen::Vector3d rpy;
						rpy = zfun::Quaterniond2Euler(imudata.orientation_x, imudata.orientation_y, imudata.orientation_z, imudata.orientation_w);
						float roll = zfun::numFormat(rpy[0] * 180 / 3.14159, 1);

						float ddirection = acos(imudata.orientation_w) * 2 * 180 / 3.14159;

						if (ddirection + roll > 179 && ddirection + roll < 181)
							dir_now = dir_st - ddirection;
						else if((int)ddirection==(int)roll)
						{
							dir_now = dir_st + ddirection;
						}
					}
					else if(move_flag==1)
					{
						if(dsl>0.01)
							dir_now = atan2(ds.y, ds.x)*180/3.14159;
					}
					

				}

				
			}

			ReleaseMutex(theApp.robotServer.hMutex);//解锁
			ReleaseMutex(theApp.visionLSys.hMutex);//解锁
			lastobj_estim = casheobj;
		}
		//继承方向
		if (theApp.visionLSys.estimation_Algorithm == 1)
		{
			for (size_t i = 0; i < theApp.robotServer.getRobotListNum(); i++)
			{
				int objindex_last = theApp.visionLSys.findVecterElm(lastobj_estim, theApp.robotServer.robotlist[i].robotID);
				int objindex_new = theApp.visionLSys.findVecterElm(casheobj, theApp.robotServer.robotlist[i].robotID);
				if (objindex_last >= 0 && objindex_new >= 0)
				{
					//给这个id的机器人obj角度信息
					casheobj[objindex_new].direction3D[0] = cos(dir_now*3.14159 / 180.0);
					casheobj[objindex_new].direction3D[1] = sin(dir_now*3.14159 / 180.0);
				}
			}
		}

		//（2）运动补偿
		if (theApp.movecompFlag == true)
		{
			WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
			for (size_t i = 0; i < theApp.robotServer.getRobotListNum(); i++)
			{
				Point2f dxy;
				//计算上一帧obj的角度
				int objindex = theApp.visionLSys.findVecterElm(lastobj, theApp.robotServer.robotlist[i].robotID);
				float lasttheta;
				if (objindex >= 0)
				{
					if (lastobj[objindex].direction3D[0] > 0)
						lasttheta = atan(lastobj[objindex].direction3D[1] / lastobj[objindex].direction3D[0]);
					if (lastobj[objindex].direction3D[0] < 0)
					{
						lasttheta = atan(lastobj[objindex].direction3D[1] / lastobj[objindex].direction3D[0]) + 3.14159;
					}
					dxy = theApp.robotServer.robotlist[i].pvw.displace(theApp.visionLSys.delayTime, lasttheta);
				}



				objindex = theApp.visionLSys.findVecterElm(casheobj, theApp.robotServer.robotlist[i].robotID);
				if (objindex >= 0)
				{
					casheobj[objindex].coordinate3D[0] += dxy.x;
					casheobj[objindex].coordinate3D[1] += dxy.y;
				}
			}
			ReleaseMutex(theApp.robotServer.hMutex);//解锁
			ReleaseMutex(theApp.visionLSys.hMutex);//解锁
		}

		//（3）刷新obj
		WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
		theApp.obj = casheobj;
		if (theApp.track_data_flag == true && casheobj.size()>=1)
		{
			Vec4f data; 
			data[0] = casheobj[0].coordinate3D[0];
			data[1] = casheobj[0].coordinate3D[1];
			data[2] = casheobj[0].coordinate3D[2];
			data[3] = (float)(GetTickCount() - starttime)/1000.0;
			theApp.track_data.push_back(data);
		}
		
		ReleaseMutex(theApp.visionLSys.hMutex);//解锁
		lastobj = casheobj;
		
	}

	return 0;
}

/*--------------------IPC视觉处理显示监控、地图线程-----------------------*/
DWORD WINAPI IPCvisionLocationSon_ShowThreadFun(LPVOID p)
{
	//int looklp[3] = { 2500 ,2500,800 };//x,y,scale

	struct tomouseCall
	{
		int x;//观察框中心点相对于map图片的像素坐标
		int y;
		int w;
		float x_mouse_t;//鼠标所指点的真实坐标
		float y_mouse_t;
		float x_mouse;//鼠标观察框的像素坐标
		float y_mouse;
		Mat img;
	}pra_mouseCall;
	pra_mouseCall.x = 2500; pra_mouseCall.y = 2500; pra_mouseCall.w = 2000;
	float map_size = 800;

	while (theApp.ThreadOn)
	{
		//（4）显示监控
		
		if (theApp.seleteimshow == -1 && theApp.IPCshowImg.size()>0)
		{
			for (size_t j = 0; j < theApp.visionLSys.getIPCNum(); j++)
			{
				string istr = std::to_string(j);
				if (j < theApp.IPCshowImg.size())
				{
					//画一个世界坐标系
					WaitForSingleObject(theApp.IPCshowImgMutex, INFINITE);//锁挂
					Mat showimg= theApp.IPCshowImg[j].clone();
					ReleaseMutex(theApp.IPCshowImgMutex);//解锁
					
					Vec3d rvecs, tvecs;
					Rodrigues(theApp.visionLSys.IPC[j].RwMatrix, rvecs);
					tvecs = theApp.visionLSys.IPC[j].TwVec;
					cv::aruco::drawAxis(showimg, theApp.visionLSys.IPC[j].cameraMatrix, theApp.visionLSys.IPC[j].distCoeffs, rvecs, tvecs, 0.5);

					resize(showimg, showimg, theApp.showsize);
					cv::imshow("outimg" + istr, showimg);
				}
			}
		}
		else if(theApp.seleteimshow >=0 && theApp.IPCshowImg.size()>theApp.seleteimshow)
		{
			int j = theApp.seleteimshow;
			//画一个世界坐标系
			WaitForSingleObject(theApp.IPCshowImgMutex, INFINITE);//锁挂
			Mat showimg = theApp.IPCshowImg[j].clone();
			ReleaseMutex(theApp.IPCshowImgMutex);//解锁

			Vec3d rvecs, tvecs;
			Rodrigues(theApp.visionLSys.IPC[j].RwMatrix, rvecs);
			tvecs = theApp.visionLSys.IPC[j].TwVec;
			cv::aruco::drawAxis(showimg, theApp.visionLSys.IPC[j].cameraMatrix, theApp.visionLSys.IPC[j].distCoeffs, rvecs, tvecs, 0.5);
			resize(showimg, showimg, theApp.showsize);
			cv::imshow("outimg", showimg);
		}
		else
		{
			destroyWindow("outimg");
		}
	


		//（5）显示地图obj
		if (theApp.show2Dflag == true)
		{
			Mat showobj;
			
			showobj = theApp.visionLSys.paintObject(theApp.obj, Point2i(pra_mouseCall.x, pra_mouseCall.y), pra_mouseCall.w);
			//显示坐标
			string zbstr;
			string strx = to_string(pra_mouseCall.x_mouse_t); strx = strx.substr(0, strx.size() - 3);
			string stry = to_string(pra_mouseCall.y_mouse_t); stry = stry.substr(0, stry.size() - 3);
			zbstr = "(" + strx + "," + stry + ")";
			putText(showobj, zbstr, Point(pra_mouseCall.x_mouse, pra_mouseCall.y_mouse-4), FONT_HERSHEY_COMPLEX, 0.4, Scalar(0, 0, 0), 1, 8);
			//显示爱米家机器人走点任务的目标点，并且连接目标点和机器人
			if (theApp.aimiTaskQueue.size() > 0 && theApp.aimiTaskQueue[0].taskname == 1)
			{
				Point2f tureptask, robotp;
				WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);
				tureptask.x = theApp.aimiTaskQueue[0].x; tureptask.y = theApp.aimiTaskQueue[0].y;
				ReleaseMutex(theApp.robotServer.aimirobot.hMutex);//解锁

				int aimiindex = theApp.visionLSys.findVecterElm(theApp.obj, theApp.robotServer.aimirobot.robotInfo.RobotID.id);
				if (aimiindex >= 0)
				{
					robotp.x = theApp.obj[aimiindex].coordinate3D[0]; robotp.y = theApp.obj[aimiindex].coordinate3D[1];
					Point taskimgp, robotimgp;

					//将真实坐标转换成屏幕坐标
					taskimgp.x = (tureptask.x*(float)theApp.visionLSys.m2pix - pra_mouseCall.x + theApp.visionLSys.getMapSize() / 2 + pra_mouseCall.w / 2) / ((float)pra_mouseCall.w / map_size);
					taskimgp.y = (-tureptask.y*(float)theApp.visionLSys.m2pix - pra_mouseCall.y + theApp.visionLSys.getMapSize() / 2 + pra_mouseCall.w / 2) / ((float)pra_mouseCall.w / map_size);
					robotimgp.x = (robotp.x*(float)theApp.visionLSys.m2pix - pra_mouseCall.x + theApp.visionLSys.getMapSize() / 2 + pra_mouseCall.w / 2) / ((float)pra_mouseCall.w / map_size);
					robotimgp.y = (-robotp.y*(float)theApp.visionLSys.m2pix - pra_mouseCall.y + theApp.visionLSys.getMapSize() / 2 + pra_mouseCall.w / 2) / ((float)pra_mouseCall.w / map_size);

					//判断是否在屏幕内
					if (taskimgp.x > 0 && taskimgp.x < showobj.cols && taskimgp.y>0 && taskimgp.y < showobj.rows &&
						robotimgp.x>0 && robotimgp.x < showobj.cols && robotimgp.y>0 && robotimgp.y < showobj.rows)
					{
						line(showobj, taskimgp, robotimgp, Scalar(125, 255, 180), 2);
						circle(showobj, taskimgp, 5, Scalar(125, 0, 180), -1);
					}
				}

			}
			//显示小机器人走点任务的目标点，并且连接目标点和机器人
			for (size_t tski = 0; tski < theApp.taskqueue.size(); tski++)
			{
				if (theApp.taskqueue[tski].size() > 0 && theApp.taskqueue[tski][0].taskname == 1)
				{
					Point2f tureptask, robotp;
					WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
					tureptask.x = theApp.taskqueue[tski][0].x; tureptask.y = theApp.taskqueue[tski][0].y;
					ReleaseMutex(theApp.robotServer.hMutex);//解锁

					int index = theApp.visionLSys.findVecterElm(theApp.obj, theApp.robotServer.robotlist[tski].robotID);
					if (index >= 0)
					{
						robotp.x = theApp.obj[index].coordinate3D[0]; robotp.y = theApp.obj[index].coordinate3D[1];
						Point taskimgp, robotimgp;

						//将真实坐标转换成屏幕坐标
						taskimgp.x = (tureptask.x*(float)theApp.visionLSys.m2pix - pra_mouseCall.x + theApp.visionLSys.getMapSize() / 2 + pra_mouseCall.w / 2) / ((float)pra_mouseCall.w / map_size);
						taskimgp.y = (-tureptask.y*(float)theApp.visionLSys.m2pix - pra_mouseCall.y + theApp.visionLSys.getMapSize() / 2 + pra_mouseCall.w / 2) / ((float)pra_mouseCall.w / map_size);
						robotimgp.x = (robotp.x*(float)theApp.visionLSys.m2pix - pra_mouseCall.x + theApp.visionLSys.getMapSize() / 2 + pra_mouseCall.w / 2) / ((float)pra_mouseCall.w / map_size);
						robotimgp.y = (-robotp.y*(float)theApp.visionLSys.m2pix - pra_mouseCall.y + theApp.visionLSys.getMapSize() / 2 + pra_mouseCall.w / 2) / ((float)pra_mouseCall.w / map_size);

						//判断是否在屏幕内
						if (taskimgp.x > 0 && taskimgp.x < showobj.cols && taskimgp.y>0 && taskimgp.y < showobj.rows &&
							robotimgp.x>0 && robotimgp.x < showobj.cols && robotimgp.y>0 && robotimgp.y < showobj.rows)
						{
							line(showobj, taskimgp, robotimgp, Scalar(125, 255, 180), 2);
							circle(showobj, taskimgp, 5, Scalar(255, 0, 180), -1);
						}
					}

				}
			}


			//显示地图
			cv::imshow("showobj", showobj);
			pra_mouseCall.img = showobj;
			cv::setMouseCallback("showobj", map_mouse_callback, &pra_mouseCall);

		}
		else
		{
			destroyWindow("showobj");
		}
	
		int key = waitKey(10);

	}
	return 0;
}
void map_mouse_callback(int event, int x, int y, int flags, void* param)
{

	static Point pre_pt = (-1, -1);//初始坐标
	static Point cur_pt = (-1, -1);//实时坐标  
	static Point pre_looklp = (-1, -1);

	float map_size = 800;

	struct tomouseCall
	{
		int x;
		int y;
		int w;
		float x_mouse_t;
		float y_mouse_t;
		float x_mouse;
		float y_mouse;
		Mat img;
	};
	tomouseCall* pra_mouseCall = (tomouseCall*) param;

	if (event == CV_EVENT_LBUTTONDOWN)//左键按下，读取初始坐标.
	{
		pre_pt = Point(x, y);
		pre_looklp.x = pra_mouseCall->x;
		pre_looklp.y = pra_mouseCall->y;
		
	}
	else if (event == CV_EVENT_MOUSEMOVE && !(flags & CV_EVENT_FLAG_LBUTTON))//左键没有按下的情况下鼠标移动的处理函数，实时显示坐标  
	{
		float k = pra_mouseCall->w / map_size;
		//计算真实坐标
		Point2f turep;
		turep.x = pra_mouseCall->x - theApp.visionLSys.getMapSize() / 2 - pra_mouseCall->w / 2 + x*k;
		turep.y = - pra_mouseCall->y + theApp.visionLSys.getMapSize() / 2 + pra_mouseCall->w / 2 - y*k;

		pra_mouseCall->x_mouse_t = turep.x / (float)theApp.visionLSys.m2pix;
		pra_mouseCall->y_mouse_t = turep.y / (float)theApp.visionLSys.m2pix;
		pra_mouseCall->y_mouse = y;
		pra_mouseCall->x_mouse = x;

		
	}
	else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))//左键按下时，鼠标移动，改变looklp
	{
		int dx = x - pre_pt.x;
		int dy = y - pre_pt.y;
		float k = pra_mouseCall->w / map_size;
		//需要限幅

		pra_mouseCall->x = pre_looklp.x - (int)(dx*k);
		pra_mouseCall->y = pre_looklp.y - (int)(dy*k);

		if (pra_mouseCall->x >= theApp.visionLSys.getMapSize() - pra_mouseCall->w / 2)
		{
			pra_mouseCall->x = theApp.visionLSys.getMapSize() - (pra_mouseCall->w / 2) - 1;
		}
		if (pra_mouseCall->x <= pra_mouseCall->w / 2)
		{
			pra_mouseCall->x = (pra_mouseCall->w / 2) + 1;
		}
		if (pra_mouseCall->y >= theApp.visionLSys.getMapSize() - pra_mouseCall->w / 2)
		{
			pra_mouseCall->y = theApp.visionLSys.getMapSize() - (pra_mouseCall->w / 2) - 1;
		}
		if (pra_mouseCall->y <= pra_mouseCall->w / 2)
		{
			pra_mouseCall->y = (pra_mouseCall->w / 2) + 1;
		}

	}
	else if (event == CV_EVENT_LBUTTONUP)//左键松开，取消拖动。
	{
	}
	else if (event == CV_EVENT_MOUSEWHEEL)//鼠标滑轮
	{
		double value = getMouseWheelDelta(flags);
		
		pra_mouseCall->w -= value;

		if (pra_mouseCall->w >= theApp.visionLSys.getMapSize())
		{
			pra_mouseCall->w = theApp.visionLSys.getMapSize() - 1;
		}
		if (pra_mouseCall->w <= 50)
		{
			pra_mouseCall->w = 51;
		}

		if (pra_mouseCall->x >= theApp.visionLSys.getMapSize() - pra_mouseCall->w / 2)
		{
			pra_mouseCall->x = theApp.visionLSys.getMapSize() - (pra_mouseCall->w / 2) - 1;
		}
		if (pra_mouseCall->x <= pra_mouseCall->w / 2)
		{
			pra_mouseCall->x = (pra_mouseCall->w / 2) + 1;
		}
		if (pra_mouseCall->y >= theApp.visionLSys.getMapSize() - pra_mouseCall->w / 2)
		{
			pra_mouseCall->y = theApp.visionLSys.getMapSize() - (pra_mouseCall->w / 2) - 1;
		}
		if (pra_mouseCall->y <= pra_mouseCall->w / 2)
		{
			pra_mouseCall->y = (pra_mouseCall->w / 2) + 1;
		}

	}
	else if(event== CV_EVENT_MBUTTONDOWN)//按下鼠标中间，对爱米家机器人发布走点任务。
	{
		float k = pra_mouseCall->w / map_size;
		//计算真实坐标
		Point2f turep;
		turep.x = pra_mouseCall->x - theApp.visionLSys.getMapSize() / 2 - pra_mouseCall->w / 2 + x*k;
		turep.y = -pra_mouseCall->y + theApp.visionLSys.getMapSize() / 2 + pra_mouseCall->w / 2 - y*k;
		turep.x = turep.x / (float)theApp.visionLSys.m2pix;
		turep.y = turep.y / (float)theApp.visionLSys.m2pix;

		//发布任务
		WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);
		CMultiRobotApp::task foo;
		foo.taskname = 1; foo.x = turep.x; foo.y = turep.y;
		theApp.aimiTaskQueue.push_back(foo);	
		ReleaseMutex(theApp.robotServer.aimirobot.hMutex);//解锁
		
	}
	else if(event == CV_EVENT_RBUTTONDOWN) //按下鼠标右键，对列表中选中的机器人发布走点任务。
	{
		float k = pra_mouseCall->w / map_size;
		//计算真实坐标
		Point2f turep;
		turep.x = pra_mouseCall->x - theApp.visionLSys.getMapSize() / 2 - pra_mouseCall->w / 2 + x*k;
		turep.y = -pra_mouseCall->y + theApp.visionLSys.getMapSize() / 2 + pra_mouseCall->w / 2 - y*k;
		turep.x = turep.x / (float)theApp.visionLSys.m2pix;
		turep.y = turep.y / (float)theApp.visionLSys.m2pix;

		WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
		int rindex = theApp.robotlist_checkIndex;
		ReleaseMutex(theApp.robotServer.hMutex);

		if (rindex >= 0)
		{
			CMultiRobotApp::task foo;
			foo.taskname = 1; foo.x = turep.x; foo.y = turep.y;
			theApp.taskqueue[rindex].push_back(foo);
		}

	}

}
/*--------------------IPC子处理线程-----------------------*/
DWORD WINAPI IPCvisionLocationSonThreadFun(LPVOID p)
{

	int index = (int)p;
	if (theApp.visionLSys.IPC[index].Open() == false)
	{
		CString err;
		err.Format(_T("第%d号IP摄像头无法打开"), index);
		AfxMessageBox(err);
		return 0;
	}
	while (theApp.ThreadOn)
	{
		//取图片
		Mat img, outimg;
		//取图片是记得给这个cap流上锁。
		WaitForSingleObject(theApp.visionLSys.IPC[index].hMutexcap, INFINITE);//锁挂
		theApp.visionLSys.IPC[index].cap >> img;
		ReleaseMutex(theApp.visionLSys.IPC[index].hMutexcap);//解锁

		//resize(img, img, Size(1280, 720));
		//location
		//vector<IPCobj> objection;
		//objection = theApp.visionLSys.location(img, 0, outimg);

		WaitForSingleObject(theApp.IPCshowImgMutex, INFINITE);//锁挂
		//刷新监视图
		theApp.IPCshowImg[index] = img;
		//刷新obj
		//theApp.everyIPCobj[index] = objection;

		ReleaseMutex(theApp.IPCshowImgMutex);//解锁


		
		//Sleep(1);

	}
	return 0;
}

/*--------------------单独为aimi机器人开辟线程-----------------------*/
DWORD WINAPI aimipuls_ThreadFun(LPVOID p)
{
	string aimiipstr;
	theApp.config["AimibotIP"] >> aimiipstr;
	const char *ip = aimiipstr.c_str();
	int aimiport;
	theApp.config["AimibotIPport"] >> aimiport;

	theApp.robotServer.aimirobot.init(ip, aimiport);

	while (theApp.ThreadOn)
	{
		theApp.robotServer.aimirobot.Connect();

		while (theApp.robotServer.aimirobot.connectStatus>0)
		{
			WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);
			theApp.robotServer.aimirobot.updateInfo();
			theApp.robotServer.aimirobot.move();
			ReleaseMutex(theApp.robotServer.aimirobot.hMutex);//解锁

			Sleep(100);
		}
	}
	return 0;
	
}


//爱米家机器人的任务线程
DWORD WINAPI aimiTaskrun_ThreadFun(LPVOID p)
{
	int index = 1;
	Point2f lp; float theta;
	Point2f lpo;

	////测试用，要删掉
	/*AllocConsole();
	freopen("CONOUT$", "w", stdout);*/
	while (theApp.ThreadOn)
	{
		WaitForSingleObject(theApp.haimiTaskMutex, INFINITE);//锁挂
		WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);
		bool task_stat = theApp.robotServer.aimirobot.connectStatus > 0 && theApp.aimiTaskQueue.size() > 0;
		ReleaseMutex(theApp.robotServer.aimirobot.hMutex);//解锁
		ReleaseMutex(theApp.haimiTaskMutex);//解锁
		if (task_stat)
		{
			//读取任务队列并解析
			if (theApp.aimiTaskQueue[0].taskname == 1)
			{
				int st_flag=1;//记录机器人在执行次任务过程中的状态，1为找方向原地转圈，2为直走加偏航 0为完成任务
				float dtheta;
				Point2f ds;
				float d;
				lpo.x = theApp.aimiTaskQueue[0].x; lpo.y = theApp.aimiTaskQueue[0].y;


				while (st_flag)
				{
					//获取信息
					WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
					WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);
					int objindex = theApp.visionLSys.findVecterElm(theApp.obj, theApp.robotServer.aimirobot.robotInfo.RobotID.id);
					if (objindex < 0) break;
					lp.x = theApp.obj[objindex].coordinate3D[0]; lp.y = theApp.obj[objindex].coordinate3D[1];
					theta = atan2(theApp.obj[objindex].direction3D[1], theApp.obj[objindex].direction3D[0]) * 180 / 3.14159;
					dtheta = theta - atan2(lpo.y - lp.y, lpo.x - lp.x) * 180 / 3.14159;
					if (dtheta < -180) dtheta = 360 + dtheta;
					else if (dtheta > 180) dtheta = dtheta - 360;
					ds = lpo - lp;
					d = sqrt(ds.x*ds.x + ds.y*ds.y);
					ReleaseMutex(theApp.robotServer.aimirobot.hMutex);//解锁
					ReleaseMutex(theApp.visionLSys.hMutex);//解锁

					cout << "dtheta" << dtheta << endl;
					cout << "d:" << d << endl;

					//控制1
					if (abs(dtheta) > 2 && st_flag==1)
					{
						
						WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);
						
						float pt = 0.013;//转动pid系数
						float phph = 30; //度数阈值，低于这个度数一一个恒定值控制
						if (abs(dtheta) > phph)
						{
							theApp.robotServer.aimirobot.v = 0; theApp.robotServer.aimirobot.w = dtheta*pt*100;
						}
						else
						{
							theApp.robotServer.aimirobot.v = 0; theApp.robotServer.aimirobot.w = (dtheta / abs(dtheta)) * phph * pt*100;
						}
						ReleaseMutex(theApp.robotServer.aimirobot.hMutex);//解锁
						
					}
					else if(abs(dtheta) < 2 && st_flag == 1)
					{
						st_flag = 2;
					}
					

					//控制2
					if (d > 0.05 && st_flag == 2)
					{
						
						WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);

						float pv = 0.2, ptt = 0.01; //pid系数
						float phph2 = 0.3;//距离阈值，低于这个距离就采用恒定值控制
						if (d > phph2)
						{
							theApp.robotServer.aimirobot.v = d*pv; theApp.robotServer.aimirobot.w = dtheta*ptt;
						}
						else
						{
							theApp.robotServer.aimirobot.v = phph2*pv; theApp.robotServer.aimirobot.w = dtheta*ptt;
						}
						ReleaseMutex(theApp.robotServer.aimirobot.hMutex);//解锁
						
					}
					else if (abs(dtheta) > 5 && st_flag == 2 && d>0.2)
					{
						st_flag = 1;
					}
					

					//判断距离结束任务
					if (d < 0.05)
					{
						st_flag = 0;
					}

					Sleep(50);
				}
				
			}
			WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);
			theApp.robotServer.aimirobot.v = 0; theApp.robotServer.aimirobot.w = 0;
			ReleaseMutex(theApp.robotServer.aimirobot.hMutex);//解锁
			//删除任务队列
			std::vector<CMultiRobotApp::task>::iterator iter = theApp.aimiTaskQueue.begin();
			theApp.aimiTaskQueue.erase(iter);
		}
		else
		{
			Sleep(100);
		}

	}
	return 0;

}

//小机器人的任务线程
DWORD WINAPI Taskrun_ThreadFun(LPVOID p)
{
	int index = (int)p;
	Point2f lp; float theta;
	Point2f lpo;
	int zanting=0;

	////测试用，要删掉
	//AllocConsole();
	//freopen("CONOUT$", "w", stdout);
	while (theApp.ThreadOn)
	{
		WaitForSingleObject(theApp.hTaskMutex, INFINITE);//锁挂
		WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
		bool task_stat = theApp.robotServer.getRobotListNum() > index && theApp.taskqueue[index].size() > 0;
		ReleaseMutex(theApp.robotServer.hMutex);//解锁
		ReleaseMutex(theApp.hTaskMutex);//解锁
		if (task_stat)
		{

			//读取任务队列并解析
			if (theApp.taskqueue[index][0].taskname == 1)
			{
				int st_flag = 1;//记录机器人在执行次任务过程中的状态，1为找方向原地转圈，2为直走加偏航 0为完成任务
				float dtheta;
				Point2f ds;
				float d;

				lpo.x = theApp.taskqueue[index][0].x; lpo.y = theApp.taskqueue[index][0].y;

				while (st_flag)
				{
					//获取信息
					WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
					WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
					int objindex = theApp.visionLSys.findVecterElm(theApp.obj, theApp.robotServer.robotlist[index].robotID);
					if (objindex < 0)
					{
						ReleaseMutex(theApp.robotServer.hMutex);//解锁
						ReleaseMutex(theApp.visionLSys.hMutex);//解锁
						printf("error：Taskrun_ThreadFun：检测不到图片，停止任务");
						zanting = 1;
						break;
					}
					else
					{
						zanting = 0;
					}
					lp.x = theApp.obj[objindex].coordinate3D[0]; lp.y = theApp.obj[objindex].coordinate3D[1];
					theta = atan2(theApp.obj[objindex].direction3D[1], theApp.obj[objindex].direction3D[0]) * 180 / 3.14159;
					dtheta = theta - atan2(lpo.y - lp.y, lpo.x - lp.x) * 180 / 3.14159;
					if (dtheta < -180) dtheta = 360 + dtheta;
					else if (dtheta > 180) dtheta = dtheta - 360;
					ds = lpo - lp;
					d = sqrt(ds.x*ds.x + ds.y*ds.y);
					ReleaseMutex(theApp.robotServer.hMutex);//解锁
					ReleaseMutex(theApp.visionLSys.hMutex);//解锁

					//cout << "dtheta" << dtheta << endl;
					//cout << "d:" << d << endl;

					//控制1
					if (abs(dtheta) > 2 && st_flag == 1)
					{

						WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);

						float pt = 0.006;//转动pid系数
						float phph = 30; //度数阈值，低于这个度数一一个恒定值控制
						if (abs(dtheta) > phph)
						{
							theApp.robotServer.robotlist[index].move(0, -dtheta*pt);
						}
						else
						{
							theApp.robotServer.robotlist[index].move(0, -(dtheta / abs(dtheta)) * phph * pt);
						}
						ReleaseMutex(theApp.robotServer.hMutex);//解锁

					}
					else if (abs(dtheta) < 2 && st_flag == 1)
					{
						st_flag = 2;
					}


					//控制2
					if (d > 0.05 && st_flag == 2)
					{

						WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);

						float pv = 0.2, ptt = 0.01; //pid系数
						float phph2 = 0.3;//距离阈值，低于这个距离就采用恒定值控制
						if (d > phph2)
						{
							theApp.robotServer.robotlist[index].move(d*pv, -dtheta*ptt);
							//theApp.robotServer.aimirobot.v = d*pv; theApp.robotServer.aimirobot.w = dtheta*ptt;
						}
						else
						{
							theApp.robotServer.robotlist[index].move(phph2*pv, -dtheta*ptt);
							//theApp.robotServer.aimirobot.v = phph2*pv; theApp.robotServer.aimirobot.w = dtheta*ptt;
						}
						ReleaseMutex(theApp.robotServer.hMutex);//解锁

					}
					else if (abs(dtheta) > 5 && st_flag == 2 && d > 0.2)
					{
						st_flag = 1;
					}

					//cout << "dtheta" << dtheta << endl;
					//cout << "d:" << d << endl;
					//判断距离结束任务
					if (d < 0.05)
					{
						st_flag = 0;
					}

					Sleep(50);
				}

			}
			else if (theApp.taskqueue[index][0].taskname == 2)
			{
				int st_flag = 1;//记录机器人在执行次任务过程中的状态，1为找方向原地转圈，2为直走加偏航 0为完成任务
				float dtheta;
				Point2f ds;
				float d;

				lpo.x = theApp.taskqueue[index][0].x; lpo.y = theApp.taskqueue[index][0].y;

				while (st_flag)
				{
					//获取信息
					WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
					WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
					int objindex = theApp.visionLSys.findVecterElm(theApp.obj, theApp.robotServer.robotlist[index].robotID);
					if (objindex < 0)
					{
						ReleaseMutex(theApp.robotServer.hMutex);//解锁
						ReleaseMutex(theApp.visionLSys.hMutex);//解锁
						printf("error：Taskrun_ThreadFun：检测不到图片，停止任务");
						zanting = 1;
						break;
					}
					else
					{
						zanting = 0;
					}
					lp.x = theApp.obj[objindex].coordinate3D[0]; lp.y = theApp.obj[objindex].coordinate3D[1];
					theta = atan2(-theApp.obj[objindex].direction3D[1], -theApp.obj[objindex].direction3D[0]) * 180 / 3.14159;
					dtheta = theta - atan2(lpo.y - lp.y, lpo.x - lp.x) * 180 / 3.14159;
					if (dtheta < -180) dtheta = 360 + dtheta;
					else if (dtheta > 180) dtheta = dtheta - 360;
					ds = lpo - lp;
					d = sqrt(ds.x*ds.x + ds.y*ds.y);
					ReleaseMutex(theApp.robotServer.hMutex);//解锁
					ReleaseMutex(theApp.visionLSys.hMutex);//解锁

					//cout << "dtheta" << dtheta << endl;
					//cout << "d:" << d << endl;

					//控制1
					if (abs(dtheta) > 2 && st_flag == 1)
					{

						WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);

						float pt = 0.006;//转动pid系数
						float phph = 30; //度数阈值，低于这个度数一一个恒定值控制
						if (abs(dtheta) > phph)
						{
							theApp.robotServer.robotlist[index].move(0, -dtheta*pt);
						}
						else
						{
							theApp.robotServer.robotlist[index].move(0, -(dtheta / abs(dtheta)) * phph * pt);
						}
						ReleaseMutex(theApp.robotServer.hMutex);//解锁

					}
					else if (abs(dtheta) < 2 && st_flag == 1)
					{
						st_flag = 2;
					}


					//控制2
					if (d > 0.05 && st_flag == 2)
					{

						WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);

						float pv = 0.2, ptt = 0.01; //pid系数
						float phph2 = 0.3;//距离阈值，低于这个距离就采用恒定值控制
						if (d > phph2)
						{
							theApp.robotServer.robotlist[index].move(-d*pv, -dtheta*ptt);
							//theApp.robotServer.aimirobot.v = d*pv; theApp.robotServer.aimirobot.w = dtheta*ptt;
						}
						else
						{
							theApp.robotServer.robotlist[index].move(-phph2*pv, -dtheta*ptt);
							//theApp.robotServer.aimirobot.v = phph2*pv; theApp.robotServer.aimirobot.w = dtheta*ptt;
						}
						ReleaseMutex(theApp.robotServer.hMutex);//解锁

					}
					else if (abs(dtheta) > 5 && st_flag == 2 && d > 0.2)
					{
						st_flag = 1;
					}


					//判断距离结束任务
					if (d < 0.05)
					{
						st_flag = 0;
					}

					Sleep(50);
				}
			}
			else if (theApp.taskqueue[index][0].taskname == 3)//摆正姿态
			{
				int st_flag = 1;//记录机器人在执行次任务过程中的状态，1为找方向原地转圈，2为直走加偏航 0为完成任务
				float dtheta;


				while (st_flag)
				{
					//获取信息
					WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
					WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
					int objindex = theApp.visionLSys.findVecterElm(theApp.obj, theApp.robotServer.robotlist[index].robotID);
					if (objindex < 0) 
					{
						ReleaseMutex(theApp.robotServer.hMutex);//解锁
						ReleaseMutex(theApp.visionLSys.hMutex);//解锁
						printf("error：Taskrun_ThreadFun：检测不到图片，停止任务");
						zanting = 1;
						break; 
					}
					else
					{
						zanting = 0;
					}
					lp.x = theApp.obj[objindex].coordinate3D[0]; lp.y = theApp.obj[objindex].coordinate3D[1];
					theta = atan2(theApp.obj[objindex].direction3D[1], theApp.obj[objindex].direction3D[0]) * 180 / 3.14159;
					dtheta = theta - atan2(theApp.taskqueue[index][0].y, theApp.taskqueue[index][0].x) * 180 / 3.14159;
					if (dtheta < -180) dtheta = 360 + dtheta;
					else if (dtheta > 180) dtheta = dtheta - 360;

					if (theApp.taskqueue[index][0].lf != 0)
					{
						if (dtheta < 0) dtheta = 360 + dtheta;
						if (theApp.taskqueue[index][0].lf == 1) dtheta = dtheta - 360;
					}

					ReleaseMutex(theApp.robotServer.hMutex);//解锁
					ReleaseMutex(theApp.visionLSys.hMutex);//解锁

					
					

					//cout << "dtheta" << dtheta << endl;

					//控制1
					if (abs(dtheta) > 5 && st_flag == 1)
					{

						WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);

						float pt = 0.006;//转动pid系数
						float phph = 30; //度数阈值，低于这个度数一一个恒定值控制
						if (abs(dtheta) > phph)
						{
							theApp.robotServer.robotlist[index].move(0, -dtheta*pt);
						}
						else
						{
							theApp.robotServer.robotlist[index].move(0, -(dtheta / abs(dtheta)) * phph * pt);
						}
						ReleaseMutex(theApp.robotServer.hMutex);//解锁

					}
					else if (abs(dtheta) < 5 && st_flag == 1)
					{
						st_flag = 0;
					}
					Sleep(50);

				}
			}
			else
			{
				Sleep(100);
			}
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.robotServer.robotlist[index].move(0, 0);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁
			if (zanting == 0)
			{
				//删除任务队列
				std::vector<CMultiRobotApp::task>::iterator iter = theApp.taskqueue[index].begin();
				theApp.taskqueue[index].erase(iter);
			}
		}
	}

	return 0;
}

//多机协作中央调度程序
DWORD WINAPI MultiRobotControl_ThreadFun(LPVOID p)
{

	while (theApp.ThreadOn)
	{
		WaitForSingleObject(theApp.MultiRobotControl_Mutex, INFINITE);//锁挂
		int bigtask = theApp.bigtask;
		ReleaseMutex(theApp.MultiRobotControl_Mutex);//解锁
		if (bigtask==1)
		{

			WaitForSingleObject(theApp.MultiRobotControl_Mutex, INFINITE);//锁挂
			theApp.bigtask = 0;
			ReleaseMutex(theApp.MultiRobotControl_Mutex);//解锁
			//写入机器人分拣搬运任务分配，
			CMultiRobotApp::task ntask;
			int robot81_index = theApp.robotServer.findID(81);
			int robot82_index = theApp.robotServer.findID(82);

			Point2f A = Point2f(0.578, 0.65);
			Point2f B = Point2f(0.027, 0.7);
			Point2f C = Point2f(-0.67, 0.66);
			Point2f D = Point2f(-0.67, 1.3);
			Point2f E = Point2f(1.9, 1.3);
			Point2f P81 = Point2f(1.9, 0.65);
			Point2f P82 = Point2f(1.9, 0.3);

			ntask.taskname = 1; ntask.x = A.x; ntask.y = A.y;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot81_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			ntask.taskname = 1; ntask.x = B.x; ntask.y = B.y;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot81_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			Sleep(20000);

			ntask.taskname = 1; ntask.x = A.x; ntask.y = A.y;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot82_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			
			//等待抓取信号
			int f_flag = 0;
			while (f_flag == 0)
			{
				WaitForSingleObject(theApp.MultiRobotControl_Mutex, INFINITE);//锁挂
				f_flag = theApp.finishGet_flag;
				ReleaseMutex(theApp.MultiRobotControl_Mutex);//解锁
			}
			WaitForSingleObject(theApp.MultiRobotControl_Mutex, INFINITE);//锁挂
			theApp.finishGet_flag=0;
			ReleaseMutex(theApp.MultiRobotControl_Mutex);//解锁
			

			ntask.taskname = 1; ntask.x = C.x; ntask.y = C.y;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot81_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			ntask.taskname = 1; ntask.x = D.x; ntask.y = D.y;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot81_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			ntask.taskname = 1; ntask.x = E.x; ntask.y = E.y;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot81_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			ntask.taskname = 1; ntask.x = P82.x; ntask.y = P82.y;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot81_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			ntask.taskname = 3; ntask.x = -1; ntask.y = 0; ntask.lf = 0;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot81_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			Sleep(4000);

			ntask.taskname = 1; ntask.x = B.x; ntask.y = B.y;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot82_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			//等待抓取信号
			f_flag = 0;
			while (f_flag == 0)
			{
				WaitForSingleObject(theApp.MultiRobotControl_Mutex, INFINITE);//锁挂
				f_flag = theApp.finishGet_flag;
				ReleaseMutex(theApp.MultiRobotControl_Mutex);//解锁
			}
			WaitForSingleObject(theApp.MultiRobotControl_Mutex, INFINITE);//锁挂
			theApp.finishGet_flag = 0;
			ReleaseMutex(theApp.MultiRobotControl_Mutex);//解锁

			ntask.taskname = 1; ntask.x = C.x; ntask.y = C.y;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot82_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			ntask.taskname = 1; ntask.x = D.x; ntask.y = D.y;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot82_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			ntask.taskname = 1; ntask.x = E.x; ntask.y = E.y;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot82_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			ntask.taskname = 1; ntask.x = P81.x; ntask.y = P81.y;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot82_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			ntask.taskname = 3; ntask.x = -1; ntask.y = 0; ntask.lf = 0;
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.taskqueue[robot82_index].push_back(ntask);
			ReleaseMutex(theApp.robotServer.hMutex);//解锁

			bigtask = 0;
		}

		Sleep(100);
	}
	return 0;
}



/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
								MFC函数
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void CMultiRobotDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 1://对话框刷新
	{
		//锁挂
		WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
		//刷新机器人列表
		updataRobotListDisplay();

		//刷新指定机器人的信息：电压 IMU 
		int index = m_RobotList.GetCurSel();
		theApp.robotlist_checkIndex = index;
		if (index >= 0)
		{
			//刷新电压
			m_voltage = theApp.robotServer.robotlist[index].Voltage;
			//刷新IMU
			imu_msg imudata;
			imudata = theApp.robotServer.robotlist[index].getIMU();
			m_angular_velocity_x = zfun::numFormat(imudata.angular_velocity_x, 2);
			m_angular_velocity_y = zfun::numFormat(imudata.angular_velocity_y, 2);
			m_angular_velocity_z = zfun::numFormat(imudata.angular_velocity_z, 2);
			m_linear_acceleration_x = zfun::numFormat(imudata.linear_acceleration_x, 2);
			m_linear_acceleration_y = zfun::numFormat(imudata.linear_acceleration_y, 2);
			m_linear_acceleration_z = zfun::numFormat(imudata.linear_acceleration_z, 2);
			//四元组转RPY
			Eigen::Vector3d rpy;
			rpy = zfun::Quaterniond2Euler(imudata.orientation_x, imudata.orientation_y, imudata.orientation_z, imudata.orientation_w);
			m_roll = zfun::numFormat(rpy[0] * 180 / 3.14159, 1);
			m_pitch = zfun::numFormat(rpy[1] * 180 / 3.14159, 1);
			m_yaw = zfun::numFormat(rpy[2] * 180 / 3.14159, 1);
			m_direction = acos(imudata.orientation_w) * 2 * 180 / 3.14159;

			//刷新运动控制使能控件
			if (theApp.robotServer.robotlist[index].getTorque() == 1)
			{
				m_moveEnable.SetCheck(true);
			}
			else if (theApp.robotServer.robotlist[index].getTorque() == 0)
			{
				m_moveEnable.SetCheck(false);
			}
		}

		//解锁
		ReleaseMutex(theApp.robotServer.hMutex);

		//查看爱米家机器人连接状态
		WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);
		if (theApp.robotServer.aimirobot.connectStatus > 0)
		{
			m_aimibotID = theApp.robotServer.aimirobot.robotInfo.RobotID.id;
		}
		else
		{
			m_aimibotID = -1;
		}
		ReleaseMutex(theApp.robotServer.aimirobot.hMutex);//解锁
		UpdateData(false);
	}
	case 2://定时刷新机器的线速度角速度
	{
		WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂

		for (size_t i = 0; i < theApp.robotServer.getRobotListNum(); i++)
		{
			theApp.robotServer.robotlist[i].pvw.vpush(theApp.robotServer.robotlist[i].v);
			theApp.robotServer.robotlist[i].pvw.wpush(theApp.robotServer.robotlist[i].w);

		}

		ReleaseMutex(theApp.robotServer.hMutex);//解锁

	}
	}



	CDialogEx::OnTimer(nIDEvent);
}


//Listbox 与 robotlist 进行对比更新
void CMultiRobotDlg::updataRobotListDisplay()
{
	//刷新robotlist列表
	int flagone=1; //1是一样 0是不一样
	//判断是否一致
	if (theApp.robotServer.getRobotListNum() == m_RobotList.GetCount())
	{
		for (size_t i = 0; i < m_RobotList.GetCount(); i++)
		{
			CString robotname;
			robotname.Format(_T("%d"), theApp.robotServer.robotlist[i].robotID);
			CString m_RobotListstr;
			m_RobotList.GetText(i, m_RobotListstr);
			if (robotname != m_RobotListstr) flagone = 0;
		}
	}
	else
	{
		flagone = 0;
	}
	

	if (flagone == 0)
	{
		m_RobotList.ResetContent();
		for (size_t i = 0; i < theApp.robotServer.getRobotListNum(); i++)
		{
			CString robotname;
			robotname.Format(_T("%d"), theApp.robotServer.robotlist[i].robotID);


			m_RobotList.AddString(robotname);
		}
		UpdateData(false);
	}
}





//对该机器人进行按键操作
BOOL CMultiRobotDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	
	if (pMsg->message == WM_KEYDOWN)
	{
		if (aimibotkeyflag == 0)
		{

			//检查WSAD时用什么参数来跑
			float ilin, iang;
			if (m_wsadFlag.GetCheck() == BST_CHECKED)
			{
				ilin = (float)m_movelin.GetPos() / 100.0;
				iang = (float)m_moveang.GetPos() / 100.0;
			}
			else
			{
				ilin = 0.2;
				iang = 1.8;
			}

			switch (pMsg->wParam)
			{
			case VK_UP://前进

				WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);//锁挂
				theApp.robotServer.aimirobot.v = ilin; theApp.robotServer.aimirobot.w = 0;
				//解锁
				ReleaseMutex(theApp.robotServer.aimirobot.hMutex);
				aimibotkeyflag = 1;
				break;
			case VK_DOWN://后退
				WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);//锁挂
				theApp.robotServer.aimirobot.v = -ilin; theApp.robotServer.aimirobot.w = 0;
				//解锁
				ReleaseMutex(theApp.robotServer.aimirobot.hMutex);
				aimibotkeyflag = 1;
				break;
			case VK_LEFT://左
				WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);//锁挂
				theApp.robotServer.aimirobot.v = ilin; theApp.robotServer.aimirobot.w = -ilin*1000;
				//解锁
				ReleaseMutex(theApp.robotServer.aimirobot.hMutex);
				aimibotkeyflag = 1;
				break;
			case VK_RIGHT://右
				WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);//锁挂
				theApp.robotServer.aimirobot.v = ilin; theApp.robotServer.aimirobot.w = ilin*1000;
				//解锁
				ReleaseMutex(theApp.robotServer.aimirobot.hMutex);
				aimibotkeyflag = 1;
				break;
			default:
				break;
			}

			
		}


		if (keyflag == 0)
		{
			////查看当前选择机器人。
			int index= m_RobotList.GetCurSel();
			if (index >= 0)
			{
				//return CDialogEx::PreTranslateMessage(pMsg);
				//检查WSAD时用什么参数来跑
				float ilin, iang;
				if (m_wsadFlag.GetCheck() == BST_CHECKED)
				{
					ilin = (float)m_movelin.GetPos() / 100.0;
					iang = (float)m_moveang.GetPos() / 100.0;
				}
				else
				{
					ilin = 0.2;
					iang = 1.8;
				}

				switch (pMsg->wParam)
				{
				case 'W'://前进

					WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
					theApp.robotServer.robotlist[index].move(ilin, 0);
					//解锁
					ReleaseMutex(theApp.robotServer.hMutex);
					keyflag = 1;
					break;
				case 'S'://后退
					WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
					theApp.robotServer.robotlist[index].move(-ilin, 0);
					//解锁
					ReleaseMutex(theApp.robotServer.hMutex);
					keyflag = 1;
					break;
				case 'A'://左
					WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
					theApp.robotServer.robotlist[index].move(0, iang);
					//解锁
					ReleaseMutex(theApp.robotServer.hMutex);
					keyflag = 1;
					break;
				case 'D'://右
					WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
					theApp.robotServer.robotlist[index].move(0, -iang);
					//解锁
					ReleaseMutex(theApp.robotServer.hMutex);
					keyflag = 1;
					break;
				default:
					break;
				}
				
			}
			
		}

		
		
		return true;
	}
	else if (pMsg->message == WM_KEYUP)
	{
		if (pMsg->wParam == 'W'|| pMsg->wParam == 'S'||pMsg->wParam == 'A'||pMsg->wParam == 'D')
		{
			////查看当前选择机器人。
			int index = m_RobotList.GetCurSel();
			if (index < 0)
			{
				return CDialogEx::PreTranslateMessage(pMsg);
			}
			//锁挂
			WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
			theApp.robotServer.robotlist[index].move(0, 0);
			//解锁
			ReleaseMutex(theApp.robotServer.hMutex);

			keyflag = 0;
		}
		else if(pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN ||pMsg->wParam == VK_RIGHT || pMsg->wParam == VK_LEFT)
		{
			WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);
			theApp.robotServer.aimirobot.v = 0; theApp.robotServer.aimirobot.w = 0;
			ReleaseMutex(theApp.robotServer.aimirobot.hMutex);//解锁
			aimibotkeyflag = 0;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}



//checkbox  使能电机开关
void CMultiRobotDlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码

	//查看当前选择机器人。
	int index = m_RobotList.GetCurSel();
	if (index < 0)
	{
		return;
	}
	//锁挂
	WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
	if (m_moveEnable.GetCheck() == BST_CHECKED)//enabel
	{
		theApp.robotServer.robotlist[index].setTorque(1);
	}
	else
	{
		theApp.robotServer.robotlist[index].setTorque(0);
	}
	//解锁
	ReleaseMutex(theApp.robotServer.hMutex);
}



void CMultiRobotDlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	m_movelin_display = (float)m_movelin.GetPos()/ 100.0;
	UpdateData(false);
	*pResult = 0;
}



void CMultiRobotDlg::OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	m_moveang_display = (float)m_moveang.GetPos()/ 100.0;
	UpdateData(false);
	*pResult = 0;
}

//开始运动
void CMultiRobotDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	int index = m_RobotList.GetCurSel();
	if (index < 0)
	{
		return;
	}
	float ilin, iang;
	ilin = (float)m_movelin.GetPos() / 100.0;
	iang = (float)m_moveang.GetPos()/100;
	//锁挂
	WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
	theApp.robotServer.robotlist[index].move(ilin, iang);
	//解锁
	ReleaseMutex(theApp.robotServer.hMutex);
}


//停止运动
void CMultiRobotDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	int index = m_RobotList.GetCurSel();
	if (index < 0)
	{
		return;
	}

	//锁挂
	WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
	theApp.robotServer.robotlist[index].move(0, 0);
	//解锁
	ReleaseMutex(theApp.robotServer.hMutex);
}


void CMultiRobotDlg::On32775()
{
	// TODO: 在此添加命令处理程序代码

	directionDlg  Dlg;
	Dlg.DoModal();
}

//选IPC
void CMultiRobotDlg::OnLbnSelchangeList1()
{
	// TODO: 在此添加控件通知处理程序代码
	//查看选定的IPC编号
	int index = m_IPClist.GetCurSel();

	//显示rtsp
	m_rtsp=theApp.visionLSys.IPC[index].rtsp.c_str();
	UpdateData(false);
}

//程序关闭退出。主要用于清理内存释放。
void CMultiRobotDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	//安全退出线程
	WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
	WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
	theApp.ThreadOn = 0;
	ReleaseMutex(theApp.robotServer.hMutex);//解锁
	ReleaseMutex(theApp.visionLSys.hMutex);//解锁 
	Sleep(1000);
	//等待几大线程结束
	//WaitForMultipleObjects(1, &theApp.robotServer.hListenThread, true, INFINITE);
	//WaitForMultipleObjects(1, &theApp.visionLSys.hThread, true, INFINITE);

	//释放内存
	/*for (int i = 0; i <theApp.visionLSys.getIPCNum(); i++)
	{
		theApp.visionLSys.IPC[i].cap.release();
	}*/
	
}

//初始化IMU
void CMultiRobotDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	int index = m_RobotList.GetCurSel();
	if (index < 0)
	{
		return;
	}

	//锁挂
	WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
	theApp.robotServer.robotlist[index].initIMU();
	//解锁
	ReleaseMutex(theApp.robotServer.hMutex);

	//KillTimer(1);
	//Sleep(1000);
	//SetTimer(1, 500, NULL);

}

//测量定位延时的函数
void CMultiRobotDlg::On32777()
{
	// TODO: 在此添加命令处理程序代码
	//查看条件是否满足
	if (theApp.obj.size() <= 0)
	{
		AfxMessageBox(_T("地图中没有物体"));
		return;
	}

	if (theApp.robotServer.getRobotListNum()<=0)
	{
		AfxMessageBox(_T("没有机器人连接上服务器"));
		return;
	}

	//查找第一个机器人是否在地图上
	int objindex = theApp.visionLSys.findVecterElm(theApp.obj, theApp.robotServer.robotlist[0].robotID);
	if (objindex==-1)
	{
		AfxMessageBox(_T("第一个机器人不在地图上"));
		return;
	}
	//开始准备数据
	Vec3d lastpoint = theApp.obj[objindex].coordinate3D;

	theApp.robotServer.robotlist[0].move(0.2, 0);

	double starttime = cv::getTickCount();//开始计时
	WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
	double distance = zfun::distancePoint(lastpoint, theApp.obj[objindex].coordinate3D);
	ReleaseMutex(theApp.visionLSys.hMutex);//解锁
	while (distance<0.008)
	{
		WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
		lastpoint = theApp.obj[objindex].coordinate3D;
		ReleaseMutex(theApp.visionLSys.hMutex);//解锁

		Sleep(30);

		WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
		distance = zfun::distancePoint(lastpoint, theApp.obj[objindex].coordinate3D);
		ReleaseMutex(theApp.visionLSys.hMutex);//解锁
	}
	double overtime = ((double)cv::getTickCount() - starttime) / cv::getTickFrequency();
	theApp.robotServer.robotlist[0].move(0, 0);

	theApp.visionLSys.delayTime = overtime;
	m_delaytime = overtime;
	UpdateData(false);
	theApp.visionLSys.UpdateXMLfile();

	//定时刷新机器的线速度角速度。
	SetTimer(2, theApp.visionLSys.delayTime / CasheQueue_MAXSIZE, NULL);
	printd("测试完成");
	return ;

}

//开始标定世界坐标系
void CMultiRobotDlg::On32778()
{
	// TODO: 在此添加命令处理程序代码
	

	//对所有IPC挂锁。
	for (size_t i = 0; i < theApp.visionLSys.getIPCNum(); i++)
	{
		WaitForSingleObject(theApp.visionLSys.IPC[i].hMutexcap, INFINITE);//锁挂
	}

	bool ret=theApp.visionLSys.setWorld();

	//对所有IPC解锁。
	for (size_t i = 0; i < theApp.visionLSys.getIPCNum(); i++)
	{
		ReleaseMutex(theApp.visionLSys.IPC[i].hMutexcap);//解锁
	}


	if (ret == false)
	{
		AfxMessageBox(_T("标定失败。"));
	}
	else
	{
		printd("标定成功");
	}
}

//输出字符串信息
void CMultiRobotDlg::printd(CString cout)
{
	cout += _T("\r\n"); // 回车换行
	int lastLine = m_printout.LineIndex(m_printout.GetLineCount() - 1);
	m_printout.SetSel(lastLine + 1, lastLine + 2, 0);
	m_printout.ReplaceSel(cout); //在最后一行添加新的内容
	UpdateData(false);
}
void CMultiRobotDlg::printd(string cout)
{
	CString str(cout.c_str());
	
	str += _T("\r\n"); // 回车换行
	int lastLine = m_printout.LineIndex(m_printout.GetLineCount() - 1);
	m_printout.SetSel(lastLine + 1, lastLine + 2, 0);
	m_printout.ReplaceSel(str); //在最后一行添加新的内容
	UpdateData(false);
}
void CMultiRobotDlg::printd(int cout)
{
	CString str;
	str.Format(_T("int:%d"), cout);
	str += _T("\r\n"); // 回车换行
	int lastLine = m_printout.LineIndex(m_printout.GetLineCount() - 1);
	m_printout.SetSel(lastLine + 1, lastLine + 2, 0);
	m_printout.ReplaceSel(str); //在最后一行添加新的内容
	UpdateData(false);
}
void CMultiRobotDlg::printd(float cout)
{
	CString str;
	str.Format(_T("float:%f"), cout);
	str += _T("\r\n"); // 回车换行
	int lastLine = m_printout.LineIndex(m_printout.GetLineCount() - 1);
	m_printout.SetSel(lastLine + 1, lastLine + 2, 0);
	m_printout.ReplaceSel(str); //在最后一行添加新的内容
	UpdateData(false);
}

//开关显示2D地图
void CMultiRobotDlg::Onshow2Donoff()
{
	// TODO: 在此添加命令处理程序代码
	if (theApp.show2Dflag == false)
	{
		theApp.show2Dflag = true;
		m_Menu.CheckMenuItem(ID_32779, MF_BYCOMMAND | MF_CHECKED);
	}
	else
	{
		theApp.show2Dflag = false;
		m_Menu.CheckMenuItem(ID_32779, MF_BYCOMMAND | MF_UNCHECKED);
		
	}
	
}

//弹出产品版本
void CMultiRobotDlg::Onvision()
{
	// TODO: 在此添加命令处理程序代码
	AfxMessageBox(_T("测试版\n\r v0.13"));

}


void CMultiRobotDlg::OnAddIPC()
{
	// TODO: 在此添加命令处理程序代码
	addipcDlg Dlg;
	Size board_size = Size(9, 6);
	Size square_size;
	string ipc_rtsp;
	string filedir;
	if (Dlg.DoModal() == IDOK)
	{
		filedir = CT2CA(Dlg.m_path.GetBuffer(0));
		ipc_rtsp= CT2CA(Dlg.rtsp.GetBuffer(0));
		square_size = Size(Dlg.m_qsize, Dlg.m_qsize);
		bool ret = theApp.visionLSys.AddIPC(filedir, board_size, square_size, ipc_rtsp);
		if (ret == true)
		{
			printd("添加成功");
		}
		else
		{
			printd("添加失败");
		}
	}
}

//双击IPC列表中的IPC
void CMultiRobotDlg::OnLbnDblclkList1()
{
	// TODO: 在此添加控件通知处理程序代码
	theApp.seleteimshow = m_IPClist.GetCurSel();
}


void CMultiRobotDlg::OnBnClickedCheck3()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
	if (m_showimg == true)
	{
		theApp.seleteimshow = 0;
	}
	else
	{
		theApp.seleteimshow = -2;	
		
	}
	ReleaseMutex(theApp.visionLSys.hMutex);//解锁
}


void CMultiRobotDlg::OnTest_toPoint()
{
	// TODO: 在此添加命令处理程序代码
	WaitForSingleObject(theApp.robotServer.aimirobot.hMutex, INFINITE);
	
	CMultiRobotApp::task foo; 
	for (size_t i = 0; i < 10; i++)
	{
		foo.taskname = 1; foo.x = 0.3; foo.y = 0.2;
		theApp.taskqueue[0].push_back(foo);
		foo.taskname = 1; foo.x = 0.3; foo.y = 1.5;
		theApp.taskqueue[0].push_back(foo);
		foo.taskname = 1; foo.x = 1.7; foo.y = 1.5;
		theApp.taskqueue[0].push_back(foo);
		foo.taskname = 1; foo.x = 1.7; foo.y = 0.2;
		theApp.taskqueue[0].push_back(foo);
	}
	ReleaseMutex(theApp.robotServer.aimirobot.hMutex);//解锁


	

}



//开始分拣任务
void CMultiRobotDlg::OnsetTask()
{
	// TODO: 在此添加命令处理程序代码
	WaitForSingleObject(theApp.MultiRobotControl_Mutex, INFINITE);//锁挂
	theApp.bigtask = 1;
	ReleaseMutex(theApp.MultiRobotControl_Mutex);//解锁
}


//完成抓取信号
void CMultiRobotDlg::OnfinishGet_flag()
{
	// TODO: 在此添加命令处理程序代码
	WaitForSingleObject(theApp.MultiRobotControl_Mutex, INFINITE);//锁挂
	theApp.finishGet_flag = 1;
	ReleaseMutex(theApp.MultiRobotControl_Mutex);//解锁

}


void CMultiRobotDlg::OnEnChangeEdit14()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CMultiRobotDlg::OnSaveData()
{
	// TODO: 在此添加命令处理程序代码
	FileStorage xml("trackdata.xml", cv::FileStorage::WRITE);

	WaitForSingleObject(theApp.visionLSys.hMutex, INFINITE);//锁挂
	xml << "trackdata" << theApp.track_data;
	ReleaseMutex(theApp.visionLSys.hMutex);//解锁

	xml.release();
}

