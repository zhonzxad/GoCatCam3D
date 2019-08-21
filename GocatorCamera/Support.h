#pragma once


// Support 对话框

class Support : public CDialogEx
{
	DECLARE_DYNAMIC(Support)

public:
	Support(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Support();

	CString strInfo = NULL;
	CString strInfoTemp = NULL;

	void SetInitInfo();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SUPPORT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClickedOk();
	CStatic m_picCon;
	CEdit m_supEdit;
	CFont m_Font;
};
