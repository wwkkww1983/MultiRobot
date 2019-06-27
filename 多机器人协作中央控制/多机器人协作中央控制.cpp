
// 多机器人协作中央控制.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "多机器人协作中央控制.h"
#include "MultiRobotDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMultiRobotApp

BEGIN_MESSAGE_MAP(CMultiRobotApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMultiRobotApp 构造

CMultiRobotApp::CMultiRobotApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
	//robotServer(6000);
}


// 唯一的一个 CMultiRobotApp 对象

CMultiRobotApp theApp;


// CMultiRobotApp 初始化

BOOL CMultiRobotApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));


	//增加初始化zzq
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	CMultiRobotDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}





int CMultiRobotApp::ExitInstance()
{
	// TODO: 在此添加专用代码和/或调用基类
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
	return CWinApp::ExitInstance();
}

void CMultiRobotApp::taskqueue_push(int index, int taskn, Point2f toP, int lf)
{
	CMultiRobotApp::task ntask;

	if (taskn == 1)
	{
		ntask.taskname = taskn; ntask.x = toP.x; ntask.y = toP.y;
		WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
		theApp.taskqueue[index].push_back(ntask);
		ReleaseMutex(theApp.robotServer.hMutex);//解锁
	}
	else if(taskn==3)
	{
		ntask.taskname = taskn; ntask.x = toP.x; ntask.y = toP.y; ntask.lf = lf;
		WaitForSingleObject(theApp.robotServer.hMutex, INFINITE);
		theApp.taskqueue[index].push_back(ntask);
		ReleaseMutex(theApp.robotServer.hMutex);//解锁
	}
	
}

void CMultiRobotApp::uArmTaskQueue_push(int taskn, Vec3f p1, Vec3f p2)
{
	CMultiRobotApp::task foo;
	float dx = 293, dy = 386, dz = 0;

	WaitForSingleObject(theApp.huArmTaskMutex, INFINITE);

	foo.taskname = taskn;
	foo.x = (p1[0] * 1000 + dx);
	foo.y = (p1[1] * 1000 + dy);
	foo.z = (p1[2] * 1000 + dz);
	foo.x1 = p2[0] * 1000 + dx;
	foo.y1 = p2[1] * 1000 + dy;
	foo.z1 = p2[2] * 1000 + dz;

	if (taskn == 4)
	{
		//调整限幅
		float dd = sqrt(foo.x*foo.x + foo.y*foo.y);
		float dd1 = sqrt(foo.x1*foo.x1 + foo.y1*foo.y1);
		if (dd > 350 || dd1 > 350)
		{
			ReleaseMutex(theApp.huArmTaskMutex);//解锁
			return;
		}
			
	}
	theApp.uArmTaskQueue.push_back(foo);


	ReleaseMutex(theApp.huArmTaskMutex);//解锁
}