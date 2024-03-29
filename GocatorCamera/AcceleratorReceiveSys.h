#pragma once
#include "CCLogWnd.h"
#include <GoSdk/GoSdk.h>

// AcceleratorReceiveSys 对话框

class AcceleratorReceiveSys : public CDialogEx
{
	DECLARE_DYNAMIC(AcceleratorReceiveSys)

public:
	AcceleratorReceiveSys(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~AcceleratorReceiveSys();

	CLogWnd *m_logdlg;
	void SetUseAcceleratorSys();
	void SetStopCamera();

	CString AcceleStrTemp;
	CString AcceleStrOut;

	kAssembly api = kNULL;
	kStatus status;
	unsigned int i, j, k;
	GoSystem Gosystem = kNULL;
	GoSensor sensor = kNULL;
	GoAccelerator accelerator = kNULL;
	GoDataSet dataset = kNULL;
	GoStamp *stamp = kNULL;
	GoProfilePositionX positionX = kNULL;
	GoDataMsg dataObj;
	kIpAddress ipAddress;
	GoMeasurementData *measurementData = kNULL;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ACCELERATORRECEIVESYS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_accEdit;
	afx_msg void OnEnUpdateAcceleraedit();
	afx_msg void OnBnClickedAccReflesh();
	afx_msg void OnBnClickedOk();
};
