#pragma once


// addipcDlg 对话框

class addipcDlg : public CDialogEx
{
	DECLARE_DYNAMIC(addipcDlg)

public:
	addipcDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~addipcDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 图片路径
	CString m_path;
	afx_msg void OnBnClickedButton1();
	CString rtsp;
	// 格子宽度 mm
	float m_qsize;
};
