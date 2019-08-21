#pragma once
#include "CCLogWnd.h"

class SetUseLogObj
{
public:
	SetUseLogObj();
	~SetUseLogObj();

	void SetINFO(CString str);
	void SetWARN(CString str);
	void SetERR(CString str);

	CLogWnd m_ShowObj;
};

