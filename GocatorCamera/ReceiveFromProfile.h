#pragma once
#include <GoSdk/GoSdk.h>
#include "NewReceiveProfile.h"

// ReceiveFromProfile 对话框

class ReceiveFromProfile : public CDialogEx
{
	DECLARE_DYNAMIC(ReceiveFromProfile)

public:
	ReceiveFromProfile(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ReceiveFromProfile();

	kAssembly api = kNULL;
	kStatus status;
	GoSystem Gosystem = kNULL;
	GoSensor sensor = kNULL;
	GoProfilePositionX positionX = kNULL;
	kIpAddress ipAddress;
	GoSetup setup = kNULL;
	GoTools tools = kNULL;
	GoOutput outputModule = kNULL;
	GoEthernet ethernetOutput = kNULL;
	NewReceiveProfile m_newMode;

	void SetUseReceiveFromProfile();

	void SetStopCamera();
	
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RECFROMPROFILEMODE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedProReflesh();
};
