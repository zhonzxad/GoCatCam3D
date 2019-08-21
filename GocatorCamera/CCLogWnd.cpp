// CCLogWnd.cpp: implementation of the CLogWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CCLogWnd.h"
#include <stdio.h>
#include <shlobj.h>
#include "afxdialogex.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma warning(disable:4996)
#pragma warning(disable:4002)
#pragma warning(disable:26495)


// Dialog template max-size
#define DLG_TEMPLATE_BUFFER_MAX_LENGTH 1024
#define MAXSIZE 1024

// Skip the first time
#define IGNORE_FIRST_TIME do{						\
	static bool		bIsFirst = true;				\
	if(bIsFirst) {  bIsFirst = false; return;  }	\
	}while(0)

// Function pointer
typedef BOOL (WINAPI *SETLAYEREDWINDOWATTRIBUTES)(HWND, COLORREF, BYTE, DWORD);

// Get desktop path
inline void GetDesktopPath(CString &strPath)
{
	TCHAR path[255];
    ZeroMemory(path,255);
    SHGetSpecialFolderPath(0,path,CSIDL_DESKTOPDIRECTORY,0);
	strPath = path;
}

/////////////////////////////////////////////////////////////////////////////
// CLogEdit

// Message map
BEGIN_MESSAGE_MAP(CLogEdit, CRichEditCtrl)
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// Mouse wheel message function
BOOL CLogEdit::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if(zDelta > 0)	PostMessage(WM_VSCROLL, SB_LINEUP);
	else			PostMessage(WM_VSCROLL, SB_LINEDOWN);
	return CRichEditCtrl::OnMouseWheel(nFlags, zDelta, pt);
}

// Message map
BEGIN_MESSAGE_MAP(CLogWnd, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
END_MESSAGE_MAP()

CLogWnd::CLogWnd()
{
	m_pParent   = NULL;
	m_bIsFollow = TRUE;
	m_bIsHidden = FALSE;
	m_clrInfo   = 0x00FF00;		// Green
	m_clrWarn   = 0x00FFFF;		// Yellow
	m_clrError  = 0x0000FF;		// Red
	m_pBuffer   = new WORD[DLG_TEMPLATE_BUFFER_MAX_LENGTH];
}

// Destructor
CLogWnd::~CLogWnd()
{
	delete []m_pBuffer;
}

// Create window
BOOL CLogWnd::CreateLogWnd(CString strCaption, CWnd* pParent, BOOL bIsFollow)
{
	DLGTEMPLATE* pDlgTemplate = NULL;
	BOOL bResult			  = FALSE;
	m_pParent				  = pParent;
	m_bIsFollow				  = bIsFollow;
	pDlgTemplate			  = this->CreateDlgTemplate(strCaption, WS_OVERLAPPED);
	bResult					  = CreateIndirect(pDlgTemplate, pParent);
	return bResult;
}

//Ĭ�����أ�ʹ����Ҫshow
void CLogWnd::Show()
{
	this->ShowWindow(SW_SHOW);
	SetTimer(1,3700, NULL);
}

void CLogWnd::Hidden()
{
	this->ShowWindow(SW_HIDE);
}

//���游����
void CLogWnd::MoveFollow()
{
	IGNORE_FIRST_TIME;
	if(m_bIsFollow)
	{
		CRect rect;
		m_pParent->GetWindowRect(&rect);
		//�޸Ĵ��ڵ�λ��
		//this->SetWindowPos(NULL,rect.right, rect.top, 0, 0, SWP_NOSIZE);
		this->SetWindowPos(NULL, rect.left+10, rect.bottom, 0, 0, SWP_NOSIZE);
	}
}

// �����־
void CLogWnd::Export2Log()
{
	/*CFile hFile;
	CString strFilePath;
	CString strDesktopPath;
	CString strLineText, strBuffer;
	int i, nLine, IdxStart, IdxEnd;*/
	
	//GetDesktopPath(strDesktopPath);
	//strFilePath = strDesktopPath + _T("\\ClogWnd.log");

	char firstName[MAXSIZE];
	memset(firstName, 0x00, MAXSIZE);
	/* 1.��ȡ�ļ��ľ���·�� */
	GetModuleFileName(AfxGetInstanceHandle(), firstName, MAXSIZE);

	/* 2.ƴ���ļ�  ���ļ����ļ����޸�Ϊhello.c */
	CString fileIniName = firstName;
	/* �����������"\"���� */
	int index = fileIniName.ReverseFind('\\');
	/* �����ҵ���λ�ý�����ȡ */
	fileIniName = fileIniName.Left(index);

	strFilePath = fileIniName + _T("\\ClogWnd.log");

	/* ��ӡ������Ϣ��firstName�д���ŵ�ǰ�ļ��ľ��Ե�ַ  ����Ϊ��d:\code\lxg\file.c*/
	//AfxMessageBox(strFilePath);
	
	try
	{
		hFile.Open(strFilePath, CFile::modeCreate | CFile::modeWrite);
		hFile.SeekToBegin();
		
		nLine = m_ctrlEdit.GetLineCount();
		for(i=0; i<nLine-1; i++)
		{
			IdxStart  = m_ctrlEdit.LineIndex(i);
			IdxEnd    = m_ctrlEdit.LineIndex(i+1)-1;
			m_ctrlEdit.SetSel(IdxStart, IdxEnd);

			strBuffer = m_ctrlEdit.GetSelText();
			strLineText.Format(("[%04d]  %s\r\n", i, strBuffer));
			hFile.Write(strLineText.GetBuffer(0), strLineText.GetLength());
		}
		hFile.Flush();
		hFile.Close();
	}
	catch(CFileException *e)
	{
		strBuffer.Format(_T("Failed!\r\nCode[%08d]"), e->m_cause);
		MessageBox(strBuffer);
		hFile.Abort();
		e->Delete();
	}
}

// ������ļ�
void CLogWnd::Export2Text()
{
	/*CFile hFile;
	CString strFilePath;
	CString strDesktopPath;
	CString strLineText, strBuffer;
	int i, nLine, IdxStart, IdxEnd;*/
	
	//GetDesktopPath(strDesktopPath);
	//strFilePath = strDesktopPath + _T("\\ClogWnd.txt");

	char firstName[MAXSIZE];
	memset(firstName, 0x00, MAXSIZE);
	/* 1.��ȡ�ļ��ľ���·�� */
	GetModuleFileName(AfxGetInstanceHandle(), firstName, MAXSIZE);

	/* 2.ƴ���ļ�  ���ļ����ļ����޸�Ϊhello.c */
	CString fileIniName = firstName;
	/* �����������"\"���� */
	int index = fileIniName.ReverseFind('\\');
	/* �����ҵ���λ�ý�����ȡ */
	fileIniName = fileIniName.Left(index);

	strFilePath = fileIniName + _T("\\ClogWnd.txt");

	//��ӡ������Ϣ��firstName�д���ŵ�ǰ�ļ��ľ��Ե�ַ  ����Ϊ��d:\code\lxg\file.c
	//AfxMessageBox(strFilePath);
	
	try
	{
		hFile.Open(strFilePath, CFile::modeCreate | CFile::modeWrite);
		hFile.SeekToBegin();
		
		nLine = m_ctrlEdit.GetLineCount();
		for(i=0; i<nLine-1; i++)
		{
			IdxStart	= m_ctrlEdit.LineIndex(i);
			IdxEnd		= m_ctrlEdit.LineIndex(i+1)-1;
			m_ctrlEdit.SetSel(IdxStart, IdxEnd);

			strLineText = m_ctrlEdit.GetSelText() + _T("\r\n");
			hFile.Write(strLineText.GetBuffer(0), strLineText.GetLength());
		}
		hFile.Flush();
		hFile.Close();
	}
	catch(CFileException *e)
	{
		strBuffer.Format(_T("Failed!\r\nCode[%08d]"), e->m_cause);
		MessageBox(strBuffer);
		hFile.Abort();
		e->Delete();
	}	
}

//��ʼ������
BOOL CLogWnd::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_hIcon)
	{
		SetIcon(m_hIcon, TRUE);			// Set big icon
		SetIcon(m_hIcon, FALSE);		// Set small icon
	}

	// ������ˢ
	m_hBkbrush.CreateSolidBrush(0x000000);

	// Reset window style
	int WS_EX_LAYERED_NEW = 0x80000;  //ԭ������û�м�����NEW
	this->ModifyStyle(WS_CAPTION, 0);
	this->ModifyStyleEx(0, WS_EX_LAYERED_NEW);
	this->ModifyStyleEx(WS_EX_DLGMODALFRAME, 0);
	SETLAYEREDWINDOWATTRIBUTES SetLayeredWindowAttributes;
	HINSTANCE hInst = LoadLibrary(_T("USER32.DLL"));
	if(hInst != NULL)
	{
		SetLayeredWindowAttributes = (SETLAYEREDWINDOWATTRIBUTES)GetProcAddress(hInst, 
														"SetLayeredWindowAttributes");
		SetLayeredWindowAttributes(m_hWnd, RGB(0, 0, 0), 194, 0x02);
	}

	// Resize Window
	CRect rect;
	m_pParent->GetWindowRect(&rect);
	//this->SetWindowPos(NULL, 0, 0, 300, rect.Height(), SWP_NOMOVE | SWP_HIDEWINDOW);  // follow mode, default
	this->SetWindowPos(NULL, 0, 0, rect.Width()-20,80,0);  // follow mode, default
	if(!m_bIsFollow)
	{
		this->SetWindowPos(NULL, 0, 0, 300, 400, SWP_HIDEWINDOW);   // non-follow mode
	}

	// Create edit control
	CRect itemRect;
	this->GetWindowRect(&rect);
	itemRect.top    = 0;
	itemRect.left   = 0;
	itemRect.right  = rect.Width();
	itemRect.bottom = rect.Height();
	itemRect.InflateRect(-5, -5);
	if (!m_ctrlEdit.Create(WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL, itemRect, this, 2))
	{
		return FALSE;
	}
	
	// Set edit style
	CHARFORMAT cf;
	m_ctrlEdit.SetReadOnly(TRUE);
	m_ctrlEdit.LimitText(-1);
	//m_ctrlEdit.SetBackgroundColor(FALSE, 0x000000);
	m_ctrlEdit.SetBackgroundColor(FALSE, 0x0F0F0F);
	m_ctrlEdit.GetDefaultCharFormat(cf);
	cf.dwEffects  &= ~CFE_AUTOCOLOR;
	cf.dwEffects  &= ~CFE_BOLD;
	cf.yHeight     = 190;
	cf.crTextColor = m_clrInfo;
    strcpy((char *)cf.szFaceName ,"Courier New");     //ԭ��������仰����ע���ˣ���Ҫ�����Ǹ�������
	m_ctrlEdit.SetDefaultCharFormat(cf);

	return TRUE;
}

// Create dialog template
DLGTEMPLATE* CLogWnd::CreateDlgTemplate(CString strCaption, DWORD dwStyle, DWORD dwStyleEx)
{
	WORD* pBuffer				  = m_pBuffer;
	DLGTEMPLATE* pDlgTemplate	  = (DLGTEMPLATE*)pBuffer;

	// Dialog template	
	pDlgTemplate->dwExtendedStyle = dwStyleEx;
	pDlgTemplate->style			  = dwStyle; 
	pDlgTemplate->cdit			  = 0;
	pDlgTemplate->cx			  = 0;
	pDlgTemplate->cy			  = 0;
	pDlgTemplate->x				  = 0;
	pDlgTemplate->y				  = 0;
	pBuffer                       = (WORD*)(pDlgTemplate+1);

	// Menu
	*(pBuffer++) = 0;

	// Class
	*(pBuffer++) = 0;

	// Caption
	wcscpy((WCHAR*)pBuffer, strCaption.AllocSysString());
	pBuffer += strCaption.GetLength()+1;

	// Font
	// Ignore...
	
	return pDlgTemplate;
}

//����log�����Ϣ
void CLogWnd::AddString(CString str, COLORREF color)
{
	// Set text format
	CHARFORMAT cf;
	m_ctrlEdit.GetDefaultCharFormat(cf);
    cf.crTextColor = color;
   	m_ctrlEdit.SetWordCharFormat(cf);
	
	// Add text
    m_ctrlEdit.SetSel(m_ctrlEdit.GetWindowTextLength(), -1);
	m_ctrlEdit.ReplaceSel(str + _T("\r\n"));
	m_ctrlEdit.PostMessage(WM_VSCROLL, SB_BOTTOM);
}

// Data exchange
void CLogWnd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

// Message function, reset window color
HBRUSH CLogWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if(nCtlColor == CTLCOLOR_DLG)
	{
		return m_hBkbrush;
	}
	return hbr;
}

// Message function, IDOK
void CLogWnd::OnOK()
{

}

void CLogWnd::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//��ʱ����Ϣ����

	switch (nIDEvent)
	{
		case 0:   //����ʱ�ر���־���ڶ�ʱ��
		{
			Hidden();
			break;
		}
		default:
			break;
	}

	CDialog::OnTimer(nIDEvent);
}
