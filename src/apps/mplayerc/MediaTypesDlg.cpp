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

// MediaTypesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "MediaTypesDlg.h"
#include "..\..\DSUtil\DSUtil.h"
#include "..\..\..\include\moreuuids.h"
#include "..\..\svplib\svplib.h"

// CMediaTypesDlg dialog

//IMPLEMENT_DYNAMIC(CMediaTypesDlg, CResizableDialog)
CMediaTypesDlg::CMediaTypesDlg(IGraphBuilderDeadEnd* pGBDE, CWnd* pParent /*=NULL*/)
	: CResizableDialog(CMediaTypesDlg::IDD, pParent)
	, m_pGBDE(pGBDE)
	, m_nomoreport(FALSE)
{
	m_subtype = GUID_NULL;
	m_type = UNKNOWN;
}

CMediaTypesDlg::~CMediaTypesDlg()
{
}

void CMediaTypesDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_pins);
	DDX_Control(pDX, IDC_EDIT1, m_report);
	DDX_Check(pDX, IDC_CHECK1, m_nomoreport);
}

void CMediaTypesDlg::AddLine(CString str)
{
	str.Replace(_T("\n"), _T("\r\n"));
	int len = m_report.GetWindowTextLength();
	m_report.SetSel(len, len, TRUE);
	m_report.ReplaceSel(str);
}

void CMediaTypesDlg::AddMediaType(AM_MEDIA_TYPE* pmt)
{
	m_subtype = pmt->subtype;
	if(pmt->majortype == MEDIATYPE_Video) m_type = VIDEO;
	else if(pmt->majortype == MEDIATYPE_Audio) m_type = AUDIO;
	else m_type = UNKNOWN;

	CAtlList<CString> sl;
	CMediaTypeEx(*pmt).Dump(sl);
	POSITION pos = sl.GetHeadPosition();
	while(pos) AddLine(sl.GetNext(pos) + '\n');
}

BEGIN_MESSAGE_MAP(CMediaTypesDlg, CResizableDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDOK, &CMediaTypesDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CMediaTypesDlg message handlers
BOOL CMediaTypesDlg::PreCreateWindow(CREATESTRUCT& cs){
	AppSettings & s = AfxGetAppSettings();
	if(!s.fReportFailedPins){
		cs.style &= ~WS_VISIBLE;
		cs.style &= ~WS_POPUP;
	}
	return __super::PreCreateWindow(cs);
}
BOOL CMediaTypesDlg::OnInitDialog()
{
	__super::OnInitDialog();

	CAtlList<CStringW> path;
	CAtlList<CMediaType> mts;

	CString szDeadPin;
	CString szReport;

	for(int i = 0; S_OK == m_pGBDE->GetDeadEnd(i, path, mts); i++)
	{
		if(!path.GetCount()) continue;
		szDeadPin.Append( path.GetTail() + _T("\n"));
		m_pins.SetItemData(m_pins.AddString(CString(path.GetTail())), (DWORD_PTR)i);
	}

	m_pins.SetCurSel(0);
	OnCbnSelchangeCombo1();

	m_report.GetWindowText(szReport);

	AppSettings & s = AfxGetAppSettings();
//	if(s.fUploadFailedPinsInfo)
//		SVP_UploadPinRenderDeadEnd(szDeadPin, szReport);

	
	if(!s.fReportFailedPins){
		OnOK();
		return FALSE;
	}
	AddAnchor(IDC_STATIC1, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_STATIC2, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_COMBO1, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_EDIT1, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDC_CHECK1, BOTTOM_LEFT);
	SetMinTrackSize(CSize(300, 200));

	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMediaTypesDlg::OnCbnSelchangeCombo1()
{
	m_report.SetWindowText(_T(""));

	int i = m_pins.GetCurSel();
	if(i < 0) return;

	CAtlList<CStringW> path;
	CAtlList<CMediaType> mts;

	if(FAILED(m_pGBDE->GetDeadEnd(i, path, mts)) || !path.GetCount()) 
		return;

	POSITION pos = path.GetHeadPosition();
	while(pos)
	{
		AddLine(CString(path.GetNext(pos)) + _T("\n"));
		if(!pos) AddLine(_T("\n"));
	}

	pos = mts.GetHeadPosition();
	for(int j = 0; pos; j++)
	{
		CString str;
		str.Format(_T("Media Type %d:\n"), j);
		AddLine(str);
		AddLine(_T("--------------------------\n"));
		AddMediaType(&mts.GetNext(pos));
		AddLine();
	}

	m_report.SetSel(0, 0);
}

void CMediaTypesDlg::OnBnClickedOk()
{
	UpdateData();
	if(m_nomoreport){
		AppSettings & s = AfxGetAppSettings();
		s.fReportFailedPins = FALSE;
	}
	OnOK();
}
