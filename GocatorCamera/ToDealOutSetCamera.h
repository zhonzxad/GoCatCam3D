#pragma once

#include <GoSdk/GoSdk.h>
#include <kApi/kApi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <memory.h>

// ToDealOutSetCamera 对话框

class ToDealOutSetCamera : public CDialogEx
{
	DECLARE_DYNAMIC(ToDealOutSetCamera)

public:
	ToDealOutSetCamera(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ToDealOutSetCamera();

	int SetOutPutStartCamera();
	void SetStaticTxt();

	
	
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GOCATORCAMERA_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
