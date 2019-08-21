// ShowAllInfo.cpp: 实现文件
//

#include "stdafx.h"
#include "GocatorCamera.h"
#include "ShowAllInfo.h"
#include "afxdialogex.h"


// ShowAllInfo 对话框

IMPLEMENT_DYNAMIC(ShowAllInfo, CDialogEx)

ShowAllInfo::ShowAllInfo(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ALlINFODLG, pParent)
{
	SetInfo();
}

ShowAllInfo::~ShowAllInfo()
{
}

void ShowAllInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ALLINFO_EDT, m_allInfoEdit);
}

BOOL ShowAllInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//TODO 设置窗口标题
	SetWindowText(_T("三维扫描系统详细信息"));

	CString strTemp;
	CString strOut;

	m_Font.CreatePointFont(120, _T("Arial"), NULL);  //_T("Arial")
	m_allInfoEdit.SetFont(&m_Font, true);

	strOut = "安全和维护参数\r\n";
	strTemp.Format(_T("----------------环境照明信息：\r\n"));
	strOut += strTemp;
	strTemp.Format(_T("本产品使用的成像仪对环境光非常敏感，因此杂散光可能对测量造成不利影响。\r\n"));
	strOut += strTemp;
	strTemp.Format(_T("请勿在可能影响测量的窗户或照明灯具附近操作本设备。\r\n"));
	strOut += strTemp;
	strTemp.Format(_T("如果设备必须安装在环境光较强的环境中，则可能需要安装遮光罩或类似设备以防止光线影响测量。\r\n"));
	strOut += strTemp;
	strTemp.Format(_T("避免将传感器安装在危险环境中\r\n"));
	strOut += strTemp;
	strTemp.Format(_T("为确保可靠操作Gocator传感器并防止其损坏，请避免将传感器安装在以下位置\r\n"));
	strOut += strTemp;
	strTemp.Format(_T("    ·潮湿、多尘或通风不良处；\r\n"));
	strOut += strTemp;
	strTemp.Format(_T("    ·高温处，如暴露在阳光直射下；\r\n"));
	strOut += strTemp;
	strTemp.Format(_T("    ·周围有易燃或易腐蚀性气体；\r\n"));
	strOut += strTemp;  
	strTemp.Format(_T("    ·可能受到强烈震动或冲击的地方；\r\n"));
	strOut += strTemp;
	strTemp.Format(_T("    ·可能被水、油或化学物质溅到的地方；\r\n"));
	strOut += strTemp;
	strTemp.Format(_T("    ·容易产生静电的地方。确保环境条件符合规格。\r\n"));
	strOut += strTemp;
	SetDlgItemText(IDC_ALLINFO_EDT, strOut);

	strOut += _T("Gocator传感器适宜的工作环境为0 - 50℃和25 - 85%相对湿度（非冷凝条件）。\r\n");
	strTemp.Format(_T("储存温度为-30 - 70℃。\r\n"));
	strOut += strTemp;
	strTemp.Format(_T("Master网络控制器的额定工作温度范围同样为0 - 50℃。\r\n"));
	strOut += strTemp;
	SetDlgItemText(IDC_ALLINFO_EDT, strOut);

	//IDC_EDITION_STA

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


BEGIN_MESSAGE_MAP(ShowAllInfo, CDialogEx)
	ON_BN_CLICKED(IDOK, &ShowAllInfo::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &ShowAllInfo::OnBnClickedCancel)
END_MESSAGE_MAP()


// ShowAllInfo 消息处理程序
void ShowAllInfo::SetInfo()
{
}


void ShowAllInfo::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}


void ShowAllInfo::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}
