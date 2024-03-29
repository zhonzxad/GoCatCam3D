#pragma once
#include "CCLogWnd.h"

// SysAbout 对话框

class SysAbout : public CDialogEx
{
	DECLARE_DYNAMIC(SysAbout)

public:
	SysAbout(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~SysAbout();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	afx_msg void OnBnClickedShowallinfoBtn();

	virtual BOOL OnInitDialog();

	CLogWnd *m_abtLogNew;

	DECLARE_MESSAGE_MAP()
};
