#pragma once


// ShowAllInfo 对话框

class ShowAllInfo : public CDialogEx
{
	DECLARE_DYNAMIC(ShowAllInfo)

public:
	ShowAllInfo(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ShowAllInfo();

	void SetInfo();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ALlINFODLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();

	CEdit m_allInfoEdit;
	CFont m_Font;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
