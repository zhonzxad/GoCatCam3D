#pragma once
#include "CCLogWnd.h"


// SensorConfigure 对话框

class SensorConfigure : public CDialogEx
{
	DECLARE_DYNAMIC(SensorConfigure)

public:
	SensorConfigure(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~SensorConfigure();

	void SetCameraConfigure();
	CLogWnd *m_logdlg;

	CEdit m_conEdit;

	CString ConfiStrOut = NULL;
	CString ConfiStrTemp = NULL;

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedConfReflesh();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONFIGUREDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	
};
