// ReceiveDataAsync.cpp: 实现文件
//
// ReceiveDataAsync 消息处理程序
/*
 * ReceiveASync.c
 *
 * Gocator 2000 Sample
 * Copyright (C) 2011-2018 by LMI Technologies Inc.
 *
 * Licensed under The MIT License.
 * Redistributions of files must retain the above copyright notice.
 *
 * Purpose: Connect to Gocator system and receive data using a callback function.
 * Ethernet output for the desired data must be enabled.
 *
 */
#include "stdafx.h"
#include "GocatorCamera.h"
#include <Windows.h>
#include "ReceiveDataAsync.h"
#include "afxdialogex.h"
#include "CCLogWnd.h"


#define SENSOR_IP           "192.168.1.10"
#define NUM_PROFILES         5
//#define INFO  1   将各个量定义为
//#define WARN  2
//#define ERR   3

#pragma warning(disable:4002)
#pragma warning(disable:4244)
#pragma warning(disable:6011)
#pragma warning(disable:4018)
#pragma warning(disable:6387)
#pragma warning(disable:6386)
#pragma warning(disable:6328)
#pragma warning(disable:26495)

kStatus kCall onReceiveAsyncData(void* ctx, void* sys, void* dataset);
CLogWnd *m_AsyncLogDlg;
CStatic m_asyncTxt;

CString AsyncStrOut = NULL;
CString AsyncStrTemp = NULL;

CEdit m_asyEditTxt;

#define NM_TO_MM(VALUE) (((k64f)(VALUE))/1000000.0)
#define UM_TO_MM(VALUE) (((k64f)(VALUE))/1000.0)
#define INVALID_RANGE_16BIT     ((signed short)0x8000)   // gocator transmits range data as 16-bit signed integers. 0x8000 signifies invalid range data. 
#define INVALID_RANGE_DOUBLE    ((k64f)-DOUBLE_MAX)             // floating point value to represent invalid range data.

typedef struct
{
	double x;   // x-coordinate in engineering units (mm) - position along laser line
	double y;   // y-coordinate in engineering units (mm) - position along the direction of travel
	double z;   // z-coordinate in engineering units (mm) - height (at the given x position)
	unsigned char intensity;
}ProfilePoint;

short int* Asy_height_map_memory = NULL;
unsigned char* Asy_intensity_image_memory = NULL;
ProfilePoint** Ast_surfaceBuffer = NULL;
//unsigned int rowIdx, colIdx;
unsigned int AsyLength, AsyWidth;
k32u Asy_surfaceBufferHeight = 0;



typedef struct
{
	k32u count;
}DataContext;

// ReceiveDataAsync 对话框
IMPLEMENT_DYNAMIC(ReceiveDataAsync, CDialogEx)

//定义一个全局指针
ReceiveDataAsync* Asyn_dlg = NULL;

ReceiveDataAsync::ReceiveDataAsync(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RECEIVEASYNC, pParent)
{
	//Rev_dlg = this;

#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

ReceiveDataAsync::~ReceiveDataAsync()
{
}

void ReceiveDataAsync::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ASYNCSTATIC, m_asyEditTxt);
}


BEGIN_MESSAGE_MAP(ReceiveDataAsync, CDialogEx)
	ON_BN_CLICKED(IDOK, &ReceiveDataAsync::OnBnClickedOk)
	ON_BN_CLICKED(IDC_ASYN_REFLESH, &ReceiveDataAsync::OnBnClickedAsynReflesh)
END_MESSAGE_MAP()

BOOL ReceiveDataAsync::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	Asyn_dlg = this;

	return TRUE;
}

//data callback function
kStatus kCall onReceiveAsyncData(void* ctx, void* sys, void* dataset)
{
	unsigned int i, j;
	DataContext *context = (DataContext *)ctx;

	//引用 https://www.cnblogs.com/ike_li/p/4431961.html
	//TODO 问题在于初始化的时候没用重载OnInit函数，定义全局变量应该没用成功
	//下一步问题在于重载函数，链接相机测试运行
	//ReceiveDataAsync m_useLogDlg = Rev_dlg; //直接使用就可以了

	
	AsyncStrTemp.Format(_T("程序进入异步回调事件中-----\r\n"));
	AsyncStrOut += AsyncStrTemp;
	AsyncStrTemp.Format(_T("Callback:\r\n"));
	AsyncStrOut += AsyncStrTemp;
	AsyncStrTemp.Format(_T("Data message received:\r\n"));
	AsyncStrOut += AsyncStrTemp;
	AsyncStrTemp.Format(_T("Dataset count: %u\r\n"), GoDataSet_Count(dataset));
	//AsyncStrTemp.Format(_T("Dataset count: %u\r\n", GoDataSet_Count(dataset)));  //原测试方法
	AsyncStrOut += AsyncStrTemp;

	//m_asyEditTxt.SetWindowText(AsyncStrOut);

	Asyn_dlg->m_asyEditTxt.SetWindowText(AsyncStrOut);

	/*m_asyEditTxt.SetSel(-1, 0, FALSE);
	m_asyEditTxt.SetFocus();*/

	Asyn_dlg->m_asyEditTxt.SetSel(-1, 0, FALSE);
	Asyn_dlg->m_asyEditTxt.SetFocus();

	Asyn_dlg->m_asyEditTxt.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	for (i = 0; i < GoDataSet_Count(dataset); ++i)
	{
		GoDataMsg dataObj = GoDataSet_At(dataset, i);
		// retrieve GoStamp message
		switch (GoDataMsg_Type(dataObj))
		{
		case GO_DATA_MESSAGE_TYPE_STAMP:
		{
			GoStampMsg stampMsg = dataObj;
			for (j = 0; j < GoStampMsg_Count(stampMsg); ++j)
			{
				GoStamp *stamp = GoStampMsg_At(stampMsg, j);
				AsyncStrTemp.Format(_T("  Timestamp: %11u\r\n"), stamp->timestamp);
				AsyncStrOut += AsyncStrTemp;
				AsyncStrTemp.Format(_T("  Encoder: %11d\r\n"), stamp->encoder);
				AsyncStrOut += AsyncStrTemp;;
				AsyncStrTemp.Format(_T("  Frame index: %11u\r\n"), stamp->frameIndex);
				AsyncStrOut += AsyncStrTemp;
				//SetDlgItemText((HWND)IDC_ASYNCSTATIC, 1020, AsyncStrOut);
				Asyn_dlg->m_asyEditTxt.SetWindowText(AsyncStrOut);
				context->count++;
			}
		}
		break;
		case GO_DATA_MESSAGE_TYPE_UNIFORM_SURFACE:
		{
			GoSurfaceMsg surfaceMsg = dataObj;
			unsigned int rowIdx, colIdx;

			double XResolution = NM_TO_MM(GoSurfaceMsg_XResolution(surfaceMsg));
			double YResolution = NM_TO_MM(GoSurfaceMsg_YResolution(surfaceMsg));
			double ZResolution = NM_TO_MM(GoSurfaceMsg_ZResolution(surfaceMsg));
			double XOffset = UM_TO_MM(GoSurfaceMsg_XOffset(surfaceMsg));
			double YOffset = UM_TO_MM(GoSurfaceMsg_YOffset(surfaceMsg));
			double ZOffset = UM_TO_MM(GoSurfaceMsg_ZOffset(surfaceMsg));

			AsyncStrTemp.Format(_T("  Surface data width: %ld\r\n"), GoSurfaceMsg_Width(surfaceMsg));
			AsyncStrOut += AsyncStrTemp;
			AsyncStrTemp.Format(_T("  Surface data length: %ld\r\n\r\n"), GoSurfaceMsg_Length(surfaceMsg));
			AsyncStrOut += AsyncStrTemp;;
			Asyn_dlg->m_asyEditTxt.SetWindowText(AsyncStrOut);

			//allocate memory if needed
			if (Ast_surfaceBuffer == NULL)
			{
				Ast_surfaceBuffer = (ProfilePoint * *)malloc(GoSurfaceMsg_Length(surfaceMsg) * sizeof(ProfilePoint*));

				for (j = 0; j < GoSurfaceMsg_Length(surfaceMsg); j++)
				{
					Ast_surfaceBuffer[j] = (ProfilePoint*)malloc(GoSurfaceMsg_Width(surfaceMsg) * sizeof(ProfilePoint));
				}

				Asy_surfaceBufferHeight = GoSurfaceMsg_Length(surfaceMsg);
			}

			for (rowIdx = 0; rowIdx < GoSurfaceMsg_Length(surfaceMsg); rowIdx++)
			{
				k16s* data = GoSurfaceMsg_RowAt(surfaceMsg, rowIdx);

				for (colIdx = 0; colIdx < GoSurfaceMsg_Width(surfaceMsg); colIdx++)
				{
					// gocator transmits range data as 16-bit signed integers
					// to translate 16-bit range data to engineering units, the calculation for each point is: 
					//          X: XOffset + columnIndex * XResolution 
					//          Y: YOffset + rowIndex * YResolution
					//          Z: ZOffset + height_map[rowIndex][columnIndex] * ZResolution

					Ast_surfaceBuffer[rowIdx][colIdx].x = XOffset + XResolution * colIdx;
					Ast_surfaceBuffer[rowIdx][colIdx].y = YOffset + YResolution * rowIdx;


					if (data[colIdx] != INVALID_RANGE_16BIT)
					{
						Ast_surfaceBuffer[rowIdx][colIdx].z = ZOffset + ZResolution * data[colIdx];
					}
					else
					{
						Ast_surfaceBuffer[rowIdx][colIdx].z = -9999;
					}
				}
				AsyLength = GoSurfaceMsg_Length(surfaceMsg);
				AsyWidth = GoSurfaceMsg_Width(surfaceMsg);
			}
		}
		break;
		// Refer to example ReceiveRange, ReceiveProfile, ReceiveMeasurement and ReceiveWholePart on how to receive data                
		}
	}

	Asyn_dlg->m_asyEditTxt.SetSel(-1, 0, FALSE);
	Asyn_dlg->m_asyEditTxt.SetFocus();

	Asyn_dlg->m_asyEditTxt.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	GoDestroy(dataset);
	return kOK;
}

void ReceiveDataAsync::SetUseReceiveAsync() //int argc, char **argv
{
	DataContext contextPointer;

	//TODO为了解决回调中输出的问题
	//ReceiveDataAsync m_useLogDlg = Rev_dlg; //直接使用就可以了

	//m_useLogDlg.GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行异步接收信号程序"));
	//Enablelogdlg(4, _T("程序进入回调事件中----"));
	//GetDlgItem(IDC_ASYNCSTATIC)->SetWindowText(_T("程序进入回调事件中----"));
	
	//AsyncStrTemp = AsyncStrOut = "0";

	AsyncStrTemp.Format(_T("初始化进入异步拍摄功能：\r\n"));
	AsyncStrOut += AsyncStrTemp;
	//SetDlgItemText(IDC_ASYNCSTATIC, AsyncStrOut);
	Asyn_dlg->m_asyEditTxt.SetWindowText(AsyncStrOut);
	Asyn_dlg->m_asyEditTxt.SetSel(-1, 0, FALSE);
	Asyn_dlg->m_asyEditTxt.SetFocus();

	Asyn_dlg->m_asyEditTxt.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
	
	// construct Gocator API Library
	if ((status = GoSdk_Construct(&api)) != kOK)
	{
		//m_useLogDlg.GetDlgItem(IDC_ASYNCSTATIC)->SetWindowText((_T("Error: GoSdk_Construct:%d\n", status))); 
		//SetDlgItemText(IDC_ASYNCSTATIC, (_T("Error: GoSdk_Construct:%d\n", status)));
		Asyn_dlg->m_asyEditTxt.SetWindowText(_T("Error: GoSdk_Construct:%d\n", status));
		return ;
	}

	// construct GoSystem object
	if ((status = GoSystem_Construct(&Gosystem, kNULL)) != kOK)
	{
		//SetDlgItemText(IDC_ASYNCSTATIC, (_T("Error: GoSystem_Construct:%d\n", status)));
		Asyn_dlg->m_asyEditTxt.SetWindowText(_T("Error: GoSystem_Construct:%d\n", status));
		return ;
	}

	// Parse IP address into address data structure
	kIpAddress_Parse(&ipAddress, SENSOR_IP);

	// obtain GoSensor object by sensor IP address
	if ((status = GoSystem_FindSensorByIpAddress(Gosystem, &ipAddress, &sensor)) != kOK)
	{
		//SetDlgItemText(IDC_ASYNCSTATIC, (_T("Error: GoSystem_FindSensor:%d\n", status)));
		Asyn_dlg->m_asyEditTxt.SetWindowText(_T("Error: GoSystem_FindSensor:%d\n", status));
		return;
	}

	// create connection to GoSystem object
	if ((status = GoSystem_Connect(Gosystem)) != kOK)
	{
		//SetDlgItemText(IDC_ASYNCSTATIC, (_T("Error: GoSystem_Connect:%d\n", status)));
		Asyn_dlg->m_asyEditTxt.SetWindowText(_T("Error: GoSystem_Connect:%d\n", status));
		return;
	}

	// enable sensor data channel
	if ((status = GoSystem_EnableData(Gosystem, kTRUE)) != kOK)
	{
		//SetDlgItemText(IDC_ASYNCSTATIC, (_T("Error: GoSensor_EnableData:%d\n", status)));
		Asyn_dlg->m_asyEditTxt.SetWindowText(_T("Error: GoSensor_EnableData:%d\n", status));
		return;
	}

	//set data handler to receive data asynchronously
	if ((status = GoSystem_SetDataHandler(Gosystem, onReceiveAsyncData, &contextPointer)) != kOK)
	{
		//SetDlgItemText(IDC_ASYNCSTATIC, (_T("Error: GoSystem_SetDataHandler:%d\n", status)));
		Asyn_dlg->m_asyEditTxt.SetWindowText(_T("Error: GoSystem_SetDataHandler:%d\n", status));
		return;
	}

	// start Gocator sensor
	if ((status = GoSystem_Start(Gosystem)) != kOK)
	{
		//SetDlgItemText(IDC_ASYNCSTATIC, (_T("Error: GoSystem_Start:%d\n", status)));
		Asyn_dlg->m_asyEditTxt.SetWindowText(_T("Error: GoSystem_Start:%d\n", status));
		return;
	}

	//m_AsyncLogDlg->WARN(_T("Press any key to stop sensor...\n"));
	//getchar();

	//// stop Gocator sensor
	//if ((status = GoSystem_Stop(system)) != kOK)
	//{
	//	m_AsyncLogDlg->WARN(_T("Error: GoSystem_Stop:%d\n", status));
	//	return;
	//}

	//// destroy handles
	//GoDestroy(system);
	//GoDestroy(api);

	//m_AsyncLogDlg->WARN(_T("Press any key to continue...\n"));
	//getchar();

	Asyn_dlg->m_asyEditTxt.SetSel(-1, 0, FALSE);
	Asyn_dlg->m_asyEditTxt.SetFocus();

	Asyn_dlg->m_asyEditTxt.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	return;
}

void ReceiveDataAsync::SetStopCamera()
{
	// stop Gocator sensor
	if ((status = GoSystem_Stop(Gosystem)) != kOK)
	{
		MessageBox("结束相机失败");
	}

	Asyn_dlg->m_asyEditTxt.SetSel(-1, 0, FALSE);
	Asyn_dlg->m_asyEditTxt.SetFocus();

	Asyn_dlg->m_asyEditTxt.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	// destroy handles
	GoDestroy(Gosystem);
	GoDestroy(api);

}

void ReceiveDataAsync::Enablelogdlg(int strType, CString strInfo)
{
	//TODO 函数废弃
	switch (strType)
	{
	case 1:
		//m_AsyncLogDlg->INFO(_T(strInfo));
		break;
	case 2:
		//m_AsyncLogDlg->WARN(_T(strInfo));
		break;
	case 3:
		//m_AsyncLogDlg->ERR(_T(strInfo));
		break;
	case 4:
		//SetDlgItemText(IDC_ASYNCSTATIC, AsyncStrOut);
		break;
	default:
		break;
	}
}

void ReceiveDataAsync::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	MessageBox(_T("无效确认操作！\r\n"), _T("GoCator相机操作"), MB_OK | MB_ICONEXCLAMATION);
}

void ReceiveDataAsync::OnBnClickedAsynReflesh()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(_T("刷新相机需要重新创建配置\r\n期间请不要移动界面\r\n等待相机完成操作\r\n等待2到3秒"), _T("GoCator相机提示"), MB_OK);

	AsyncStrTemp.Format(_T("等待相机进行重连操作....\r\n"));
	AsyncStrOut += AsyncStrTemp;
	//SetDlgItemText(IDC_ASYNCSTATIC, AsyncStrOut);
	Asyn_dlg->m_asyEditTxt.SetWindowText(AsyncStrOut);

	Asyn_dlg->m_asyEditTxt.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	SetUseReceiveAsync();
	SetStopCamera();
}

void ReceiveDataAsync::ToSavaPointCloudData()
{
	if (Ast_surfaceBuffer != NULL)
	{
		Asyn_dlg->m_asyEditTxt.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

		errno_t err;
		FILE* fpWrite;

		TCHAR szFilter[] = _T("文本文件(*.txt)|*.txt | 所有文件(*.*)|*.*||");
		// 设置按时间保存文件名
		CTime m_Time;
		m_Time = CTime::GetCurrentTime();//获取当前系统时间
		CString str = (CString)m_Time.Format(_T("%Y-%m-%d_%H-%M-%S"));
		// 构造保存文件对话框   
		CFileDialog fileDlg(FALSE, _T("txt"), str, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);


		// 显示保存文件对话框   
		if (IDOK == fileDlg.DoModal())
		{
			// 如果点击了文件对话框上的“保存”按钮，则将选择的文件路径显示到编辑框里   
			strFilePath = fileDlg.GetPathName();
			setFileName = fileDlg.GetFileName();

			//SetDlgItemText(IDC_FILENAMEINFOSHOW, setFileName);

			//TODO 默认先将已有文件名删除一遍
			remove(strFilePath);
			//fpWrite = fopen(setFileName, "w+");
			err = fopen_s(&fpWrite, strFilePath, "w");
			if (err == 0)
			{
				for (int row = 0; row < AsyLength; row++)
				{
					for (int col = 0; col < AsyWidth; col++)
					{
						fprintf(fpWrite, "|%-4.3f||%-4.3f||%-4.3f|\n",
							Ast_surfaceBuffer[row][col].x, Ast_surfaceBuffer[row][col].y, Ast_surfaceBuffer[row][col].z);
					}
				}
			}
			else
			{
				SetDlgItemText(IDC_SYSINFOSHOW, "错误文件名\n");
			}
			fclose(fpWrite);
			SetDlgItemText(IDC_SYSINFOSHOW, "输出数据已完成\n");

			AsyncStrTemp.Format(_T("---保存到文件成功,文件路径为: " + strFilePath + "---\r\n\r\n"));
			AsyncStrOut += AsyncStrTemp;

			SetDlgItemText(IDC_ASYNCSTATIC, AsyncStrOut);

			//m_logdlg->INFO(_T("保存到文件成功,文件路径为: " + strFilePath + "--INFO消息"));

		}
		else
		{
			//m_logdlg->WARN(_T("无效的目录，请重新选择---WARN消息"));
			MessageBox(_T("无效的目录，请重新选择---WARN消息"), _T("3D相机管理系统-GUI"), MB_OK);
		}
	}
	else
	{
		MessageBox(_T("无效的保存文件\r\n数据还未生成"), _T("3D相机管理系统-GUI"), MB_OK | MB_ICONEXCLAMATION);
	}
}