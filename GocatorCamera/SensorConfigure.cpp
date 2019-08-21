// SensorConfigure.cpp: 实现文件
//

#include "stdafx.h"
#include "GocatorCamera.h"
#include "SensorConfigure.h"
#include "afxdialogex.h"

#pragma warning(disable:4002)
// SensorConfigure 对话框

IMPLEMENT_DYNAMIC(SensorConfigure, CDialogEx)

SensorConfigure::SensorConfigure(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CONFIGUREDLG, pParent)
{

}

SensorConfigure::~SensorConfigure()
{
}

void SensorConfigure::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONFIGUREEDIT, m_conEdit);
}


BEGIN_MESSAGE_MAP(SensorConfigure, CDialogEx)
	ON_BN_CLICKED(IDOK, &SensorConfigure::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CONF_REFLESH, &SensorConfigure::OnBnClickedConfReflesh)
END_MESSAGE_MAP()


// SensorConfigure 消息处理程序
#include <GoSdk/GoSdk.h>
#include <stdio.h>

#define SENSOR_IP           "192.168.1.10"   // serial number of the sensor used for sensor connection GoSystem_FindSensor() call.

void SensorConfigure::SetCameraConfigure()  //int argc, char **argv
{
	kStatus status;
	kAssembly api = kNULL;
	GoSystem system = kNULL;
	GoSensor sensor = kNULL;
	GoSetup setup = kNULL;
	k64f currentExposure;
	k64f newExposure;
	kIpAddress ipAddress;

	m_conEdit.SetSel(-1, 0, FALSE);
	m_conEdit.SetFocus();

	// construct Gocator API Library
	ConfiStrOut = "初始化进入保存环境配置程序\r\n";
	SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);

	if ((status = GoSdk_Construct(&api)) != kOK)
	{
		ConfiStrOut.Format(_T("Error: GoSdk_Construct:%d\r\n"), status);
		SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);
		return;
	}

	// construct GoSystem object
	if ((status = GoSystem_Construct(&system, kNULL)) != kOK)
	{
		ConfiStrOut.Format(_T("Error: GoSystem_Construct:%d\r\n"), status);
		SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);
		return;
	}

	// Parse IP address into address data structure
	kIpAddress_Parse(&ipAddress, SENSOR_IP);
	ConfiStrOut = (_T("已经绑定连接到相机"));
	SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);

	// obtain GoSensor object by IP address
	if ((status = GoSystem_FindSensorByIpAddress(system, &ipAddress, &sensor)) != kOK)
	{
		ConfiStrOut.Format(_T("Error: GoSystem_FindSensor:%d\r\n"), status);
		SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);
		return;
	}

	// create connection to GoSensor object
	if ((status = GoSensor_Connect(sensor)) != kOK)
	{
		ConfiStrOut.Format(_T("Error: GoSensor_Connect:%d\r\n"), status);
		SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);
		return;
	}

	// retrieve setup handle
	if ((setup = GoSensor_Setup(sensor)) == kNULL)
	{
		ConfiStrOut = (_T("Error: GoSensor_Setup: Invalid Handle\r\n"));
		SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);
		return;
	}

	if ((status = GoSensor_CopyFile(sensor, "_live.job", "oldExposure.job")) != kOK)
	{
		ConfiStrOut.Format(_T("Error: GoSensor_CopyFile:%d\r\n"), status);
		SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);
		return;
	}

	// read current parameters
	currentExposure = GoSetup_Exposure(setup, GO_ROLE_MAIN);
	ConfiStrTemp.Format(_T("Current Parameters:\r\n"));
	ConfiStrOut += ConfiStrTemp;
	ConfiStrTemp.Format(_T("-------------------\r\n"));
	ConfiStrOut += ConfiStrTemp;
	ConfiStrTemp.Format(_T("Exposure:%f us\r\r\n\n"), currentExposure);
	ConfiStrOut += ConfiStrTemp;
	SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);
	m_conEdit.SetSel(-1, 0, FALSE);
	m_conEdit.SetFocus();

	// modify parameter in main sensor
	if ((status = GoSetup_SetExposure(setup, GO_ROLE_MAIN, currentExposure + 200)) != kOK)
	{
		ConfiStrOut.Format(_T("Error: GoSetup_SetExposure:%d\r\n"), status);
		SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);
		return;
	}

	// GoSensorFlush() - immediately synchronizes configuration changes to the sensor
	// *The changes will be shown on the web browser GUI after the browser has been refreshed.
	// NOTE: Sensor is not automatically synchronized with every call to function that modifies a setting.
	// This allows for rapid configuring sensors without delay caused by synchronization after every call.
	// Generally functions that retreieve setting values causes automatic synchronization while functions that set values don't.
	// Synchronization is also always guranteed prior to sensor entering running state. The GoSensor_Flush() function
	// should only be used when configuration changes are needed to be seen immediately.
	GoSensor_Flush(sensor);

	newExposure = GoSetup_Exposure(setup, GO_ROLE_MAIN);
	ConfiStrTemp.Format(_T("New Parameters:\r\n"));
	ConfiStrOut += ConfiStrTemp;
	ConfiStrTemp.Format(_T("---------------\r\n"));
	ConfiStrOut += ConfiStrTemp;
	ConfiStrTemp.Format(_T("Exposure:%f us\r\r\n\n"), newExposure);
	ConfiStrOut += ConfiStrTemp;
	SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);
	m_conEdit.SetSel(-1, 0, FALSE);
	m_conEdit.SetFocus();

	// Save the configuration and template into a new file set. This is the same behavior
	// as if the user clicks the save button in the toolbar.

	if ((status = GoSensor_CopyFile(sensor, "_live.job", "newExposure.job")) != kOK)
	{
		ConfiStrOut.Format(_T("Error: GoSensor_CopyFile:%d\r\n"), status);
		SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);
		return;
	}

	// Set the saved configuration as default job. This makes it the active configuration when the sensor powers up.
	if ((status = GoSensor_SetDefaultJob(sensor, "newExposure.job")) != kOK)
	{
		ConfiStrOut.Format(_T("Error: GoSensor_SetDefaultJob:%d\r\n"), status);
		SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);
		return;
	}

	// Switches back to the original exposure
	if ((status = GoSensor_CopyFile(sensor, "oldExposure.job", "_live.job")) != kOK)
	{
		ConfiStrOut.Format(_T("Error: GoSensor_CopyFile:%d\r\n"), status);
		SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);
		return;
	}

	// destroy handles
	GoDestroy(system);
	GoDestroy(api);

	ConfiStrOut = (_T("------------------------------\r\n"));
	SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);
	//m_logdlg->INFO(_T("Press any key to continue...\n"));

	m_conEdit.SetSel(-1, 0, FALSE);
	m_conEdit.SetFocus();
	//getchar();

	return;
}

void SensorConfigure::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	MessageBox(_T("无效确认操作！\r\n"), _T("GoCator相机操作"), MB_OK | MB_ICONEXCLAMATION);
}


void SensorConfigure::OnBnClickedConfReflesh()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(_T("刷新相机需要重新创建配置\r\n期间请不要移动界面\r\n等待相机完成操作\r\n等待2到3秒"), _T("GoCator相机提示"), MB_OK);

	ConfiStrTemp.Format(_T("等待相机进行重连操作....\r\n"));
	ConfiStrOut += ConfiStrTemp; 
	SetDlgItemText(IDC_CONFIGUREEDIT, ConfiStrOut);

	SetCameraConfigure();
}
