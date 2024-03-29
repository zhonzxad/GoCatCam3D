#pragma once
#include "CCLogWnd.h"
#include <GoSdk/GoSdk.h>

// BackupRestore 对话框

class BackupRestore : public CDialogEx
{
	DECLARE_DYNAMIC(BackupRestore)

public:
	BackupRestore(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~BackupRestore();

	CLogWnd *m_logdlg;

	CString BackUpTemp;
	CString BackUpOut;

	kStatus status;
	kAssembly api = kNULL;
	GoSystem Gosystem = kNULL;
	GoSensor sensor = kNULL;
	char* backupFile = "SystemBackup.bin";
	kIpAddress ipAddress;
	void SetUseBackupRestore();

	void SetStopCamera();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BACKUPRESTORE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_backEdit;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBackReflesh();
};
