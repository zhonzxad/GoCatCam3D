// ReceiveDataSurface.cpp: 实现文件
//
// ReceiveDataSurface 消息处理程序
/*
 * ReceiveSurface.c
 *
 * Gocator 2000 Sample
 * Copyright (C) 2011-2018 by LMI Technologies Inc.
 *
 * Licensed under The MIT License.
 * Redistributions of files must retain the above copyright notice.
 *
 * Purpose: Connect to Gocator system and receive Surface data and translate to engineering units. Gocator must be in Surface Mmode.
 * Ethernet output for the whole part and/or intensity data must be enabled.
 *
 */

#include "stdafx.h"
#include "GocatorCamera.h"
#include "ReceiveDataSurface.h"
#include "afxdialogex.h"

#pragma warning(disable:4002)
#pragma warning(disable:4244)
#pragma warning(disable:6273)
#pragma warning(disable:6386)
#pragma warning(disable:6387)
#pragma warning(disable:6011)
#pragma warning(disable:26495)

// ReceiveDataSurface 对话框

IMPLEMENT_DYNAMIC(ReceiveDataSurface, CDialogEx)

ReceiveDataSurface::ReceiveDataSurface(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RECEIVESURFACE, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

ReceiveDataSurface::~ReceiveDataSurface()
{
}

void ReceiveDataSurface::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SURFACEEDIT, m_surEdit);
}


BEGIN_MESSAGE_MAP(ReceiveDataSurface, CDialogEx)
	ON_BN_CLICKED(IDOK, &ReceiveDataSurface::OnBnClickedIDOK)
	ON_BN_CLICKED(IDC_SURF_REFLESH, &ReceiveDataSurface::OnBnClickedSurfReflesh)
END_MESSAGE_MAP()


#include <GoSdk/GoSdk.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define NUM_PROFILES            2
#define RECEIVE_TIMEOUT			20000000
#define INVALID_RANGE_16BIT     ((signed short)0x8000)          // gocator transmits range data as 16-bit signed integers. 0x8000 signifies invalid range data. 
#define DOUBLE_MAX              ((k64f)1.7976931348623157e+308) // 64-bit double - largest positive value.  
#define INVALID_RANGE_DOUBLE    ((k64f)-DOUBLE_MAX)             // floating point value to represent invalid range data.
#define SENSOR_IP               "192.168.1.10"  

#define NM_TO_MM(VALUE) (((k64f)(VALUE))/1000000.0)
#define UM_TO_MM(VALUE) (((k64f)(VALUE))/1000.0)

typedef struct
{
	double x;   // x-coordinate in engineering units (mm) - position along laser line
	double y;   // y-coordinate in engineering units (mm) - position along the direction of travel
	double z;   // z-coordinate in engineering units (mm) - height (at the given x position)
	//unsigned char intensity;
	char intensity;
}ProfilePoint;

short int* Surf_height_map_memory = NULL;
unsigned char* Surf_intensity_image_memory = NULL;
ProfilePoint** Surf_surfaceBuffer;
unsigned int Surf_Length, Surf_Width;
k32u Surf_surfaceBufferHeight = 0;


void ReceiveDataSurface::SetUseReceiveSurfaceFun() //int argc, char **argv
{
	//GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行接收表面数据程序----------------------------"));
	
	// construct Gocator API Library
	SurfaceStrOut = "初始化进入采集表面数据程序\r\n";
	SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);
	m_surEdit.SetSel(-1, 0, FALSE);
	m_surEdit.SetFocus();

	m_surEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	// construct Gocator API Library
	if ((status = GoSdk_Construct(&api)) != kOK)
	{
		SurfaceStrTemp.Format(_T("Error: GoSdk_Construct:%d\r\n"), status);
		SurfaceStrOut += SurfaceStrTemp;
		SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);
		return ;
	}

	// construct GoSystem object
	if ((status = GoSystem_Construct(&Gosystem, kNULL)) != kOK)
	{
		SurfaceStrTemp.Format(_T("Error: GoSystem_Construct:%d\n"), status);
		SurfaceStrOut += SurfaceStrTemp;
		SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);
		return;
	}

	// Parse IP address into address data structure
	kIpAddress_Parse(&ipAddress, SENSOR_IP);
	SurfaceStrTemp.Format(_T("已通过正确设置访问到传感器编号： %d\r\n"), SENSOR_IP);
	SurfaceStrOut += SurfaceStrTemp;
	SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);

	// obtain GoSensor object by sensor IP address
	if ((status = GoSystem_FindSensorByIpAddress(Gosystem, &ipAddress, &sensor)) != kOK)
	{
		SurfaceStrTemp.Format(_T("Error: GoSystem_FindSensor:%d\r\n"), status);
		SurfaceStrOut += SurfaceStrTemp;
		SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);
		return;
	}

	// create connection to GoSensor object
	if ((status = GoSensor_Connect(sensor)) != kOK)
	{
		SurfaceStrTemp.Format(_T("Error: GoSensor_Connect:%d\r\n"), status);
		SurfaceStrOut += SurfaceStrTemp;
		SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);
		return;
	}

	// enable sensor data channel
	if ((status = GoSystem_EnableData(Gosystem, kTRUE)) != kOK)
	{
		SurfaceStrTemp.Format(_T("Error: GoSensor_EnableData:%d\r\n"), status);
		SurfaceStrOut += SurfaceStrTemp;
		SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);
		return;
	}

	// start Gocator sensor
	if ((status = GoSystem_Start(Gosystem)) != kOK)
	{
		SurfaceStrTemp.Format(_T("Error: GoSystem_Start:%d\r\n"), status);
		SurfaceStrOut += SurfaceStrTemp;
		SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);
		return;
	}

	SurfaceStrTemp.Format(_T("Waiting for Whole Part data...\r\n\r\n"));
	SurfaceStrOut += SurfaceStrTemp;
	SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);

	int n = 0;
	while (n < NUM_PROFILES)
	{
		if (GoSystem_ReceiveData(Gosystem, &dataset, RECEIVE_TIMEOUT) == kOK)
		{

			SurfaceStrTemp.Format(_T("Dataset count: %u\r\n"), GoDataSet_Count(dataset));
			SurfaceStrOut += SurfaceStrTemp;
			SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);

			// each result can have multiple data items
			// loop through all items in result message
			for (i = 0; i < GoDataSet_Count(dataset); ++i)
			{
				GoDataMsg dataObj = GoDataSet_At(dataset, i);

				switch (GoDataMsg_Type(dataObj))
				{
				case GO_DATA_MESSAGE_TYPE_STAMP:
				{
					GoStampMsg stampMsg = dataObj;

					SurfaceStrTemp.Format(_T("   Stamp Message batch count: %u\r\n\r\n"), GoStampMsg_Count(stampMsg));
					SurfaceStrOut += SurfaceStrTemp;
					SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);

					for (j = 0; j < GoStampMsg_Count(stampMsg); j++)
					{
						GoStamp* stamp = GoStampMsg_At(stampMsg, j);
						SurfaceStrTemp.Format(_T("  Timestamp: %llu\r\n"), stamp->timestamp);
						SurfaceStrOut += SurfaceStrTemp;
						SurfaceStrTemp.Format(_T("  Encoder position at leading edge: %lld\r\n"), stamp->encoder);
						SurfaceStrOut += SurfaceStrTemp;;
						SurfaceStrTemp.Format(_T("  Frame index: %llu\r\n"), stamp->frameIndex);
						SurfaceStrOut += SurfaceStrTemp;
						SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);
					}
				}
				break;
				case GO_DATA_MESSAGE_TYPE_SURFACE_INTENSITY:
				{
					GoSurfaceIntensityMsg surfaceIntMsg = dataObj;
					unsigned int rowIdx, colIdx;
					double XResolution = NM_TO_MM(GoSurfaceIntensityMsg_XResolution(surfaceIntMsg));
					double YResolution = NM_TO_MM(GoSurfaceIntensityMsg_YResolution(surfaceIntMsg));
					double XOffset = UM_TO_MM(GoSurfaceIntensityMsg_XOffset(surfaceIntMsg));
					double YOffset = UM_TO_MM(GoSurfaceIntensityMsg_YOffset(surfaceIntMsg));

					SurfaceStrTemp.Format(_T("  Surface intensity width: %ld\r\n"),
						GoSurfaceIntensityMsg_Width(surfaceIntMsg));
					SurfaceStrOut += SurfaceStrTemp;
					SurfaceStrTemp.Format(_T("  Surface intensity height: %ld\r\n"),
						GoSurfaceIntensityMsg_Length(surfaceIntMsg));
					SurfaceStrOut += SurfaceStrTemp;
					SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);

					/*m_logdlg->WARN(_T("  Surface intensity width: %ld\n", GoSurfaceIntensityMsg_Width(surfaceIntMsg)));
					m_logdlg->WARN(_T("  Surface intensity height: %ld\n", GoSurfaceIntensityMsg_Length(surfaceIntMsg)));*/

					//allocate memory if needed
					if (Surf_surfaceBuffer == NULL)
					{
						Surf_surfaceBuffer = (ProfilePoint * *)malloc(GoSurfaceIntensityMsg_Length(surfaceIntMsg) * sizeof(ProfilePoint*));

						for (j = 0; j < GoSurfaceIntensityMsg_Length(surfaceIntMsg); j++)
						{
							Surf_surfaceBuffer[j] = (ProfilePoint*)malloc(GoSurfaceIntensityMsg_Width(surfaceIntMsg) * sizeof(ProfilePoint));
						}

						Surf_surfaceBufferHeight = GoSurfaceIntensityMsg_Length(surfaceIntMsg);
					}

					for (rowIdx = 0; rowIdx < GoSurfaceIntensityMsg_Length(surfaceIntMsg); rowIdx++)
					{
						k8u* data = GoSurfaceIntensityMsg_RowAt(surfaceIntMsg, rowIdx);

						// gocator transmits intensity data as an 8-bit grayscale image of identical width and height as the corresponding height map
						for (colIdx = 0; colIdx < GoSurfaceIntensityMsg_Width(surfaceIntMsg); colIdx++)
						{
							Surf_surfaceBuffer[rowIdx][colIdx].x = XOffset + XResolution * colIdx;
							Surf_surfaceBuffer[rowIdx][colIdx].y = YOffset + YResolution * rowIdx;
							Surf_surfaceBuffer[rowIdx][colIdx].intensity = data[colIdx];
						}
					}
					Surf_Length = GoSurfaceIntensityMsg_Length(surfaceIntMsg);
					Surf_Width = GoSurfaceIntensityMsg_Width(surfaceIntMsg);
				}
				break;
				}
			}

			m_surEdit.SetSel(-1, 0, FALSE);
			m_surEdit.SetFocus();

			m_surEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

			GoDestroy(dataset);
			
		}
		else
		{
			SurfaceStrTemp.Format(_T("Error: No data received during the waiting period\r\n"));
			SurfaceStrOut += SurfaceStrTemp;
			SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);
		}
		n++;
	}

	// stop Gocator sensor
	if ((status = GoSystem_Stop(Gosystem)) != kOK)
	{
		SurfaceStrTemp.Format(_T("Error: GoSystem_Stop:%d\r\n"), status);
		SurfaceStrOut += SurfaceStrTemp;
		SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);
		return;
	}

	// destroy handles  
	GoDestroy(Gosystem);
	GoDestroy(api);

	m_surEdit.SetSel(-1, 0, FALSE);
	m_surEdit.SetFocus();

	return;
}

void ReceiveDataSurface::SetStopCamera()
{
	// stop Gocator sensor
	if ((status = GoSystem_Stop(Gosystem)) != kOK)
	{
		SurfaceStrTemp.Format(_T("Error: GoSystem_Stop:%d\r\n"), status);
		SurfaceStrOut += SurfaceStrTemp;
		SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);
		return;
	}

	// destroy handles  
	GoDestroy(Gosystem);
	GoDestroy(api);

	SurfaceStrTemp.Format(_T("接收表面数据功能结束，传感器复位，为了减轻CPU压力，请2至3秒后在操作\r\n\r\n"));
	SurfaceStrOut += SurfaceStrTemp;
	SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);

	m_surEdit.SetSel(-1, 0, FALSE);
	m_surEdit.SetFocus();

	m_surEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

}

void ReceiveDataSurface::OnBnClickedIDOK()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	MessageBox(_T("无效确认操作！\r\n"), _T("GoCator相机操作"), MB_OK | MB_ICONEXCLAMATION);
}


void ReceiveDataSurface::OnBnClickedSurfReflesh()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(_T("刷新相机需要重新创建配置\r\n期间请不要移动界面\r\n等待相机完成操作\r\n等待2到3秒"), _T("GoCator相机提示"), MB_OK);

	SurfaceStrTemp.Format(_T("等待相机进行重连操作.....\r\n"));
	SurfaceStrOut += SurfaceStrTemp;
	SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);

	m_surEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	SetUseReceiveSurfaceFun();
	//SetStopCamera();
}

void ReceiveDataSurface::ToSavaIntensData()
{
	if (Surf_surfaceBuffer != NULL)
	{
		m_surEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

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
			err = fopen_s(&fpWrite, strFilePath, "w+");
			if (err == 0)
			{
				for (unsigned int row = 0; row < Surf_Length; row++)
				{
					for (unsigned int col = 0; col < Surf_Width; col++)
					{
						fprintf(fpWrite, "|%-4.3f||%-4.3f||%c|\n",
							Surf_surfaceBuffer[row][col].x, Surf_surfaceBuffer[row][col].y, Surf_surfaceBuffer[row][col].intensity);
					}
				}

			}
			else
			{
				SetDlgItemText(IDC_SYSINFOSHOW, "错误文件名\n");
			}
			fclose(fpWrite);
			SetDlgItemText(IDC_SYSINFOSHOW, "输出数据已完成\n");

			SurfaceStrTemp.Format(_T("---保存到文件成功,文件路径为: " + strFilePath + "---\r\n\r\n"));
			SurfaceStrOut += SurfaceStrTemp;

			SetDlgItemText(IDC_SURFACEEDIT, SurfaceStrOut);

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

	////释放buffer缓存
	//if (Surf_surfaceBuffer)
	//{
	//	unsigned int i;
	//	for (i = 0; i < Surf_surfaceBufferHeight; i++)
	//	{
	//		free(Surf_surfaceBuffer[i]);
	//	}
	//}

}