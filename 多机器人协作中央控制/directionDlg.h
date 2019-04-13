#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// directionDlg 对话框

class directionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(directionDlg)

public:
	directionDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~directionDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 目录
	CTreeCtrl m_contents;
	virtual BOOL OnInitDialog();
	// //帮助说明
	CEdit m_readmedisplay;
	afx_msg void OnNMClickTree1(NMHDR *pNMHDR, LRESULT *pResult);
};
