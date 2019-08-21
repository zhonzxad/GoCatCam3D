#pragma once
// GocatorCameraDlg.h: 头文件
//

#include "ReceiveDataAsync.h"
#include "ReceiveDataSurface.h"
#include "ReceiveSyncData.h"
#include "ReceiveFromProfile.h"
#include "NewReceiveProfile.h"
#include "BackupRestore.h"
#include "ToDealOutSetCamera.h"
#include "BuddySenSystem.h"
#include "AcceleratorReceiveSys.h"
#include "SysStatus.h"
#include "Support.h"
#include "SysAbout.h"
#include "CCLogWnd.h"
#include <GoSdk/GoSdk.h>
#include <kApi/kApi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <memory.h>

#pragma once
#pragma comment(lib, "GoSdk.lib")
#pragma comment(lib, "kApi.lib")



// CGocatorCameraDlg 对话框
class CGocatorCameraDlg : public CDialogEx
{
// 构造
public:
	CGocatorCameraDlg(CWnd* pParent = nullptr);	// 标准构造函数

	CTabCtrl TabCtrl;
	ReceiveDataAsync m_recAsy;
	ReceiveDataSurface m_recSurface;
	ReceiveSyncData m_recSyncData;
	ReceiveFromProfile m_recFromPro;
	NewReceiveProfile m_newProfile;
	BackupRestore m_backRes;
	ToDealOutSetCamera m_dealOut;
	BuddySenSystem m_buddySys;
	AcceleratorReceiveSys m_accRecSys;
	Support m_supObj;


	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedSavetobmpBtn();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GOCATORCAMERA_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	BOOL m_bMoving;
	CPoint m_ptMouse;
	CRect m_rect;
	CStatusBar  m_Statusbar;
	POINT old;
	CLogWnd *m_logdlg;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	void ReSize(void);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


public:
	void InitChangeTabForm();
	int SaveImageFromSaveBtnClicked(CString strfilePath);

	
	afx_msg void OnBnClickedCloseBtn();
	afx_msg void OnBnClickedMinsizeBtn();
	afx_msg void OnBnClickedStartCamLongBtn();
	afx_msg void OnBnClickedOutSetStartCam();
	afx_msg void OnBnClickedReadsysconfigure();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBnClickedAbout();
	afx_msg void OnBnClickedSystempause();
};
