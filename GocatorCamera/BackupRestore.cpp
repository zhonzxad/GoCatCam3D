// BackupRestore.cpp: 实现文件
//
/*
* BackupRestore.c
*
* Gocator 2000 Sample
* Copyright (C) 2011-2018 by LMI Technologies Inc.
*
* Licensed under The MIT License.
* Redistributions of files must retain the above copyright notice.
*
* Purpose: Backup a Gocator system and restore it
*
*/

#include "stdafx.h"
#include "GocatorCamera.h"
#include "BackupRestore.h"
#include "afxdialogex.h"
#include <GoSdk/GoSdk.h>
#include <stdio.h>

#pragma warning(disable:4002)
#pragma warning(disable:26495)

extern CString BackUpStrOut = NULL;
extern CString BackUpStrTemp = NULL;

// BackupRestore 对话框

IMPLEMENT_DYNAMIC(BackupRestore, CDialogEx)

BackupRestore::BackupRestore(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BACKUPRESTORE, pParent)
{

}

BackupRestore::~BackupRestore()
{
}

void BackupRestore::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BACKUPRESTOREEDIT, m_backEdit);
}


BEGIN_MESSAGE_MAP(BackupRestore, CDialogEx)
	ON_BN_CLICKED(IDOK, &BackupRestore::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BACK_REFLESH, &BackupRestore::OnBnClickedBackReflesh)
END_MESSAGE_MAP()


// BackupRestore 消息处理程序

#define SENSOR_IP           "192.168.1.10"                      

void BackupRestore::SetUseBackupRestore()  // int argc, char **argv
{
	BackUpStrOut = (_T("进入备份传感器配置数据中（以bin后缀生产数据）:\r\n"));
	SetDlgItemText(IDC_BACKUPRESTOREEDIT, BackUpStrOut);
	m_backEdit.SetSel(-1, 0, FALSE);
	m_backEdit.SetFocus();

	//IDC_BACKUPRESTOREEDIT

	// construct Gocator API Library
	if ((status = GoSdk_Construct(&api)) != kOK)
	{
		BackUpStrTemp.Format(_T("Error: GoSdk_Construct:%d\r\n"), status);
		BackUpStrOut += BackUpStrTemp;
		SetDlgItemText(IDC_SURFACEEDIT, BackUpStrOut);
		return;
	}

	// construct GoSystem object
	if ((status = GoSystem_Construct(&Gosystem, kNULL)) != kOK)
	{
		BackUpStrTemp.Format(_T("Error: GoSystem_Construct:%d\r\n"), status);
		BackUpStrOut += BackUpStrTemp;
		SetDlgItemText(IDC_BACKUPRESTOREEDIT, BackUpStrOut);
		return;
	}

	// Parse IP address into address data structure
	kIpAddress_Parse(&ipAddress, SENSOR_IP);

	// obtain GoSensor object by sensor IP address
	if ((status = GoSystem_FindSensorByIpAddress(Gosystem, &ipAddress, &sensor)) != kOK)
	{
		BackUpStrTemp.Format(_T("Error: GoSystem_FindSensorByIpAddress:%d\r\n"), status);
		BackUpStrOut += BackUpStrTemp;
		SetDlgItemText(IDC_BACKUPRESTOREEDIT, BackUpStrOut);
		return;
	}

	// create connection to GoSensor object
	if ((status = GoSensor_Connect(sensor)) != kOK)
	{
		BackUpStrTemp.Format(_T("Error: GoSensor_Connect:%d\r\n"), status);
		BackUpStrOut += BackUpStrTemp;
		SetDlgItemText(IDC_BACKUPRESTOREEDIT, BackUpStrOut);
		return;
	}

	BackUpStrTemp.Format(_T("Creating backup file:%s\r\n\r\n"), backupFile);
	BackUpStrOut += BackUpStrTemp;
	BackUpStrTemp.Format(_T("------------------------\r\n"));
	BackUpStrOut += BackUpStrTemp;
	SetDlgItemText(IDC_BACKUPRESTOREEDIT, BackUpStrOut);

	/*m_logdlg->WARN(_T("Creating backup file:%s\n", backupFile));
	m_logdlg->WARN(_T("-------------------\n"));*/

	if ((status = GoSensor_Backup(sensor, backupFile)) != kOK)
	{
		BackUpStrTemp.Format(_T("Error: GoSystem_Backup:%d\r\n"), status);
		BackUpStrOut += BackUpStrTemp;
		SetDlgItemText(IDC_BACKUPRESTOREEDIT, BackUpStrOut);
		return;
	}

	BackUpStrTemp.Format(_T("Restoring from  backup file: %s\r\n"), backupFile);
	BackUpStrOut += BackUpStrTemp;
	BackUpStrTemp.Format(_T("-------------------\r\n\r\n"));
	BackUpStrOut += BackUpStrTemp;
	SetDlgItemText(IDC_BACKUPRESTOREEDIT, BackUpStrOut);

	/*m_logdlg->WARN(_T("Restoring from  backup file:%s\n", backupFile));
	m_logdlg->WARN(_T("-------------------\n"));*/

	if ((status = GoSensor_Restore(sensor, backupFile)) != kOK)
	{
		BackUpStrTemp.Format(_T("Error: GoSystem_Restore:%d\r\n"), status);
		BackUpStrOut += BackUpStrTemp;
		SetDlgItemText(IDC_BACKUPRESTOREEDIT, BackUpStrOut);
		return;
	}

	m_backEdit.SetSel(-1, 0, FALSE);
	m_backEdit.SetFocus();

	// destroy handles
	GoDestroy(Gosystem);
	GoDestroy(api);

	BackUpStrTemp.Format(_T("备份配置文件以生成完毕，生成到本项目文件夹\r\n"));
	BackUpStrOut += BackUpStrTemp;
	BackUpStrTemp.Format(_T("并以新的备份操作操作重启相机\r\n"));
	BackUpStrOut += BackUpStrTemp;
	SetDlgItemText(IDC_BACKUPRESTOREEDIT, BackUpStrOut);
	//getchar();
	m_backEdit.SetSel(-1, 0, FALSE);
	m_backEdit.SetFocus();

	return;
}

void BackupRestore::SetStopCamera()
{
	// stop Gocator sensor
	//废弃的操作

}

void BackupRestore::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	MessageBox(_T("无效确认操作！\r\n"), _T("GoCator相机操作"), MB_OK | MB_ICONEXCLAMATION);
}


void BackupRestore::OnBnClickedBackReflesh()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(_T("刷新相机需要重新创建配置\r\n期间请不要移动界面\r\n等待相机完成操作\r\n等待2到3秒"), _T("GoCator相机提示"), MB_OK);

	BackUpStrTemp.Format(_T("等待进行相机重连.....\r\n"));
	BackUpStrOut += BackUpStrTemp;
	SetDlgItemText(IDC_BACKUPRESTOREEDIT, BackUpStrOut);

	SetUseBackupRestore();

}
