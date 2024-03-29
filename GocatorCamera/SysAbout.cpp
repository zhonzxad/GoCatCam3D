// SysAbout.cpp: 实现文件
//

#include "stdafx.h"
#include "GocatorCamera.h"
#include "SysAbout.h"
#include "afxdialogex.h"

#include "showallinfo.h"


// SysAbout 对话框
#pragma warning(disable:26495)

IMPLEMENT_DYNAMIC(SysAbout, CDialogEx)

SysAbout::SysAbout(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ABOUTBOX, pParent)
{

}

SysAbout::~SysAbout()
{
}

void SysAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(SysAbout, CDialogEx)
	ON_BN_CLICKED(IDC_SHOWALLINFO_BTN, &SysAbout::OnBnClickedShowallinfoBtn)
END_MESSAGE_MAP()


// SysAbout 消息处理程序

void SysAbout::OnBnClickedShowallinfoBtn()
{
	// TODO: 在此添加控件通知处理程序代码

	//非模态界面
	ShowAllInfo *pDlg = new ShowAllInfo;
	pDlg->Create(IDD_ALlINFODLG, this);
	pDlg->ShowWindow(SW_SHOW);

}

BOOL SysAbout::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//TODO 设置版本信息
	//IDC_EDITION_STA  GocatorCamera，版本 1.8
	SetDlgItemText(IDC_STATE_STA, _T("GocatorCamera，版本 1.9.4"));

	return TRUE;
}
