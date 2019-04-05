
// MultiRobotDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "多机器人协作中央控制.h"
#include "MultiRobotDlg.h"
#include "afxdialogex.h"


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
	m_Menu.LoadMenu(IDR_MENU1);//初始化菜单
	SetMenu(&m_Menu);
	//zzq测试代码
	theApp.robotServer.init(6000);
	if (theApp.robotServer.is_Open() < 0)
	{
		AfxMessageBox(_T("服务器初始化失败"));
	}
	theApp.robotServer.hMutex = CreateMutex(NULL, FALSE, NULL);
	//开启监听线程
	theApp.robotServer.hListenThread = CreateThread(NULL, 0, ListenAcceptThreadFun, NULL, 0, &theApp.robotServer.ListenThreadID);
	//设置对话框刷新时间
	SetTimer(1, 500, NULL); m_voltage = 0;
	//初始化运动控制滑动条
	m_movelin.SetRange(-50, 50);
	m_movelin.SetTicFreq(5);
	m_movelin.SetPos(0);
	//m_movelin.SetLineSize(5);//一行的大小，对应键盘的方向键
	m_moveang.SetRange(-180, 180);
	m_moveang.SetTicFreq(5);
	m_moveang.SetPos(0);
	





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





/*----------------------------------------------------------------------
						多线程函数定义
*/

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
	
	while (true)
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
			uint8_t therobotid = theApp.robotServer.robotlist.back().robotID;
			h1 = CreateThread(NULL, 0, updataRobotStatusThreadFun, &therobotid, 0, &id1);
			hUpdataRobotThread.push_back(h1);
			UpdataRobotThreadID.push_back(id1);
			//解锁
			ReleaseMutex(theApp.robotServer.hMutex);
		}
	}


}


//刷新机器人状态 如果掉线直接从robotServer.robotlist中pop掉，并且关闭线程
DWORD WINAPI updataRobotStatusThreadFun(LPVOID p)
{
	uint8_t robotid = *(uint8_t*)p;
	int robotindex = theApp.robotServer.findID(robotid);

	//判断网络连接 断开则跳出循环
	while (theApp.robotServer.robotlist[robotindex].connectStatus>0)
	{
		//updata 电压值
		WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
		robotindex = theApp.robotServer.findID(robotid);
		theApp.robotServer.robotlist[robotindex].Voltage = theApp.robotServer.robotlist[robotindex].getVoltage();
		//解锁
		ReleaseMutex(theApp.robotServer.hMutex);

		

		Sleep(1000);
	}
	
	//删除该id的进程 从robotServer.robotlist中pop掉
	WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
	robotindex = theApp.robotServer.findID(robotid);
	vector<robot>::iterator it = theApp.robotServer.robotlist.begin() + robotindex;
	theApp.robotServer.robotlist.erase(it);
	//解锁
	ReleaseMutex(theApp.robotServer.hMutex);
	return 0;
}




void CMultiRobotDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	
	//锁挂
	WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
	//刷新机器人列表
	updataRobotListDisplay();
	
	//刷新指定机器人的信息：电压 IMU 
	int index = m_RobotList.GetCurSel();
	if (index >= 0)
	{
		//刷新电压
		m_voltage = theApp.robotServer.robotlist[index].Voltage;
		//刷新IMU
		imu_msg imudata;
		imudata = theApp.robotServer.robotlist[index].getIMU();
		m_angular_velocity_x = zfun::numFormat(imudata.angular_velocity_x,2);
		m_angular_velocity_y = zfun::numFormat(imudata.angular_velocity_y,2);
		m_angular_velocity_z = zfun::numFormat(imudata.angular_velocity_z,2);
		m_linear_acceleration_x = zfun::numFormat(imudata.linear_acceleration_x,2);
		m_linear_acceleration_y = zfun::numFormat(imudata.linear_acceleration_y,2);
		m_linear_acceleration_z = zfun::numFormat(imudata.linear_acceleration_z,2);
		//四元组转RPY
		Eigen::Vector3d rpy;
		rpy = zfun::Quaterniond2Euler(imudata.orientation_x, imudata.orientation_y, imudata.orientation_z, imudata.orientation_w);
		m_roll = zfun::numFormat(rpy[0],3);
		m_pitch = zfun::numFormat(rpy[1], 3);
		m_yaw = zfun::numFormat(rpy[2], 3);
		//刷新运动控制使能控件
		if (theApp.robotServer.robotlist[index].getTorque() == 1)
		{
			m_moveEnable.SetCheck(true);
		}
		else if(theApp.robotServer.robotlist[index].getTorque() == 0)
		{
			m_moveEnable.SetCheck(false);
		}
	}

	//解锁
	ReleaseMutex(theApp.robotServer.hMutex);

	UpdateData(false);

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
			robotname.Format(_T("%x"), theApp.robotServer.robotlist[i].robotID);
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
			robotname.Format(_T("%x"), theApp.robotServer.robotlist[i].robotID);


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
		if (keyflag == 0)
		{
			////查看当前选择机器人。
			int index= m_RobotList.GetCurSel();
			if (index < 0)
			{
				return CDialogEx::PreTranslateMessage(pMsg);
			}
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
				break;
			case 'S'://后退
				WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
				theApp.robotServer.robotlist[index].move(-ilin, 0);
				//解锁
				ReleaseMutex(theApp.robotServer.hMutex);
				break;
			case 'A'://左
				WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
				theApp.robotServer.robotlist[index].move(0, iang);
				//解锁
				ReleaseMutex(theApp.robotServer.hMutex);
				break;
			case 'D'://右
				WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);//锁挂
				theApp.robotServer.robotlist[index].move(0, -iang);
				//解锁
				ReleaseMutex(theApp.robotServer.hMutex);
				break;
			default:
				break;
			}
		}
		keyflag = 1;
		return true;
	}
	else if (pMsg->message == WM_KEYUP)
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

	return CDialogEx::PreTranslateMessage(pMsg);
}


//void CMultiRobotDlg::OnEnChangeEdit2()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//}

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
	m_movelin_display = m_movelin.GetPos();
	UpdateData(false);
	*pResult = 0;
}



void CMultiRobotDlg::OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	m_moveang_display = m_moveang.GetPos();
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
