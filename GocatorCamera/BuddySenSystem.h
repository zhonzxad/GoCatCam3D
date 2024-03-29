#pragma once
#include "CCLogWnd.h"
#include <GoSdk/GoSdk.h>
// BuddySenSystem 对话框

class BuddySenSystem : public CDialogEx
{
	DECLARE_DYNAMIC(BuddySenSystem)

public:
	BuddySenSystem(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~BuddySenSystem();

	CLogWnd *m_logdlg;

	CString BuddyStrTemp;
	CString BuddyStrOut;

	kAssembly api = kNULL;
	kStatus status;
	unsigned int i, j, k, arrayIndex;
	GoSystem Gosystem = kNULL;
	GoSensor sensor = kNULL;
	GoSensor buddy = kNULL;
	GoDataSet dataset = kNULL;
	GoStamp *stamp = kNULL;
	GoDataMsg dataObj;
	kIpAddress ipAddress, ipAddress_buddy;
	GoSetup setup = kNULL;
	GoSetup buddy_setup = kNULL;
	k32u profilePointCount;

	void SetUseBuddySysteam();

	void SetStopCamera();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BUDDYSYSTEM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_buddyEdit;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBuddReflesh();
};
