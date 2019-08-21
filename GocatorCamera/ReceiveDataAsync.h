#pragma once
#include <GoSdk/GoSdk.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
// ReceiveDataAsync 对话框

class ReceiveDataAsync : public CDialogEx
{
	DECLARE_DYNAMIC(ReceiveDataAsync)

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RECEIVEASYNC };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	ReceiveDataAsync(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ReceiveDataAsync();

	CString setFileName;
	CString strFilePath;
	CString outputInfo;

	void SetStopCamera();
	void Enablelogdlg(int strType, CString strInfo);

	void SetUseReceiveAsync();

	kAssembly api = kNULL;
	GoSystem Gosystem = kNULL;
	GoSensor sensor = kNULL;
	GoDataSet dataset = kNULL;
	kStatus status = kNULL;
	kIpAddress ipAddress ;

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedAsynReflesh();

	void ToSavaPointCloudData();
	
	CEdit m_asyEditTxt;
};
