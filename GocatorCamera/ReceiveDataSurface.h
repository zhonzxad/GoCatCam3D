#pragma once
#include "CCLogWnd.h"
#include <GoSdk/GoSdk.h>

// ReceiveDataSurface 对话框

class ReceiveDataSurface : public CDialogEx
{
	DECLARE_DYNAMIC(ReceiveDataSurface)

public:
	ReceiveDataSurface(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ReceiveDataSurface();

	CLogWnd *m_logdlg;

	CString SurfaceStrOut;
	CString SurfaceStrTemp;

	kAssembly api = kNULL;
	kStatus status;
	GoSystem Gosystem = kNULL;
	GoSensor sensor = kNULL;
	GoDataSet dataset = kNULL;
	unsigned int i, j;
	kIpAddress ipAddress;

	void SetUseReceiveSurfaceFun();

	void SetStopCamera();
	
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RECEIVESURFACE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_surEdit;
	CString strFilePath, setFileName, outputInfo;

	afx_msg void OnBnClickedIDOK();
	afx_msg void OnBnClickedSurfReflesh();

	void ToSavaIntensData();
};
