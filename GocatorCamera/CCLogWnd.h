#ifndef __CLOGWND_H__ZZ_153__2019_03_15__
#define __CLOGWND_H__ZZ_153__2019_03_15__

#if		  _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
//! CLogEdit window
class CLogEdit : public CRichEditCtrl
{
public:
	//! Constructor
	CLogEdit(){};

	//! Destructor
	virtual ~CLogEdit(){};


protected:
	//! Message function, mouse wheel response
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	//! Message map
	DECLARE_MESSAGE_MAP()
};

// CLogWnd window
class CLogWnd : public CDialogEx
{
private:
	//! Source
	enum { IDICON=1 };   
	HICON			m_hIcon;

	//! Background brush
	CBrush			m_hBkbrush;

	//! String color
	COLORREF		m_clrInfo;
	COLORREF		m_clrWarn;
	COLORREF		m_clrError;

	//! Parent window
	CWnd*			m_pParent;

	//! Edit control
	CLogEdit		m_ctrlEdit;

	//! Template buffer 
	WORD*			m_pBuffer;

	//! @TRUE	Means 'CLogWnd' attached to the parent window.
	BOOL			m_bIsFollow;
	
	//! @TRUE	Means 'CLogWnd' is hidden
	BOOL			m_bIsHidden;


public:
	//! Constructor
	CLogWnd();
	
	//! Destructor
	virtual ~CLogWnd();

	//! Create window
	//! @param strCaption		Window name
	//! @param pParent			Parent window pointer, if it's 'NULL', m_bIsFollow is 'FALSE'
	//! @param bIsFollow		If 'TRUE', 'CLogWnd' attached to the parent window	
	BOOL CreateLogWnd(CString strCaption, CWnd* pParent=NULL,  BOOL bIsFollow=TRUE);
	
	//! Show window
	void Show();

	//! Hidden window
	void Hidden();

	//! Follow parent window
	void MoveFollow();

	//! Export to log
	void Export2Log();

	//! Export to text
	void Export2Text();
	
	//! AddString #Log Info
	void INFO(CString strInfo)
	{
		Show();
		AddString(strInfo, m_clrInfo);
		//Sleep(10000);
		//Hidden();
		SetTimer(0, 3700, NULL);
	}

	//! AddString #Log warn
	void WARN(CString strWarn)
	{
		Show();
		AddString(strWarn, m_clrWarn);
		//Sleep(10000);
		//Hidden();
		SetTimer(0, 3700, NULL);
	}

	//! AddString #Log error
	void ERR(CString strErr)
	{
		Show();
		AddString(strErr, m_clrError);
		//Sleep(10000);
		//Hidden();
		SetTimer(0, 3700, NULL);
	}



	CFile hFile;
	CString strFilePath;
	CString strDesktopPath;
	CString strLineText, strBuffer;
	int i, nLine, IdxStart, IdxEnd;


private:
	//! Init dialog
	virtual BOOL OnInitDialog();

	//! Create dialog template
	DLGTEMPLATE* CreateDlgTemplate(CString strCaption, DWORD dwStyle, DWORD dwStyleEx=0);

	//! Add String
	void AddString(CString str, COLORREF color);
	
	
protected:
	//! Data exchange
	virtual void DoDataExchange(CDataExchange* pDX);

	//! Message function, reset window color 
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	//! Message function, IDOK
	virtual void OnOK();
	
	//! Message map
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};



#endif /* __CLOGWND_H__ZZ_153__2019_03_15__ */
