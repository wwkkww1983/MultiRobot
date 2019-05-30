// directionDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "多机器人协作中央控制.h"
#include "directionDlg.h"
#include "afxdialogex.h"


// directionDlg 对话框

IMPLEMENT_DYNAMIC(directionDlg, CDialogEx)

directionDlg::directionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

directionDlg::~directionDlg()
{
}

void directionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_contents);
	DDX_Control(pDX, IDC_EDIT1, m_readmedisplay);
}


BEGIN_MESSAGE_MAP(directionDlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_TREE1, &directionDlg::OnNMClickTree1)
//	ON_NOTIFY(NM_THEMECHANGED, IDC_TREE1, &directionDlg::OnNMThemeChangedTree1)
END_MESSAGE_MAP()


// directionDlg 消息处理程序


BOOL directionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	HTREEITEM hRobotMFC= m_contents.InsertItem(_T("人机交互界面"));
	m_contents.InsertItem(_T("机器人列表"), hRobotMFC);
	m_contents.InsertItem(_T("机器人传感器数据"), hRobotMFC);
	m_contents.InsertItem(_T("机器人运动操作"), hRobotMFC);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void directionDlg::OnNMClickTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	DWORD dwpos = GetMessagePos();
	TVHITTESTINFO ht = { 0 };
	ht.pt.x = GET_X_LPARAM(dwpos);
	ht.pt.y = GET_Y_LPARAM(dwpos);
	::MapWindowPoints(HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1); //把屏幕坐标转换成控件坐标
	TreeView_HitTest(pNMHDR->hwndFrom, &ht);   //确定点击的是哪一项
	CString str = m_contents.GetItemText(ht.hItem);

	if (str == "机器人列表")
	{
		//清空
		m_readmedisplay.SetSel(0, -1);
		m_readmedisplay.ReplaceSel(_T(""));
		//添加内容
		CString neirongstr = _T("机器人列表\r\n会显示已经连上的turbot机器人，选上可以得到数据以及进行控制");
		m_readmedisplay.ReplaceSel(neirongstr); 
	}
	else if(str=="机器人传感器数据")
	{
		//清空
		m_readmedisplay.SetSel(0, -1);
		m_readmedisplay.ReplaceSel(_T(""));
		//添加内容
		CString neirongstr = _T("机器人传感器数据\r\n未加入内容");
		m_readmedisplay.ReplaceSel(neirongstr);
	}
	else if(str== "机器人运动操作")
	{
		//清空
		m_readmedisplay.SetSel(0, -1);
		m_readmedisplay.ReplaceSel(_T(""));
		//添加内容
		CString neirongstr = _T("机器人运动操作\r\nWSAD控制turbot机器人，选中机器人进行控制。\r\n上下左右控制爱米家机器人。");
		m_readmedisplay.ReplaceSel(neirongstr);
	}
}


//void directionDlg::OnNMThemeChangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	// 该功能要求使用 Windows XP 或更高版本。
//	// 符号 _WIN32_WINNT 必须 >= 0x0501。
//	// TODO: 在此添加控件通知处理程序代码
//	*pResult = 0;
//}
