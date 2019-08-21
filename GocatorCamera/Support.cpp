// Support.cpp: 实现文件
//

#include "stdafx.h"
#include "GocatorCamera.h"
#include "Support.h"
#include "afxdialogex.h"


// Support 对话框

IMPLEMENT_DYNAMIC(Support, CDialogEx)

Support::Support(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SUPPORT, pParent)
{
	
}

Support::~Support()
{
}

void Support::SetInitInfo()
{
	//// TODO: 在此添加控件通知处理程序代码
	//CBitmap bitmap; // CBitmap对象，用于加载位图
	//HBITMAP hBmp;   // 保存CBitmap加载的位图的句柄

	//bitmap.LoadBitmapA(IDB_SOFTSUPPORT);// 将位图IDB_BITMAP1加载到bitmap
	//hBmp = (HBITMAP)bitmap.GetSafeHandle(); // 获取bitmap加载位图的句柄
	//m_picCon.SetBitmap(hBmp);   // 设置图片空间m_pictureControl的位图为IDB_BITMAP1
}

void Support::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SUPINFO_EDT, m_supEdit);
}

BOOL Support::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//设置编辑框的字体
	m_Font.CreatePointFont(116, _T("微软雅黑"), NULL);  //_T("Arial")
	m_supEdit.SetFont(&m_Font, true);

	//设置默认光标位于最后一行
	

	//m_Font = m_supEdit.GetFont();//获取CFont对象
	//LOGFONT lf;
	//m_Font->GetLogFont(&lf);//获取LOGFONT结构体
	//lf.lfHeight = -26;    //修改字体大小
	//lf.lfItalic = TRUE;        //倾斜
	//lf.lfWeight = 400;   //修改字体的粗细
	//m_Font->CreateFontIndirectA(&lf); //.CreateFontIndirectW(&lf);//创建一个新的字体
	//m_supEdit.SetFont(m_Font);

	strInfo = "0";
	strInfoTemp = "0";

	strInfo = "Gocator 快照 LED 投影传感器专为三维测量和控制应用而设计。";
	strInfoTemp.Format(_T("Gocator 传感器使用 Web 浏览器进行配置,可连接至各种输入和输出设备。Gocator 传感器也可依用户自定义程序进行配置。\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("-Pico - C\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("网站：\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("http ://code.google.com/p/picoc/\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("许可证：\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("picoc 是在“新 BSD 许可证”下发布的。\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("http ://www.opensource.org/licenses/bsd-license.php\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("版权所有(c) 2009 - 2011，Zik Saleeba\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("保留所有权利。\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("如果符合下述条件，即允许重新发布和使用源文件及二进制格式（无论是否有修订）。\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("* 重新发布本软件的源代码时必须保留上述的版权声明、该条件列表，以及下述的免责声明。\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("* 以二进制格式重新发布时，必须在随附的发布文档和 / 或材料中复制上述版权声明、该条件列表及下述\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("免责声明。\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("* 未经事先书面许可，不得使用 Zik Saleeba 或其编著者之名称，来为本软件之衍生产品进行冠名或推广。\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("本软件由版权所有者和编著者按“原样”提供，不作任何明示或暗示的担保，包括但不限于对适销性和\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("特定用途的适用性的暗示保证。任何情况下，无论基于任何法律理论，合约、严格责任或侵权行为（包括疏忽或其\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("他），版权所有者或贡献者均无需就使用本软件出现的任何直接、间接、偶然、特殊、惩罚性或继发性损害\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("（包括但不限于采购替代商品或服务；损失使用价值、数据或利润；或业务中断）承担责任，无论是否告知\r\n"));
	strInfo += strInfoTemp;
	strInfoTemp.Format(_T("发生该等损害的可能性。\r\n"));
	strInfo += strInfoTemp;
	SetDlgItemText(IDC_SUPINFO_EDT, strInfo);
	//IDC_SUPPORTEDIT

	m_supEdit.SetSel(-1,0, FALSE);
	m_supEdit.SetFocus();

	/*int length = m_supEdit.GetWindowTextLength();
	m_supEdit.SetSel(length, length, FALSE);
	m_supEdit.SetFocus();*/

	UpdateData(false);

	return TRUE;
}


BEGIN_MESSAGE_MAP(Support, CDialogEx)
	ON_BN_CLICKED(IDOK, &Support::OnBnClickedOk)
END_MESSAGE_MAP()


// Support 消息处理程序


void Support::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}


