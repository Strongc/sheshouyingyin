/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

// PPageBase.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "PPageBase.h"

// CPPageBase dialog

IMPLEMENT_DYNAMIC(CPPageBase, CCmdUIPropertyPage)
CPPageBase::CPPageBase(UINT nIDTemplate, UINT nIDCaption)
	: CCmdUIPropertyPage(nIDTemplate, nIDCaption)
{
}

CPPageBase::~CPPageBase()
{
}

void CPPageBase::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
}

void CPPageBase::CreateToolTip()
{
	m_wndToolTip.Create(this);
	m_wndToolTip.Activate(TRUE);
	m_wndToolTip.SetMaxTipWidth(300);
	m_wndToolTip.SetDelayTime(TTDT_AUTOPOP, 10000);
	for(CWnd* pChild = GetWindow(GW_CHILD); pChild; pChild = pChild->GetWindow(GW_HWNDNEXT))
	{
		CString strToolTip;
		if(strToolTip.LoadString(pChild->GetDlgCtrlID()))
			m_wndToolTip.AddTool(pChild, strToolTip);
	}
}

BOOL CPPageBase::PreTranslateMessage(MSG* pMsg)
{
	if(IsWindow(m_wndToolTip))
	if(pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)
	{
		MSG msg;
		memcpy(&msg, pMsg, sizeof(MSG));
		for(HWND hWndParent = ::GetParent(msg.hwnd); 
			hWndParent && hWndParent != m_hWnd;
			hWndParent = ::GetParent(hWndParent))
		{
			msg.hwnd = hWndParent;
		}

		if(msg.hwnd)
		{
			m_wndToolTip.RelayEvent(&msg);
		}
	}

	return __super::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CPPageBase, CCmdUIPropertyPage)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CPPageBase message handlers

BOOL CPPageBase::OnSetActive()
{
	AfxGetApp()->WriteProfileInt(ResStr(IDS_R_SETTINGS), _T("LastUsedPage"), (UINT)m_pPSP->pszTemplate);

	return __super::OnSetActive();
}

void CPPageBase::OnDestroy()
{
	__super::OnDestroy();

	m_wndToolTip.DestroyWindow();
}