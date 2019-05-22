// addipcDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "多机器人协作中央控制.h"
#include "addipcDlg.h"
#include "afxdialogex.h"


// addipcDlg 对话框

IMPLEMENT_DYNAMIC(addipcDlg, CDialogEx)

addipcDlg::addipcDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG2, pParent)
	, m_path(_T(""))
	, rtsp(_T(""))
	, m_qsize(0)
{

}

addipcDlg::~addipcDlg()
{
}

void addipcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_path);
	DDX_Text(pDX, IDC_EDIT2, rtsp);
	DDX_Text(pDX, IDC_EDIT3, m_qsize);
}


BEGIN_MESSAGE_MAP(addipcDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &addipcDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// addipcDlg 消息处理程序


void addipcDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	//// TODO: 在此添加控件通知处理程序代码    
	
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = m_hWnd;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	BOOL bRet = FALSE;
	TCHAR szFolder[MAX_PATH * 2];
	szFolder[0] = _T('/0');
	if (pidl)
	{
		if (SHGetPathFromIDList(pidl, szFolder))
			bRet = TRUE;
		IMalloc *pMalloc = NULL;
		if (SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc)
		{
			pMalloc->Free(pidl);
			pMalloc->Release();
		}
	}
	m_path = szFolder;//选择的文件夹路径    
	UpdateData(false);
}
