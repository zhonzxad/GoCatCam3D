#include "stdafx.h"
#include "SetUseLogObj.h"


SetUseLogObj::SetUseLogObj()
{
}


SetUseLogObj::~SetUseLogObj()
{
}

void SetUseLogObj::SetINFO(CString str)
{
	m_ShowObj.INFO(str);
}

void SetUseLogObj::SetWARN(CString str)
{
	m_ShowObj.WARN(str);
}

void SetUseLogObj::SetERR(CString str)
{
	m_ShowObj.ERR(str);
}
