
// MultiRobotDlg.h : 头文件
//

#pragma once
#include "afxvslistbox.h"
#include "afxwin.h"
#include "afxcmn.h"


// CMultiRobotDlg 对话框
class CMultiRobotDlg : public CDialogEx
{
// 构造
public:
	CMultiRobotDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MY_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	CMenu m_Menu;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// 机器人列表,对应robotAPI中机器人服务器类的robotlist，选中则聚焦这个机器人。
	CListBox m_RobotList;
	// 选中机器人的电压
	float m_voltage;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//对比更新robotlist的显示
	void updataRobotListDisplay();


	int keyflag = 0;//用于判断按键状态，是按下状态还是没按状态。0是没按
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
//	afx_msg void OnEnChangeEdit2();
	//定义用于显示IMU位姿信息
	float m_angular_velocity_x;
	float m_angular_velocity_y;
	float m_angular_velocity_z;
	float m_linear_acceleration_x;
	float m_linear_acceleration_y;
	float m_linear_acceleration_z;
	float m_roll;
	float m_pitch;
	float m_yaw;
	// 运动使能
	CButton m_moveEnable;
	afx_msg void OnBnClickedCheck1();
	// 移动的速度
	CSliderCtrl m_movelin;
	// 移动的角度
	CSliderCtrl m_moveang;
	float m_movelin_display;
	float m_moveang_display;

	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult);
	//开始运动按钮函数
	afx_msg void OnBnClickedButton1();
	//停止运动按钮函数
	afx_msg void OnBnClickedButton2();
	// 用于表示wsad按键是否使用滑动块的参数来跑
	CButton m_wsadFlag;
	afx_msg void On32775();
	// 显示IP地址和端口号
	CEdit m_disIPaddr;
	// 展示IPC的列表
	CListBox m_IPClist;
	// 显示所选IPC的rtsp地址
	CString m_rtsp;
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButton3();
	// 显示水平方向上的方向角度
	float m_direction;

	//测量定位延时的函数
	afx_msg void On32777();
	// 延时时间
	float m_delaytime;
	afx_msg void On32778();
	// 调试输出。
	CEdit m_printout;

	//自定义函数，用于调试输出
	void printd(CString cout);//输出字符串信息
	void printd(string cout);//输出字符串信息
	void printd(int cout);
	void printd(float cout);

	//开关显示2D地图
	afx_msg void Onshow2Donoff();
	
	afx_msg void Onvision();
	afx_msg void OnAddIPC();
	afx_msg void OnLbnDblclkList1();
	// 显示与不显示监控
	BOOL m_showimg;
	afx_msg void OnBnClickedCheck3();
};

//核心！！！！！
//程序的几大线程

//socket监听线程
DWORD WINAPI ListenAcceptThreadFun(LPVOID p);
//-socket子线程
DWORD WINAPI updataRobotStatusThreadFun(LPVOID p);

//IPC视觉处理定位线程
DWORD WINAPI IPCvisionLocationSystemThreadFun(LPVOID p);
//-IPC子处理线程
DWORD WINAPI IPCvisionLocationSonThreadFun(LPVOID p);
//显示线程
DWORD WINAPI IPCvisionLocationSon_ShowThreadFun(LPVOID p);
void map_mouse_callback(int event, int x, int y, int flags, void* param); //显示线程中的鼠标回调函数