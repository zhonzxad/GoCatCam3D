// SysStatus.cpp: 实现文件
//

#include "stdafx.h"
#include "GocatorCamera.h"
#include "SysStatus.h"
#include "afxdialogex.h"


// SysStatus 对话框

IMPLEMENT_DYNAMIC(SysStatus, CDialogEx)

SysStatus::SysStatus(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SYSSTATUS, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

SysStatus::~SysStatus()
{
}

void SysStatus::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SYSTEAM, m_SysStat);
	DDX_Control(pDX, IDC_STATISTICS, m_SysCount);
}

BOOL SysStatus::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// TODO: 在此添加额外的初始化代码

	//设置标题栏
	SetWindowText(_T("GoCator传感器状态和拍摄信息"));

	////m_SysCount.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES); // 整行选择、网格线
	m_SysStat.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
	m_SysStat.InsertColumn(0, _T("名称"), LVCFMT_LEFT, 110);
	m_SysStat.InsertColumn(1, _T("数据"), LVCFMT_LEFT, 137);

	m_SysCount.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
	m_SysCount.InsertColumn(0, _T("名称"), LVCFMT_LEFT, 80);
	m_SysCount.InsertColumn(1, _T("ID"), LVCFMT_LEFT, 30);
	m_SysCount.InsertColumn(2, _T("数值"), LVCFMT_LEFT, 45);
	m_SysCount.InsertColumn(3, _T("最小值"), LVCFMT_LEFT, 52);
	m_SysCount.InsertColumn(4, _T("最大值"), LVCFMT_LEFT, 52);
	m_SysCount.InsertColumn(5, _T("平均值"), LVCFMT_LEFT, 52);
	m_SysCount.InsertColumn(6, _T("范围"), LVCFMT_LEFT, 43);
	m_SysCount.InsertColumn(7, _T("标准偏差"), LVCFMT_LEFT, 60);
	m_SysCount.InsertColumn(8, _T("合格"), LVCFMT_LEFT, 43);
	m_SysCount.InsertColumn(9, _T("不合格"), LVCFMT_LEFT, 52);
	m_SysCount.InsertColumn(10, _T("无效"), LVCFMT_LEFT, 45);
	m_SysCount.InsertColumn(11, _T("溢出"), LVCFMT_LEFT, 45);



	//初始化系统信息
	ShowSysInfo();
	ShowSysCount();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

BEGIN_MESSAGE_MAP(SysStatus, CDialogEx)
	ON_BN_CLICKED(IDC_BUT_SURE, &SysStatus::OnBnClickedButSure)
	ON_BN_CLICKED(IDC_BUT_CLOSE, &SysStatus::OnBnClickedButClose)
END_MESSAGE_MAP()


void SysStatus::OnBnClickedButSure()
{
	// TODO: 在此添加控件通知处理程序代码
	//this->CloseWindow();
	CDialog::EndDialog(0); //结束对话框
}

void SysStatus::OnBnClickedButClose()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialog::EndDialog(0); //结束对话框
}

void SysStatus::PostNcDestroy()
{
	// TODO: 在此添加专用代码和/或调用基类
	delete this;

	CDialogEx::PostNcDestroy();
}

void SysStatus::ShowSysInfo()
{
	m_SysStat.InsertItem(0, _T("传感器状态"), 0); // 插入0行0列
	m_SysStat.SetItemText(0, 1, _T("就绪"));
	m_SysStat.InsertItem(1, _T("固件版本"), 0); // 插入1行0列
	m_SysStat.SetItemText(1, 1, _T("4.7.12.47"));
	m_SysStat.InsertItem(2, _T("激光安全"), 0);
	m_SysStat.SetItemText(2, 1, _T("开启"));
	m_SysStat.InsertItem(3, _T("开机时间"), 0);
	m_SysStat.SetItemText(3, 1, _T("Null"));
	m_SysStat.InsertItem(4, _T("CPU负载"), 0);
	m_SysStat.SetItemText(4, 1, _T("1%"));
	m_SysStat.InsertItem(5, _T("当前速度"), 0);
	m_SysStat.SetItemText(5, 1, _T("0/1093"));
	m_SysStat.InsertItem(6, _T("编码器数值"), 0);
	m_SysStat.SetItemText(6, 1, _T("0"));
	m_SysStat.InsertItem(7, _T("编码器频率"), 0);
	m_SysStat.SetItemText(7, 1, _T("0 Hz"));
	m_SysStat.InsertItem(8, _T("内存负载"), 0);
	m_SysStat.SetItemText(8, 1, _T("322.94/1024.00MB"));
	m_SysStat.InsertItem(9, _T("存储负载"), 0);
	m_SysStat.SetItemText(9, 1, _T("31.5/238.88MB"));
	m_SysStat.InsertItem(10, _T("以太网连接速度"), 0);
	m_SysStat.SetItemText(10, 1, _T("1000 Mbps"));
	m_SysStat.InsertItem(11, _T("以太网负载"), 0);
	m_SysStat.SetItemText(11, 1, _T("0.00 MB/s"));
	m_SysStat.InsertItem(12, _T("内部温度"), 0);
	m_SysStat.SetItemText(12, 1, _T("38.062℃"));
	m_SysStat.InsertItem(13, _T("处理延迟"), 0);
	m_SysStat.SetItemText(13, 1, _T("0 μs"));
	m_SysStat.InsertItem(14, _T("处理延迟峰值"), 0);
	m_SysStat.SetItemText(14, 1, _T("0 μs"));
	m_SysStat.InsertItem(15, _T("校准状态"), 0);
	m_SysStat.SetItemText(15, 1, _T("已校准"));
}

void SysStatus::ShowSysCount()
{
	m_SysCount.InsertItem(0, _T("轮廓面积"), 0); // 插入0行0列
	m_SysCount.SetItemText(0, 1, _T("0"));
	m_SysCount.SetItemText(0, 2, _T("0.031"));
	m_SysCount.SetItemText(0, 3, _T("0.031"));
	m_SysCount.SetItemText(0, 4, _T("0.031"));
	m_SysCount.SetItemText(0, 5, _T("0.031"));
	m_SysCount.SetItemText(0, 6, _T("0"));
	m_SysCount.SetItemText(0, 7, _T("0"));
	m_SysCount.SetItemText(0, 8, _T("0"));
	m_SysCount.SetItemText(0, 9, _T("53"));
	m_SysCount.SetItemText(0, 10, _T("0"));
	m_SysCount.SetItemText(0, 11, _T("0"));

	m_SysCount.InsertItem(1, _T("点云体积"), 0); // 插入1行0列
	m_SysCount.SetItemText(1, 1, _T("22"));
	m_SysCount.SetItemText(1, 2, _T("2.228"));
	m_SysCount.SetItemText(1, 3, _T("2.228"));
	m_SysCount.SetItemText(1, 4, _T("2.228"));
	m_SysCount.SetItemText(1, 5, _T("2.228"));
	m_SysCount.SetItemText(1, 6, _T("0"));
	m_SysCount.SetItemText(1, 7, _T("0"));
	m_SysCount.SetItemText(1, 8, _T("0"));
	m_SysCount.SetItemText(1, 9, _T("45"));
	m_SysCount.SetItemText(1, 10, _T("0"));
	m_SysCount.SetItemText(1, 11, _T("0"));

	m_SysCount.InsertItem(2, _T("轮廓距离"), 0); // 插入2行0列
	m_SysCount.SetItemText(2, 1, _T("16"));
	m_SysCount.SetItemText(2, 2, _T("10.26"));
	m_SysCount.SetItemText(2, 3, _T("10.26"));
	m_SysCount.SetItemText(2, 4, _T("10.26"));
	m_SysCount.SetItemText(2, 5, _T("10.26"));
	m_SysCount.SetItemText(2, 6, _T("0"));
	m_SysCount.SetItemText(2, 7, _T("0"));
	m_SysCount.SetItemText(2, 8, _T("0"));
	m_SysCount.SetItemText(2, 9, _T("43"));
	m_SysCount.SetItemText(2, 10, _T("0"));
	m_SysCount.SetItemText(2, 11, _T("0"));

	m_SysCount.InsertItem(3, _T("轮廓位置-X"), 0); // 插入3行0列
	m_SysCount.SetItemText(3, 1, _T("18"));
	m_SysCount.SetItemText(3, 2, _T("-2.104"));
	m_SysCount.SetItemText(3, 3, _T("-2.104"));
	m_SysCount.SetItemText(3, 4, _T("-2.104"));
	m_SysCount.SetItemText(3, 5, _T("-2.104"));
	m_SysCount.SetItemText(3, 6, _T("0"));
	m_SysCount.SetItemText(3, 7, _T("0"));
	m_SysCount.SetItemText(3, 8, _T("0"));
	m_SysCount.SetItemText(3, 9, _T("39"));
	m_SysCount.SetItemText(3, 10, _T("0"));
	m_SysCount.SetItemText(3, 11, _T("0"));

	m_SysCount.InsertItem(4, _T("轮廓圆半径"), 0); // 插入4行0列
	m_SysCount.SetItemText(4, 1, _T("23"));
	m_SysCount.SetItemText(4, 2, _T("11.193"));
	m_SysCount.SetItemText(4, 3, _T("11.193"));
	m_SysCount.SetItemText(4, 4, _T("11.193"));
	m_SysCount.SetItemText(4, 5, _T("11.193"));
	m_SysCount.SetItemText(4, 6, _T("0"));
	m_SysCount.SetItemText(4, 7, _T("0"));
	m_SysCount.SetItemText(4, 8, _T("0"));
	m_SysCount.SetItemText(4, 9, _T("30"));
	m_SysCount.SetItemText(4, 10, _T("0"));
	m_SysCount.SetItemText(4, 11, _T("0"));

	m_SysCount.InsertItem(5, _T("轮廓边界框-X"), 0); // 插入5行0列
	m_SysCount.SetItemText(5, 1, _T("20"));
	m_SysCount.SetItemText(5, 2, _T("0.034"));
	m_SysCount.SetItemText(5, 3, _T("0.034"));
	m_SysCount.SetItemText(5, 4, _T("0.034"));
	m_SysCount.SetItemText(5, 5, _T("0.034"));
	m_SysCount.SetItemText(5, 6, _T("0"));
	m_SysCount.SetItemText(5, 7, _T("0"));
	m_SysCount.SetItemText(5, 8, _T("0"));
	m_SysCount.SetItemText(5, 9, _T("35"));
	m_SysCount.SetItemText(5, 10, _T("0"));
	m_SysCount.SetItemText(5, 11, _T("0"));

	m_SysCount.InsertItem(6, _T("轮廓倒角-角度"), 0); // 插入6行0列
	m_SysCount.SetItemText(6, 1, _T("31"));
	m_SysCount.SetItemText(6, 2, _T("无效"));
	m_SysCount.SetItemText(6, 3, _T("0"));
	m_SysCount.SetItemText(6, 4, _T("0"));
	m_SysCount.SetItemText(6, 5, _T("0"));
	m_SysCount.SetItemText(6, 6, _T("0"));
	m_SysCount.SetItemText(6, 7, _T("0"));
	m_SysCount.SetItemText(6, 8, _T("0"));
	m_SysCount.SetItemText(6, 9, _T("7"));
	m_SysCount.SetItemText(6, 10, _T("0"));
	m_SysCount.SetItemText(6, 11, _T("0"));

	m_SysCount.InsertItem(7, _T("轮廓凹槽宽"), 0); // 插入7行0列
	m_SysCount.SetItemText(7, 1, _T("25"));
	m_SysCount.SetItemText(7, 2, _T("0.587"));
	m_SysCount.SetItemText(7, 3, _T("0.587"));
	m_SysCount.SetItemText(7, 4, _T("0.587"));
	m_SysCount.SetItemText(7, 5, _T("0.587"));
	m_SysCount.SetItemText(7, 6, _T("0"));
	m_SysCount.SetItemText(7, 7, _T("0"));
	m_SysCount.SetItemText(7, 8, _T("0"));
	m_SysCount.SetItemText(7, 9, _T("26"));
	m_SysCount.SetItemText(7, 10, _T("0"));
	m_SysCount.SetItemText(7, 11, _T("0"));

	m_SysCount.InsertItem(8, _T("轮廓凸起宽度"), 0); // 插入8行0列
	m_SysCount.SetItemText(8, 1, _T("32"));


	m_SysCount.InsertItem(9, _T("轮廓交叉-X"), 0); // 插入9行0列
	m_SysCount.SetItemText(9, 1, _T("27"));
	m_SysCount.SetItemText(9, 2, _T("无效"));
	m_SysCount.SetItemText(9, 3, _T("-231.4"));
	m_SysCount.SetItemText(9, 4, _T("84.789"));
	m_SysCount.SetItemText(9, 5, _T("-20.607"));
	m_SysCount.SetItemText(9, 6, _T("316.189"));
	m_SysCount.SetItemText(9, 7, _T("182.551"));
	m_SysCount.SetItemText(9, 8, _T("0"));
	m_SysCount.SetItemText(9, 9, _T("3"));
	m_SysCount.SetItemText(9, 10, _T("19"));
	m_SysCount.SetItemText(9, 11, _T("0"));
}




