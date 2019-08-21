#pragma once
#include "CCLogWnd.h"
#include "SetUseLogObj.h"
#include <GoSdk/GoSdk.h>
#include <kApi/kApi.h>

// ReceiveSyncData 对话框

class ReceiveSyncData : public CDialogEx
{
	DECLARE_DYNAMIC(ReceiveSyncData)

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RECEIVESYSDATA };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:

	ReceiveSyncData(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ReceiveSyncData();

	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();

	int ToSaveBMP(CString strFilePath, CString setFileName);
	
	CEdit m_syncEdit;
	afx_msg void OnEnChangeReceiveedit();
	afx_msg void OnBnClickedExportcsvBtn();

	int SaveCsvFromSaveBtnClicked(CString strfilePath);
	int SaveCsvFromSaveBtnClicked();
	void ToDoSaveCsv();
	afx_msg void OnBnClickedSyncReflesh();

	CString SyncStrOut = "0";
	CString SyncStrTemp = "0";


	CLogWnd m_logdlg;
	SetUseLogObj m_UseLog;
	CString strFilePath, setFileName, outputInfo;

	void SetUseReceiveSync();
	void SetStopCamera();

	void ToSavaPointCloudData();

	void TestFor();
};
