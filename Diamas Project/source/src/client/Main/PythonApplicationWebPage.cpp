#include "StdAfx.h"
#include "PythonApplication.h"
#include "../EterBase/vk.h"

bool CPythonApplication::IsWebPageMode()
{
    return true;
}

void CPythonApplication::ShowWebPage(const char *c_szURL, const RECT &c_rcWebPage)
{
}

void CPythonApplication::MoveWebPage(const RECT &c_rcWebPage)
{
}

void CPythonApplication::HideWebPage()
{
}
