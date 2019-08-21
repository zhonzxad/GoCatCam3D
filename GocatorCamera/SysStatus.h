#pragma once


// SysStatus 对话框

class SysStatus : public CDialogEx
{
	DECLARE_DYNAMIC(SysStatus)

public:
	SysStatus(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~SysStatus();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SYSSTATUS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	CListCtrl m_SysStat;
	CListCtrl m_SysCount;
	afx_msg void OnBnClickedButSure();
	afx_msg void OnBnClickedButClose();
	virtual void PostNcDestroy();
	virtual void ShowSysInfo();
	virtual void ShowSysCount();
};
