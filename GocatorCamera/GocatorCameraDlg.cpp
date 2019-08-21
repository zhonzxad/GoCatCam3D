// GocatorCameraDlg.cpp: 实现文件
//
#include "stdafx.h"
#include "GocatorCamera.h"
#include "GocatorCameraDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(disable:4002)
#pragma warning(disable:26495)
#pragma warning(disable:26454)


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedShowallinfoBtn();
	
	CLogWnd *m_abtLog;
	afx_msg void OnStnClickedEditionSta();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_SHOWALLINFO_BTN, &CAboutDlg::OnBnClickedShowallinfoBtn)
	ON_STN_CLICKED(IDC_EDITION_STA, &CAboutDlg::OnStnClickedEditionSta)
END_MESSAGE_MAP()


// CGocatorCameraDlg 对话框
CGocatorCameraDlg::CGocatorCameraDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GOCATORCAMERA_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINICON);
}

void CGocatorCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, TabCtrl);
}

BEGIN_MESSAGE_MAP(CGocatorCameraDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CGocatorCameraDlg::OnTcnSelchangeTab)
	ON_WM_TIMER()
	ON_WM_ACTIVATE()
	ON_WM_GETMINMAXINFO()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOVE()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_SAVETOBMP_BTN, &CGocatorCameraDlg::OnBnClickedSavetobmpBtn)
	ON_BN_CLICKED(IDC_CLOSE_BTN, &CGocatorCameraDlg::OnBnClickedCloseBtn)
	ON_BN_CLICKED(IDC_MINSIZE_BTN, &CGocatorCameraDlg::OnBnClickedMinsizeBtn)
	ON_BN_CLICKED(IDC_STARTCAMLONG_BTN, &CGocatorCameraDlg::OnBnClickedStartCamLongBtn)
	ON_BN_CLICKED(IDC_OUTSETSTARTCAMERA, &CGocatorCameraDlg::OnBnClickedOutSetStartCam)
	ON_BN_CLICKED(IDC_READSYSCONFIGURE, &CGocatorCameraDlg::OnBnClickedReadsysconfigure)
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_ABOUT, &CGocatorCameraDlg::OnBnClickedAbout)
	ON_BN_CLICKED(IDC_SYSTEMPAUSE, &CGocatorCameraDlg::OnBnClickedSystempause)
END_MESSAGE_MAP()


// CGocatorCameraDlg 消息处理程序

BOOL CGocatorCameraDlg::OnInitDialog()
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	SetWindowText(_T("三维扫描操作系统-GUI"));

	// TODO: 在此添加额外的初始化代码

	//初始化标签页选择
	InitChangeTabForm();

	//设置状态栏初始化
	GetClientRect(&m_rect);
	//m_Statusbar.Create(this);
	if (!m_Statusbar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_FLYBY))
	{
		m_logdlg->WARN(_T("未能创建状态栏\n"));
		//return -1;      // fail to create   
	}

	UINT nID[5] = { 10000,10001,10002,10003 };
	m_Statusbar.SetIndicators(nID, 4);
	m_Statusbar.SetPaneInfo(0, 10000, SBPS_NORMAL, 76);
	m_Statusbar.SetPaneInfo(1, 10001, SBPS_STRETCH, 0);
	m_Statusbar.SetPaneInfo(2, 10002, SBPS_NORMAL, 150);
	m_Statusbar.SetPaneInfo(3, 10003, SBPS_NORMAL, 50);
	m_Statusbar.MoveWindow(0, m_rect.bottom - 30, m_rect.right, 30, true);

	CString str;
	str.Format(_T("   Log日志"));
	m_Statusbar.SetPaneText(0, str, true);

	//str.Format(_T("这个是状态栏的第二个栏，验证一下长度"));
	str.Format(_T("版权所有--V1.9.4---郑州大学软件学院_张璇---2019"));
	m_Statusbar.SetPaneText(1, str, true);

	//启动定时器参数：定时器标号，定时时间（ms）。启动定时器1，每隔1s刷新一次
	SetTimer(2, 1000, NULL);

	str.Format(_T("ZH/中国"));
	m_Statusbar.SetPaneText(3, str, true);

	// 获取对话框初始大小    
	GetClientRect(&m_rect);  //获取对话框的大小
	old.x = m_rect.right - m_rect.left;
	old.y = m_rect.bottom - m_rect.top;

	//日志窗口初始化
	m_logdlg = new CLogWnd;
	m_logdlg->CreateLogWnd(_T("CLogWnd"), this, TRUE); // follow mode
	//m_logdlg->CreateLogWnd(_T("CLogWnd"), this, FALSE); // non-follow mode
	//m_logdlg->Show();
	
	//TODO 测试首页静态文本文字可改变

	//TODO 初始化系统信息栏
	SetDlgItemText(IDC_SYSINFOSHOW, _T("【请在程序开启前，确认相机已经上电运作。】"));

	//TODO 初始化软件支持信息
	m_supObj.SetInitInfo();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CGocatorCameraDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CGocatorCameraDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CGocatorCameraDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGocatorCameraDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialogEx::OnActivate(nState, pWndOther, bMinimized);

	// TODO: 在此处添加消息处理程序代码

	if (nState == WA_INACTIVE && m_bMoving)
	{
		m_bMoving = FALSE;
		ReleaseCapture();
	}

}

void CGocatorCameraDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	//调整最小高度与宽度 x是长度，y是宽度
	lpMMI->ptMinTrackSize.x = 703;
	lpMMI->ptMinTrackSize.y = 508;
	//调整最大高度与宽度
	lpMMI->ptMaxTrackSize.x = 703;
	lpMMI->ptMaxTrackSize.y = 508;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CGocatorCameraDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	m_bMoving = TRUE;
	SetCapture();
	m_ptMouse = point;

	PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));

	//TODO 点击状态栏日志显示相应的日志界面，缺点无法获取到状态栏的点击事件
	/*if (point.x > 0 && point.x < 80)
		if (point.y > 597 && point.y < 622)
			m_logdlg->Show();*/		//old方法
	if (point.x > 0 && point.x < 80)
		m_logdlg->Show();

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CGocatorCameraDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_bMoving)
	{
		m_bMoving = FALSE;
		ReleaseCapture();
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}

void CGocatorCameraDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);

	// TODO: 在此处添加消息处理程序代码
	m_logdlg->MoveFollow();
}


void CGocatorCameraDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_bMoving)
	{
		int x, y;
		x = point.x - m_ptMouse.x;
		y = point.y - m_ptMouse.y;
		if (abs(x) >= 2 || abs(y) >= 2)
		{
			RECT rc = { 0 };
			GetWindowRect(&rc);
			OffsetRect(&rc, x, y);
			MoveWindow(&rc, TRUE);
		}
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void CGocatorCameraDlg::ReSize(void)
{
	float fsp[2];
	POINT Newp; //获取现在对话框的大小  
	CRect recta;
	GetClientRect(&recta);     //取客户区大小    
	Newp.x = recta.right - recta.left;
	Newp.y = recta.bottom - recta.top;
	fsp[0] = (float)Newp.x / old.x;
	fsp[1] = (float)Newp.y / old.y;
	CRect Rect;
	int woc;
	CPoint OldTLPoint, TLPoint; //左上角  
	CPoint OldBRPoint, BRPoint; //右下角  
	HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);  //列出所有控件    
	while (hwndChild) {
		woc = ::GetDlgCtrlID(hwndChild);//取得ID  
		GetDlgItem(woc)->GetWindowRect(Rect);
		ScreenToClient(Rect);
		OldTLPoint = Rect.TopLeft();
		TLPoint.x = long(OldTLPoint.x*fsp[0]);
		TLPoint.y = long(OldTLPoint.y*fsp[1]);
		OldBRPoint = Rect.BottomRight();
		BRPoint.x = long(OldBRPoint.x *fsp[0]);
		BRPoint.y = long(OldBRPoint.y *fsp[1]);
		Rect.SetRect(TLPoint, BRPoint);
		GetDlgItem(woc)->MoveWindow(Rect, TRUE);
		hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
	}
	old = Newp;
}

void CGocatorCameraDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码

	//判断窗口是不是最小化了，因为窗口最小化之后 ，窗口的长和宽会变成0，当前一次变化的时就会出现除以0的错误操作
	if (nType == SIZE_MINIMIZED) return;   //如果是窗体最小化则什么都不做
	else
	{
		ReSize();
		Invalidate();
	}
}

void CGocatorCameraDlg::InitChangeTabForm()
{
	TabCtrl.InsertItem(0, _T("同步接收数据"));
	m_recSyncData.Create(IDD_RECEIVESYSDATA, &TabCtrl);
	TabCtrl.InsertItem(1, _T("异步接收数据"));
	m_recAsy.Create(IDD_RECEIVEASYNC, &TabCtrl);
	TabCtrl.InsertItem(2, _T("接收表面数据"));
	m_recSurface.Create(IDD_RECEIVESURFACE, &TabCtrl);
	TabCtrl.InsertItem(3, _T("(线扫)接受轮廓数据"));
	m_recFromPro.Create(IDD_RECFROMPROFILEMODE, &TabCtrl);
	TabCtrl.InsertItem(4, _T("备份当前工作环境"));
	m_backRes.Create(IDD_BACKUPRESTORE, &TabCtrl);
	TabCtrl.InsertItem(5, _T("伙伴传感器系统"));
	m_buddySys.Create(IDD_BUDDYSYSTEM, &TabCtrl);
	TabCtrl.InsertItem(6, _T("传感器加速器系统"));
	m_accRecSys.Create(IDD_ACCELERATORRECEIVESYS, &TabCtrl);
	TabCtrl.InsertItem(7, _T("支持"));
	m_supObj.Create(IDD_SUPPORT, &TabCtrl);

	//获取TAB的客户端矩形框，从而设置各属性页在TAB上的物理位置-xcy
	CRect rs;	//控制标签控件客户区的位置和大小-xcy
	TabCtrl.GetClientRect(&rs);	//获取标签控件客户区Rect
	rs.top += 25;
	rs.bottom += 12;
	rs.left -= 2;
	rs.right -= 2;

	//设置默认为异步拍摄
	TabCtrl.SetCurSel(0);

	//根据rs设置子对话框尺寸
	m_recAsy.SetWindowPos(NULL, rs.left, rs.top, rs.Width(), rs.Height(), SWP_HIDEWINDOW);
	m_recSurface.SetWindowPos(NULL, rs.left, rs.top, rs.Width(), rs.Height(), SWP_HIDEWINDOW);
	m_recSyncData.SetWindowPos(NULL, rs.left, rs.top, rs.Width(), rs.Height(), SWP_SHOWWINDOW);
	m_recFromPro.SetWindowPos(NULL, rs.left, rs.top, rs.Width(), rs.Height(), SWP_HIDEWINDOW);
	m_backRes.SetWindowPos(NULL, rs.left, rs.top, rs.Width(), rs.Height(), SWP_HIDEWINDOW);
	m_buddySys.SetWindowPos(NULL, rs.left, rs.top, rs.Width(), rs.Height(), SWP_HIDEWINDOW);
	m_accRecSys.SetWindowPos(NULL, rs.left, rs.top, rs.Width(), rs.Height(), SWP_HIDEWINDOW);
	m_supObj.SetWindowPos(NULL, rs.left, rs.top, rs.Width(), rs.Height(), SWP_HIDEWINDOW);
}

void CGocatorCameraDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	switch (TabCtrl.GetCurSel())
	{
	// 如果标签控件当前选择标签为“同步数据”
	case 0:
		m_recSyncData.ShowWindow(SW_SHOW);
		m_recAsy.ShowWindow(SW_HIDE);
		m_recSurface.ShowWindow(SW_HIDE);
		m_recFromPro.ShowWindow(SW_HIDE);
		m_backRes.ShowWindow(SW_HIDE);
		m_buddySys.ShowWindow(SW_HIDE);
		m_accRecSys.ShowWindow(SW_HIDE);
		m_supObj.ShowWindow(SW_HIDE);
		break;
	// 如果标签控件当前选择标签为“异步接收数据”
	case 1:
		m_recSyncData.ShowWindow(SW_HIDE);
		m_recAsy.ShowWindow(SW_SHOW);
		m_recSurface.ShowWindow(SW_HIDE);
		m_recFromPro.ShowWindow(SW_HIDE);
		m_backRes.ShowWindow(SW_HIDE);
		m_buddySys.ShowWindow(SW_HIDE);
		m_accRecSys.ShowWindow(SW_HIDE);
		m_supObj.ShowWindow(SW_HIDE);
		break;
	// 如果标签控件当前选择标签为“接收表面数据”
	case 2:
		m_recSyncData.ShowWindow(SW_HIDE);
		m_recAsy.ShowWindow(SW_HIDE);
		m_recSurface.ShowWindow(SW_SHOW);
		m_recFromPro.ShowWindow(SW_HIDE);
		m_backRes.ShowWindow(SW_HIDE);
		m_buddySys.ShowWindow(SW_HIDE);
		m_accRecSys.ShowWindow(SW_HIDE);
		m_supObj.ShowWindow(SW_HIDE);
		break;
	// 如果标签控件当前选择标签为“（线扫）接收轮廓数据”
	case 3:
		m_recSyncData.ShowWindow(SW_HIDE);
		m_recAsy.ShowWindow(SW_HIDE);
		m_recSurface.ShowWindow(SW_HIDE);
		m_recFromPro.ShowWindow(SW_SHOW);
		m_backRes.ShowWindow(SW_HIDE);
		m_buddySys.ShowWindow(SW_HIDE);
		m_accRecSys.ShowWindow(SW_HIDE);
		m_supObj.ShowWindow(SW_HIDE);
		break;
	// 如果标签控件当前选择标签为“备份工作环境”
	case 4:
		m_recSyncData.ShowWindow(SW_HIDE);
		m_recAsy.ShowWindow(SW_HIDE);
		m_recSurface.ShowWindow(SW_HIDE);
		m_recFromPro.ShowWindow(SW_HIDE);
		m_backRes.ShowWindow(SW_SHOW);
		m_buddySys.ShowWindow(SW_HIDE);
		m_accRecSys.ShowWindow(SW_HIDE);
		m_supObj.ShowWindow(SW_HIDE);
		break;
	// 如果标签控件当前选择标签为“伙伴传感器”
	case 5:
		m_recSyncData.ShowWindow(SW_HIDE);
		m_recAsy.ShowWindow(SW_HIDE);
		m_recSurface.ShowWindow(SW_HIDE);
		m_recFromPro.ShowWindow(SW_HIDE);
		m_backRes.ShowWindow(SW_HIDE);
		m_buddySys.ShowWindow(SW_SHOW);
		m_accRecSys.ShowWindow(SW_HIDE);
		m_supObj.ShowWindow(SW_HIDE);
		break;
	// 如果标签控件当前选择标签为“传感器加速器”
	case 6:
		m_recSyncData.ShowWindow(SW_HIDE);
		m_recAsy.ShowWindow(SW_HIDE);
		m_recSurface.ShowWindow(SW_HIDE);
		m_recFromPro.ShowWindow(SW_HIDE);
		m_backRes.ShowWindow(SW_HIDE);
		m_buddySys.ShowWindow(SW_HIDE);
		m_accRecSys.ShowWindow(SW_SHOW);
		m_supObj.ShowWindow(SW_HIDE);
		break;
	// 如果标签控件当前选择标签为“支持信息”
	case 7:
		m_recSyncData.ShowWindow(SW_HIDE);
		m_recAsy.ShowWindow(SW_HIDE);
		m_recSurface.ShowWindow(SW_HIDE);
		m_recFromPro.ShowWindow(SW_HIDE);
		m_backRes.ShowWindow(SW_HIDE);
		m_buddySys.ShowWindow(SW_HIDE);
		m_accRecSys.ShowWindow(SW_HIDE);
		m_supObj.ShowWindow(SW_SHOW);
		break;
	default:
		break;
	}
}

void CGocatorCameraDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 1:   //定时器1处理函数，定时发送数据进行更新
		{
			//char *a = NULL;
			//a = new char[12];
			//a[0] = 0x55;  a[1] = 0xAA;  a[2] = 0xdc;             //数据开头
			//a[3] = 0x06;
			//a[4] = 0x00;
			//a[5] = 0x03;
			//a[6] = 0x44;
			//a[7] = 0x44;
			//a[8] = 0xcc;  a[9] = 0x33;  a[10] = 0xc3; a[11] = 0x3c;//数据结尾
			//m_SerialPort.WriteData(a, 12);
			//a = NULL;
			//delete a;
			//break;
		}

	case 2:   //处理状态栏时间定时器
		{
			CTime m_Time;
			m_Time = CTime::GetCurrentTime();//获取当前系统时间
			CString str = (CString)m_Time.Format(_T("%Y-%m-%d  %H:%M:%S"));
			m_Statusbar.SetPaneText(2, str, TRUE); //状态栏显示时间 
			break;
		}

	case 3:	 //处理定时关闭日志窗口定时器
		{
			//OnHideLogDig();
			m_logdlg->Hidden();
			break;
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

int CGocatorCameraDlg::SaveImageFromSaveBtnClicked(CString strfilePath)
{
	kAssembly assembly = kNULL;      //部件句柄
	kImage image = kNULL;            //图像句柄
	kSize width = 320;
	kSize height = 640;
	kSize i, j;

	//获得kApi核心程序集的初始化引用（kApiLib）
	kCheck(kApiLib_Construct(&assembly));

	//创建一个图像对象
	kCheck(kImage_Construct(&image, kTypeOf(k8u), width, height, kNULL));

	//将图像像素设置为测试图案
	for (i = 0; i < height; ++i)
	{
		//获取指向当前行中第一个元素的指针
		k8u* row = kImage_RowAtT(image, i, k8u);
		for (j = 0; j < width; ++j)
		{
			row[j] = (k8u)(i + j);
		}
	}

	//使用bmp格式保存图像
	kImage_Export(image, (kChar *)(LPSTR)(LPCTSTR)strfilePath);
	//kCheck(kImage_Export(image, (char *)strfilePath));

	//销毁图像对象
	kCheck(kObject_Destroy(image));

	//销毁kApiLib程序集
	kCheck(kObject_Destroy(assembly));

	return kOK;
}

void CGocatorCameraDlg::OnBnClickedMinsizeBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	PostMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

void CGocatorCameraDlg::OnBnClickedOutSetStartCam()
{
	//TODO 开启设置外部触发模式
	m_dealOut.SetOutPutStartCamera();
	m_dealOut.SetStaticTxt();
}

void CGocatorCameraDlg::OnBnClickedReadsysconfigure()
{
	// TODO: 在此添加控件通知处理程序代码

	// TODO: 在此添加控件通知处理程序代码
	/*SysStatus m_dlg;
	m_dlg.DoModal();*/
	SysStatus *pDlg = new SysStatus;
	pDlg->Create(IDD_SYSSTATUS, this);
	pDlg->ShowWindow(SW_SHOW);

	//非模态界面执行即可执行相关的程序。
	//m_logdlg->ERR(_T("状态界面出现错误---error错误"));
}


LRESULT CGocatorCameraDlg::OnNcHitTest(CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// 取得鼠标所在的窗口区域
	UINT nHitTest = CDialog::OnNcHitTest(point);

	// 如果鼠标在窗口客户区，则返回标题条代号给Windows
	// 使Windows按鼠标在标题条上类进行处理，即可单击移动窗口
	return (nHitTest == HTCLIENT) ? HTCAPTION : nHitTest;
}


void CGocatorCameraDlg::OnBnClickedAbout()
{
	// TODO: 在此添加控件通知处理程序代码
	SysAbout m_sysAbo;
	m_sysAbo.DoModal();

	////模态界面执行返回后执行相关的程序。
	m_logdlg->INFO(_T("关于界面正常初始化，读取一次系统的状态"));
}


void CAboutDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}

void CAboutDlg::OnBnClickedShowallinfoBtn()
{
	//// TODO: 在此添加控件通知处理程序代码
	//无用注释，稍后取消
	//SysStatus *pDlg = new SysStatus;
	//pDlg->Create(IDD_SYSSTATUS, this);
	//pDlg->ShowWindow(SW_SHOW);

	////非模态界面执行即可执行相关的程序。
	//m_abtLog->ERR(_T("状态界面出现错误---error错误"));
}


void CAboutDlg::OnStnClickedEditionSta()
{
	// TODO: 在此添加控件通知处理程序代码
	//函数废弃
}

void CGocatorCameraDlg::OnBnClickedCloseBtn()
{
	// TODO: 创建线程方法
	//TODO 创建进程 https://blog.csdn.net/yuyan987/article/details/78644922

	//使用WinExec更加快速，但会卡死主线程
	//WinExec("C:\\Users\\zxuan\\Desktop\\3DCameraProject\\Discover-v0.2\\Debug\\Discover.exe", SW_SHOW);
	//WinExec("..\\..\\\\Discover-v0.2\\Debug\\Discover.exe", SW_SHOW);
	//CreateProcess(TEXT("..\\..\\\\Discover-v0.2\\Debug\\Discover.exe"),);

	//使用CreateProcess更加稳定，但是速度不够快
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	TCHAR szCommandLine[] = TEXT("..\\DiscoverRelease\\Discover.exe");
	CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	AfxGetMainWnd()->SendMessage(WM_CLOSE);

}

void CGocatorCameraDlg::OnBnClickedStartCamLongBtn()
{
	CString strOut;
	CString strTemp;
	// TODO: 在此添加控件通知处理程序代码
	switch (TabCtrl.GetCurSel())
	{
	case 0:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_同步拍摄_程序"));
		MessageBox(_T("为了保证程序运行稳定\r\n同步拍摄设置一次拍5张\r\n拍摄循环一次\r\n"), _T("Gocator操作提示"), MB_ICONEXCLAMATION | MB_OK);
		m_recSyncData.SetUseReceiveSync();
		m_logdlg->INFO(_T("进入__SyncReceive__正常"));
		break;
	case 1:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_异步拍摄_程序"));
		m_recAsy.SetUseReceiveAsync();
		m_logdlg->WARN(_T("异常消息__检测到帧处理缺失"));
		//MessageBox("这是一个最简单的消息框！");
		break;
	case 2:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_拍摄表面_程序"));
		MessageBox(_T("每次拍摄\r\n只拍摄一次轮廓\r\n可以循环操作\r\n"), _T("Gocator操作提示"), MB_ICONEXCLAMATION | MB_OK);
		m_recSurface.SetUseReceiveSurfaceFun();
		m_logdlg->INFO(_T("进入__ShotSurface__正常"));
		break;
	case 3:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_拍摄表里轮廓_程序"));
		//m_recFromPro.SetUseReceiveFromProfile();  //旧方法，使用太过冗余
		m_newProfile.SetStartCamera();
		m_logdlg->ERR(_T("错误消息__IP地址错误"));
		break;
	case 4:
		m_logdlg->INFO(_T("进入__BackUpRestore__正常"));
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_备份传感器配置_程序"));
		m_backRes.SetUseBackupRestore();
		break;
	case 5:
		m_logdlg->INFO(_T("进入__BuddySensor__正常"));
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_连接伙伴传感器_程序"));
		m_buddySys.SetUseBuddySysteam();
		break;
	case 6:
		m_logdlg->INFO(_T("进入__Acceleractor_正常"));
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_传感器加速器_相机程序"));
		m_accRecSys.SetUseAcceleratorSys();
		break;
	case 7:
		m_logdlg->INFO(_T("进入__Support__正常"));
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("支持"));
		//m_recSync.SetUseReceiveSync();
		break;
	default:
		break;
	}
}

void CGocatorCameraDlg::OnBnClickedSystempause()
{
	switch (TabCtrl.GetCurSel())
	{
	case 0:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_同步拍摄_暂停相机程序"));
		//m_recSyncData.SetStopCamera();
		MessageBox(_T("无效的暂停操作!!\r\n"), _T("Gocator设置错误"), MB_ICONEXCLAMATION | MB_OK);
		break;
	case 1:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_异步拍摄_暂停相机程序"));
		m_recAsy.SetStopCamera();
		break;
	case 2:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_物件表面拍摄_暂停相机程序"));
		MessageBox(_T("无效的暂停操作!!\r\n"), _T("Gocator设置错误"), MB_ICONEXCLAMATION | MB_OK);
		//m_recSurface.SetStopCamera();
		break;
	case 3:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_物件轮廓拍摄_暂停相机程序"));
		//m_recFromPro.SetStopCamera();
		m_newProfile.SetStopCamera();
		break;
	case 4:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("无效的暂停相机操作。"));
		MessageBox(_T("无效的暂停相机操作!!!\r\n"), _T("Gocaor设置错误"), MB_ICONEXCLAMATION | MB_OK);
		break;
	case 5:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_伙伴传感器_暂停相机程序"));
		m_buddySys.SetStopCamera();
		break;
	case 6:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_加速器_暂停相机程序"));
		m_accRecSys.SetStopCamera();
		break;
	case 7:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("支持"));
		MessageBox(_T("无效的暂停操作！\r\n"), _T("Gocator操作提示"), MB_ICONEXCLAMATION | MB_OK);
		//m_recSync.SetUseReceiveSync();
		break;
	default:
		break;
	}
}

void CGocatorCameraDlg::OnBnClickedSavetobmpBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	//TODO 保存文件命令
	//kapi中要求函数返回必须使用int型，

	//// 设置过滤器   
	//TCHAR szFilter[] = _T("位图文件(*.bmp)|*.bmp|文本文件(*.txt)|*.txt|所有文件(*.*)|*.*||");
	//// 设置按时间保存文件名
	//CTime m_Time;
	//m_Time = CTime::GetCurrentTime();//获取当前系统时间
	//CString str = (CString)m_Time.Format(_T("%Y-%m-%d_%H-%M-%S"));
	//// 构造保存文件对话框   
	//CFileDialog fileDlg(FALSE, _T("bmp"), str, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	//CString strFilePath, setFileName, outputInfo;

	//// 显示保存文件对话框   
	//if (IDOK == fileDlg.DoModal())
	//{
	//	// 如果点击了文件对话框上的“保存”按钮，则将选择的文件路径显示到编辑框里   
	//	strFilePath = fileDlg.GetPathName();
	//	setFileName = fileDlg.GetFileName();

	//	if (SaveImageFromSaveBtnClicked(strFilePath))			//类型强转发生截断错误
	//	{
	//		GetDlgItem(IDC_FILENAMEINFOSHOW)->SetWindowText(setFileName);
	//		UpdateData(FALSE);
	//		m_logdlg->INFO(_T("保存到文件成功,文件路径为: " + strFilePath + "--INFO消息"));
	//	}
	//	else
	//		m_logdlg->WARN(_T("存储文件失败，请重新操作---WARN消息"));
	//}
	//else
	//	m_logdlg->WARN(_T("无效的目录，请重新选择---WARN消息"));

	//TODO 设置根据选择的标签页保存最后一次拍摄的数据
	switch (TabCtrl.GetCurSel())
	{
	case 0:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_保存同步拍摄数据_程序"));
		//MessageBox(_T("为了保证程序运行稳定\r\n同步拍摄设置一次拍5张\r\n拍摄循环一次\r\n"), _T("Gocator操作提示"), MB_ICONEXCLAMATION | MB_OK);
		m_recSyncData.ToSavaPointCloudData();
		GetDlgItem(IDC_FILENAMEINFOSHOW)->SetWindowText(m_recSyncData.setFileName);
		m_logdlg->WARN(_T("同步导出正常"));
		break;
	case 1:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_保存异步拍摄数据_程序"));
		m_recAsy.ToSavaPointCloudData();
		GetDlgItem(IDC_FILENAMEINFOSHOW)->SetWindowText(m_recAsy.setFileName);
		m_logdlg->WARN(_T("异步导出正常"));
		break;
	case 2:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("开始运行_保存表面拍摄数据_程序"));
		//m_recSurface.SetUseReceiveSurfaceFun();
		MessageBox(_T("此次保存的表面亮度值数据！\r\n"), _T("Gocator保存提示"), MB_ICONEXCLAMATION | MB_OK);
		m_recSurface.ToSavaIntensData();
		GetDlgItem(IDC_FILENAMEINFOSHOW)->SetWindowText(m_recSurface.setFileName);
		m_logdlg->WARN(_T("亮度导出正常"));
		break;
	case 3:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("无效操作"));
		//m_recFromPro.SetUseReceiveFromProfile();
		MessageBox(_T("无效的保存数据操作！\r\n"), _T("Gocator错误提示"), MB_ICONEXCLAMATION | MB_OK);
		m_logdlg->WARN(_T("同步导出正常"));
		break;
	case 4:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("无效操作"));
		//m_backRes.SetUseBackupRestore();
		MessageBox(_T("无效的保存数据操作！\r\n"), _T("Gocator错误提示"), MB_ICONEXCLAMATION | MB_OK);
		break;
	case 5:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("无效操作"));
		//m_buddySys.SetUseBuddySysteam();
		MessageBox(_T("无效的保存数据操作！\r\n"), _T("Gocator错误提示"), MB_ICONEXCLAMATION | MB_OK);
		break;
	case 6:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("无效操作"));
		//m_accRecSys.SetUseAcceleratorSys();
		MessageBox(_T("无效的保存数据操作！\r\n"), _T("Gocator错误提示"), MB_ICONEXCLAMATION | MB_OK);
		break;
	case 7:
		GetDlgItem(IDC_SYSINFOSHOW)->SetWindowText(_T("支持"));
		//m_recSync.SetUseReceiveSync();
		MessageBox(_T("无效的保存数据操作！\r\n"), _T("Gocator错误提示"), MB_ICONEXCLAMATION | MB_OK);
		break;
	default:
		break;
	}

}