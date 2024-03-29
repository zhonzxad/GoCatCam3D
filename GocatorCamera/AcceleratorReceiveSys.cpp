// AcceleratorReceiveSys.cpp: 实现文件
//

#include "stdafx.h"
#include "GocatorCamera.h"
#include "AcceleratorReceiveSys.h"
#include "afxdialogex.h"
#include <GoSdk/GoSdk.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

// AcceleratorReceiveSys 对话框
#pragma warning(disable:4002)
#pragma warning(disable:26495)

IMPLEMENT_DYNAMIC(AcceleratorReceiveSys, CDialogEx)

AcceleratorReceiveSys::AcceleratorReceiveSys(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ACCELERATORRECEIVESYS, pParent)
{

}

AcceleratorReceiveSys::~AcceleratorReceiveSys()
{
}

void AcceleratorReceiveSys::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ACCELERAEDIT, m_accEdit);
}


BEGIN_MESSAGE_MAP(AcceleratorReceiveSys, CDialogEx)
	ON_EN_UPDATE(IDC_ACCELERAEDIT, &AcceleratorReceiveSys::OnEnUpdateAcceleraedit)
	ON_BN_CLICKED(IDC_ACC_REFLESH, &AcceleratorReceiveSys::OnBnClickedAccReflesh)
	ON_BN_CLICKED(IDOK, &AcceleratorReceiveSys::OnBnClickedOk)
END_MESSAGE_MAP()


// AcceleratorReceiveSys 消息处理程序



#define RECEIVE_TIMEOUT         (20000000) 
#define INVALID_RANGE_16BIT     ((signed short)0x8000)          // gocator transmits range data as 16-bit signed integers. 0x8000 signifies invalid range data. 
#define DOUBLE_MAX              ((k64f)1.7976931348623157e+308) // 64-bit double - largest positive value.  
#define INVALID_RANGE_DOUBLE    ((k64f)-DOUBLE_MAX)             // floating point value to represent invalid range data.    
#define SENSOR_IP               "192.168.1.6"                      

#define NM_TO_MM(VALUE) (((k64f)(VALUE))/1000000.0)
#define UM_TO_MM(VALUE) (((k64f)(VALUE))/1000.0)

void AcceleratorReceiveSys::SetUseAcceleratorSys()  //int argc, char **argv
{
	AcceleStrOut = (_T("进入GoMax加速器连接系统:\r\n"));
	SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
	m_accEdit.SetSel(-1, 0, FALSE);
	m_accEdit.SetFocus();

	// construct Gocator API Library
	if ((status = GoSdk_Construct(&api)) != kOK)
	{
		AcceleStrTemp.Format(_T("Error: GoSdk_Construct:%d\r\n"), status);
		AcceleStrOut += AcceleStrTemp;
		SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
		return;
	}

	AcceleStrTemp.Format(_T("正在寻找局域网中的加速器....\r\n\r\n"));
	AcceleStrOut += AcceleStrTemp;
	SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);

	// construct GoSystem object
	if ((status = GoSystem_Construct(&Gosystem, kNULL)) != kOK)
	{
		AcceleStrTemp.Format(_T("Error: GoSystem_Construct:%d\r\n"), status);
		AcceleStrOut += AcceleStrTemp;
		SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
		return;
	}

	// construct GoAccelerator object
	if ((status = GoAccelerator_Construct(&accelerator, kNULL)) != kOK)
	{
		AcceleStrTemp.Format(_T("Error: GoAccelerator_Construct:%d\r\n"), status);
		//AcceleStrTemp.Format(_T("未找到适合IP地址的加速器传感器！！！\r\n"));
		AcceleStrOut += AcceleStrTemp;
		SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
		return;
	}

	// start Accelerator service
	if ((status = GoAccelerator_Start(accelerator)) != kOK)
	{
		AcceleStrTemp.Format(_T("Error: GoAccelerator_Start:%d\r\n"), status);
		AcceleStrOut += AcceleStrTemp;
		SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
		return;
	}

	// Parse IP address into address data structure
	kIpAddress_Parse(&ipAddress, SENSOR_IP);

	// obtain GoSensor object by sensor IP address
	if ((status = GoSystem_FindSensorByIpAddress(Gosystem, &ipAddress, &sensor)) != kOK)
	{
		//AcceleStrTemp.Format(_T("Error: GoSystem_FindSensorByIpAddress:%d\r\n", status));
		AcceleStrTemp.Format(_T("无效的IP地址查找。\r\n"));
		AcceleStrOut += AcceleStrTemp;
		AcceleStrTemp.Format(_T("当前网络中没有存在加速器系统。\r\n"));
		AcceleStrOut += AcceleStrTemp;
		SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
		return;
	}

	// attach accelerator
	if ((status = GoAccelerator_Attach(accelerator, sensor)) != kOK)
	{
		AcceleStrTemp.Format(_T("Error: GoAccelerator_Attach:%d\r\n"), status);
		AcceleStrOut += AcceleStrTemp;
		SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
		return;
	}

	// create connection to GoSensor object
	if ((status = GoSensor_Connect(sensor)) != kOK)
	{
		AcceleStrTemp.Format(_T("Error: GoSensor_Connect:%d\r\n"), status);
		AcceleStrOut += AcceleStrTemp;
		SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
		return;
	}

	// enable sensor data channel
	if ((status = GoSystem_EnableData(Gosystem, kTRUE)) != kOK)
	{
		AcceleStrTemp.Format(_T("Error: GoSensor_EnableData:%d\r\n"), status);
		AcceleStrOut += AcceleStrTemp;
		SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
		return;
	}

	// start Gocator sensor
	if ((status = GoSystem_Start(Gosystem)) != kOK)
	{
		AcceleStrTemp.Format(_T("Error: GoSystem_Start:%d\r\n"), status);
		AcceleStrOut += AcceleStrTemp;
		SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
		return;
	}

	if (GoSystem_ReceiveData(Gosystem, &dataset, RECEIVE_TIMEOUT) == kOK)
	{
		AcceleStrTemp.Format(_T("Data message received:\r\n"));
		AcceleStrOut += AcceleStrTemp;
		AcceleStrOut.Format(_T("Dataset count: %u\r\n"), GoDataSet_Count(dataset));
		AcceleStrOut += AcceleStrTemp;
		SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);

		// each result can have multiple data items
		// loop through all items in result message
		for (i = 0; i < GoDataSet_Count(dataset); ++i)
		{
			dataObj = GoDataSet_At(dataset, i);
			//Retrieve GoStamp message
			switch (GoDataMsg_Type(dataObj))
			{
			case GO_DATA_MESSAGE_TYPE_STAMP:
			{
				GoStampMsg stampMsg = dataObj;

				AcceleStrTemp.Format(_T("Stamp Message batch count: %u\r\n"), GoStampMsg_Count(stampMsg));
				AcceleStrOut += AcceleStrTemp;
				SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);

				for (j = 0; j < GoStampMsg_Count(stampMsg); ++j)
				{
					stamp = GoStampMsg_At(stampMsg, j);

					AcceleStrTemp.Format(_T("  Timestamp: %llu\r\n"), stamp->timestamp);
					AcceleStrOut += AcceleStrTemp;
					AcceleStrTemp.Format(_T("  Encoder: %lld\r\n"), stamp->encoder);
					AcceleStrOut += AcceleStrTemp;
					AcceleStrTemp.Format(_T("  Frame index: %llu\r\n"), stamp->frameIndex);
					AcceleStrOut += AcceleStrTemp;
					SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);

				}
			}
			break;
			case GO_DATA_MESSAGE_TYPE_MEASUREMENT:
			{
				GoMeasurementMsg measurementMsg = dataObj;

				AcceleStrTemp.Format(_T("Measurement Message batch count: %u\r\n"), GoMeasurementMsg_Count(measurementMsg));
				AcceleStrOut += AcceleStrTemp;
				SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);

				for (k = 0; k < GoMeasurementMsg_Count(measurementMsg); ++k)
				{
					measurementData = GoMeasurementMsg_At(measurementMsg, k);
					AcceleStrTemp.Format(_T("Measurement ID: %u\r\n"), GoMeasurementMsg_Id(measurementMsg));
					AcceleStrOut += AcceleStrTemp;
					AcceleStrTemp.Format(_T("Measurement Value: %.1f\r\n"), measurementData->value);
					AcceleStrOut += AcceleStrTemp;
					AcceleStrTemp.Format(_T("Measurement Decision: %d\r\n"), measurementData->decision);
					AcceleStrOut += AcceleStrTemp;
					SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);

				}
			}
			break;
			}
		}
		GoDestroy(dataset);
	}
	else
	{
		AcceleStrTemp.Format(_T("Error: No data received during the waiting period\r\n"));
		AcceleStrOut += AcceleStrTemp;
		SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
		//m_logdlg->WARN(_T("Error: No data received during the waiting period\n"));
	}

	// stop Gocator sensor
	if ((status = GoSystem_Stop(Gosystem)) != kOK)
	{
		AcceleStrTemp.Format(_T("Error: GoSystem_Stop:%d\r\n"), status);
		AcceleStrOut += AcceleStrTemp;
		SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
		return;
	}

	m_accEdit.SetSel(-1, 0, FALSE);
	m_accEdit.SetFocus();

	//// stop Accelerator service
	//if ((status = GoAccelerator_Stop(accelerator)) != kOK)
	//{
	//	AcceleStrTemp.Format(_T("Error: GoAccelerator_Start:%d\n", status));
	//	AcceleStrOut += AcceleStrTemp;
	//	SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
	//	return;
	//}

	//// destroy handles
	//GoDestroy(accelerator);
	//GoDestroy(Gosystem);
	//GoDestroy(api);

	
	//getchar();
	return;
}

void AcceleratorReceiveSys::SetStopCamera()
{
	// stop Accelerator service
	if ((status = GoAccelerator_Stop(accelerator)) != kOK)
	{
		AcceleStrTemp.Format(_T("Error: GoAccelerator_Start:%d\r\n"), status);
		AcceleStrOut += AcceleStrTemp;
		SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);
		return;
	}

	// destroy handles
	GoDestroy(accelerator);
	GoDestroy(Gosystem);
	GoDestroy(api);

	AcceleStrTemp.Format(_T("传感器链接结束，销毁此帧对象。\r\n"));
	AcceleStrOut += AcceleStrTemp;
	SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);

	m_accEdit.SetSel(-1, 0, FALSE);
	m_accEdit.SetFocus();
}


void AcceleratorReceiveSys::OnEnUpdateAcceleraedit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数，以将 EM_SETEVENTMASK 消息发送到该控件，
	// 同时将 ENM_UPDATE 标志“或”运算到 lParam 掩码中。

	// TODO:  在此添加控件通知处理程序代码

	m_accEdit.SetSel(-1, 0, FALSE);
	m_accEdit.SetFocus();
}


void AcceleratorReceiveSys::OnBnClickedAccReflesh()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(_T("刷新相机需要重新创建配置\r\n期间请不要移动界面\r\n等待相机完成操作\r\n等待2到3秒"), _T("GoCator相机提示"), MB_OK );

	AcceleStrTemp.Format(_T("等待进行相机重连.....\r\n"));
	AcceleStrOut += AcceleStrTemp;
	SetDlgItemText(IDC_ACCELERAEDIT, AcceleStrOut);

	SetUseAcceleratorSys();
	SetStopCamera();
}


void AcceleratorReceiveSys::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	MessageBox(_T("无效确认操作！\r\n"), _T("GoCator相机操作"), MB_OK | MB_ICONEXCLAMATION);
}
