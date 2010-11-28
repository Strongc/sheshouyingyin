﻿/* 
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

#include "stdafx.h"
#include <math.h>
#include <afxinet.h>
#include "..\..\..\lib\ATL Server\include\atlrx.h"
#include <atlutil.h>
#include "mplayerc.h"
#include "mainfrm.h"
#include "..\..\DSUtil\DSUtil.h"
#include "SaveTextFileDialog.h"
#include ".\playerplaylistbar.h"
#include "../../svplib/svplib.h"
#include "../../svplib/SVPToolBox.h"
#include "../../svplib/SVPRarLib.h"
#include "OpenFileDlg.h"
#include <Strings.h>
#include "UserInterface/Renderer/PlaylistView_Win.h"

#include "Utils/ContentType.h"
#include "Controller\PlayerPreference.h"
#include "Controller\SPlayerDefs.h"

IMPLEMENT_DYNAMIC(CPlayerPlaylistBar, CSizingControlBarG)
CPlayerPlaylistBar::CPlayerPlaylistBar()
	: m_list(0)
	, m_nTimeColWidth(0)
	, m_csDataLock(0),
  m_caption_height(::GetSystemMetrics(SM_CYICON)*7/8),
  m_bottom_height(::GetSystemMetrics(SM_CYICON)),
  m_button_height(::GetSystemMetrics(SM_CYICON)*2/3),
  m_padding(::GetSystemMetrics(SM_CXICON)/4),
  m_entry_height(::GetSystemMetrics(SM_CYSMICON)*5/4),
  m_entry_padding(::GetSystemMetrics(SM_CYSMICON)/8),
  m_basecolor(RGB(78,78,78)),
  m_basecolor2(RGB(32,32,32)),
  m_basecolor3(RGB(128,128,128)),
  m_basecolor4(RGB(192,192,192)),
  m_textcolor(RGB(255,255,255)),
  m_textcolor_hilite(RGB(255,200,20))
{
	m_csDataLock = new CCritSec();
	m_bDragging = FALSE;

  //////////////////////////////////////////////////////////////////////////

  WTL::CLogFont lf;
  lf.SetMessageBoxFont();
  m_font_normal.CreateFontIndirect(&lf);
  lf.SetBold();
  m_font_bold.CreateFontIndirect(&lf);
  lf.SetMessageBoxFont();
  wcscpy_s(lf.lfFaceName, 32, L"Webdings");
  lf.lfHeight = lf.lfHeight*5/4;
  m_font_symbol.CreateFontIndirect(&lf);

  WTL::CString text;
  text.LoadString(IDS_PLAYLIST);
  Strings::Split(text, L"|", m_texts);

  m_br_list.CreateSolidBrush(m_basecolor);

  //////////////////////////////////////////////////////////////////////////
}

CPlayerPlaylistBar::~CPlayerPlaylistBar()
{
}

#define CBUTTONWIDTH 50
#define CBUTTONHEIGHT 22

BOOL CPlayerPlaylistBar::Create(CWnd* pParentWnd)
{
	if(!CSizingControlBarG::Create(_T("Playlist"), pParentWnd, 0))
		return FALSE;

	AppSettings& s = AfxGetAppSettings();

	m_list.CreateEx(
		/*WS_EX_DLGMODALFRAME|WS_EX_CLIENTEDGE*/0, 
		WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_TABSTOP
			|LVS_OWNERDRAWFIXED
			|LVS_NOCOLUMNHEADER
			|LVS_EDITLABELS
			|LVS_REPORT|LVS_AUTOARRANGE|LVS_NOSORTHEADER, // TODO: remove LVS_SINGLESEL and implement multiple item repositioning (dragging is ready)
		CRect(0,0,100,80), this, IDC_PLAYLIST);

	m_list.SetExtendedStyle(m_list.GetExtendedStyle()|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);

	m_list.InsertColumn(COL_NAME, ResStr(IDS_PLAYLIST_COL_HEADER_FILENAME), LVCFMT_LEFT, 380);

	//m_list.SetBkColor(s.GetColorFromTheme(_T("PlayListBG"), 0xdddddd));
  m_list.SetBkColor(m_basecolor);
	CDC* pDC = m_list.GetDC();
	CFont* old = pDC->SelectObject(GetFont());
	m_nTimeColWidth = pDC->GetTextExtent(_T("000:00:00")).cx + 5;
	pDC->SelectObject(old);
	m_list.ReleaseDC(pDC);
	m_list.InsertColumn(COL_TIME, ResStr(IDS_PLAYLIST_COL_HEADER_FILE_LENGTH), LVCFMT_RIGHT, m_nTimeColWidth);

    m_fakeImageList.Create(1, 16, ILC_COLOR4, 10, 10);
	m_list.SetImageList(&m_fakeImageList, LVSIL_SMALL);
	this->m_pMaindFrame = pParentWnd;
	m_clearall.Create( ResStr(IDS_PLAYLIST_BUTTON_CLEAN), WS_VISIBLE|WS_CHILD|BS_FLAT|BS_VCENTER|BS_CENTER , CRect(0,83,60,100), this, IDC_BUTTONCLEARALL );
	GetSystemFontWithScale(&font);
	m_clearall.SetFont(&font);

// 	m_addsubforplaylist.Create( ResStr(IDS_PLAYLIST_BUTTON_LOAD_SUB_FOR_PLAYLIST), WS_VISIBLE|WS_CHILD|BS_FLAT|BS_VCENTER|BS_CENTER , CRect(0,83,40,100), this, IDC_BUTTONADDSUBFORPLAYLIST );
// 	m_addsubforplaylist.SetFont(&font);
	return TRUE;
}

BOOL CPlayerPlaylistBar::PreCreateWindow(CREATESTRUCT& cs)
{
	if(!CSizingControlBarG::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_ACCEPTFILES;

	return TRUE;
}

BOOL CPlayerPlaylistBar::PreTranslateMessage(MSG* pMsg)
{
	if(IsWindow(pMsg->hwnd) && IsVisible() && pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		if(IsDialogMessage(pMsg))
			return TRUE;
	}

	return CSizingControlBarG::PreTranslateMessage(pMsg);
}

bool FindFileInList(CAtlList<CString>& sl, CString fn)
{
	bool fFound = false;
	POSITION pos = sl.GetHeadPosition();
	while(pos && !fFound) {if(!sl.GetNext(pos).CompareNoCase(fn)) fFound = true;}
	return(fFound);
}

void CPlayerPlaylistBar::AddItem(CString fn, CAtlList<CString>* subs)
{
	CAtlList<CString> sl;
	sl.AddTail(fn);
	AddItem(sl, subs);
}


void CPlayerPlaylistBar::AddItem(CAtlList<CString>& fns, CAtlList<CString>* subs)
{
	CPlaylistItem pli;
  PlayerPreference* pref = PlayerPreference::GetInstance();
	AppSettings& s = AfxGetAppSettings();
	CMediaFormats& mf = s.Formats;

	POSITION pos = fns.GetHeadPosition();
	while(pos)
	{
		POSITION cur = pos;
		CString fn = fns.GetNext(pos);
		
			
		if( mf.IsUnPlayableFile(fn) ){
			fns.RemoveAt(cur);
			continue;
		}
		
		if(!fn.Trim().IsEmpty()) pli.m_fns.AddTail(fn);
	}

	if(subs)
	{
		POSITION pos = subs->GetHeadPosition();
		while(pos)
		{
			CString fn = subs->GetNext(pos);
			if(!fn.Trim().IsEmpty()) pli.m_subs.AddTail(fn);
		}
	}

	if(pli.m_fns.IsEmpty()) return;

	
		CString fn = pli.m_fns.GetHead();

		if(pref->GetIntVar(INTVAR_AUTOLOADAUDIO) && fn.Find(_T("://")) < 0)
		{
			int i = fn.ReverseFind('.');
			if(i > 0)
			{
				//CMediaFormats& mf = AfxGetAppSettings().Formats;

				CString ext = fn.Mid(i+1).MakeLower();

				if(!mf.FindExt(ext, true))
				{
					

					CString path = fn;
					path.Replace('/', '\\');
					path = path.Left(path.ReverseFind('\\')+1);

					WIN32_FIND_DATA fd = {0};
					HANDLE hFind = FindFirstFile(fn.Left(i) + _T("*.*"), &fd);
					if(hFind != INVALID_HANDLE_VALUE)
					{
						do
						{
							if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) continue;

							CString fullpath = path + fd.cFileName;
							CString ext2 = fullpath.Mid(fullpath.ReverseFind('.')+1).MakeLower();
							if(!FindFileInList(pli.m_fns, fullpath) && ext != ext2 
							&& mf.FindExt(ext2, true) && mf.IsUsingEngine(fullpath, DirectShow))
							{
								pli.m_fns.AddTail(fullpath);
							}
						}
						while(FindNextFile(hFind, &fd));
						
						FindClose(hFind);
					}
				}
			}
		}
	
	//if(AfxGetAppSettings().fAutoloadSubtitles) //always autoload subtitles
	/*
{
		CAtlArray<CString> paths;
		paths.Add(_T("."));
		paths.Add(_T(".\\subtitles"));
		paths.Add(_T(".\\Subs"));
		paths.Add(_T("c:\\subtitles"));
		paths.Add(s.GetSVPSubStorePath());
		paths.Add();

		CAtlArray<SubFile> ret;
		
		GetSubFileNames(fn, paths, ret);

		for(size_t i = 0; i < ret.GetCount(); i++)
		{
			if(!FindFileInList(pli.m_subs, ret[i].fn))
				pli.m_subs.AddTail(ret[i].fn);
		}
	}
*/

	m_pl.AddTail(pli);
}


static bool SearchFiles(CString mask, CAtlList<CString>& sl)
{
	if(mask.Find(_T("://")) >= 0)
		return(false);

	mask.Trim();
	sl.RemoveAll();

	CMediaFormats& mf = AfxGetAppSettings().Formats;

	bool fFilterKnownExts;
	WIN32_FILE_ATTRIBUTE_DATA fad;
	mask = (fFilterKnownExts = (GetFileAttributesEx(mask, GetFileExInfoStandard, &fad) 
							&& (fad.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)))
		? CString(mask).TrimRight(_T("\\/")) + _T("\\*.*")
		: mask;

	{
		CString dir = mask.Left(max(mask.ReverseFind('\\'), mask.ReverseFind('/'))+1);

		WIN32_FIND_DATA fd;
		HANDLE h = FindFirstFile(mask, &fd);
		if(h != INVALID_HANDLE_VALUE)
		{
			do
			{
				if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) continue;

				CString fn = fd.cFileName;
				CString ext = fn.Mid(fn.ReverseFind('.')+1).MakeLower();
				CString path = dir + fd.cFileName;

				if(!fFilterKnownExts || mf.FindExt(ext))
					sl.AddTail(path);
			}
			while(FindNextFile(h, &fd));
			
			FindClose(h);

			if(sl.GetCount() == 0 && mask.Find(_T(":\\")) == 1)
			{
				GetCDROMType(mask[0], sl);
			}
		}
	}

	return(sl.GetCount() > 1
		|| sl.GetCount() == 1 && sl.GetHead().CompareNoCase(mask)
		|| sl.GetCount() == 0 && mask.FindOneOf(_T("?*")) >= 0);
}


bool CPlayerPlaylistBar::ParseBDMVPlayList(CString fn)
{
	CHdmvClipInfo		ClipInfo;
	CString				strPlaylistFile;
	CAtlList<CHdmvClipInfo::PlaylistItem>	MainPlaylist;

	CPath Path(fn);
	Path.RemoveFileSpec();
	Path.RemoveFileSpec();

	if (SUCCEEDED (ClipInfo.FindMainMovie (Path + L"\\", strPlaylistFile, MainPlaylist)))
	{
		CAtlList<CString>		strFiles;
		strFiles.AddHead (strPlaylistFile);
		Append(strFiles, MainPlaylist.GetCount()>1, NULL);
	}

	return m_pl.GetCount() > 0;
}

void CPlayerPlaylistBar::ParsePlayList(CString fn, CAtlList<CString>* subs)
{
	CAtlList<CString> sl;
	sl.AddTail(fn);
    
	ParsePlayList(sl, subs);
    
}

void CPlayerPlaylistBar::ParsePlayList(CAtlList<CString>& fns, CAtlList<CString>* subs)
{
	if(fns.IsEmpty()) return;

	AppSettings& s = AfxGetAppSettings();

	// resolve rar file
	{
		CMediaFormats& mf = s.Formats;

		CSVPRarLib svpRar;
		POSITION pos = fns.GetHeadPosition();
		while( pos)
		{
			POSITION cur = pos;
			CString fn = fns.GetNext(pos);
			if(fn.Left(6) == _T("rar://"))
				continue;

			if( CPath(fn).GetExtension().MakeLower() == _T(".rar")  ){
				CStringArray szFnsInRar;
				svpRar.ListRar(fn, &szFnsInRar);

				fns.RemoveAt(cur);
				//AfxMessageBox(fn);
				for(int i = 0; i < szFnsInRar.GetCount();i++){
					//detect if its known file type
					CString szThisFn = szFnsInRar.GetAt(i);
					if(!mf.IsUnPlayableFile( szThisFn , true ) &&  CPath(szThisFn).GetExtension().MakeLower() != _T(".rar") ){
						AddItem(CString(_T("rar://")) + fn + _T("?") + szThisFn , subs);
						//AfxMessageBox(szThisFn);
					}
				}
			}

			
		}
	}
    
	if(fns.GetCount() <= 0){
     return;
	}
//AfxMessageBox(_T("Done"));
	// resolve .lnk files

	CComPtr<IShellLink> pSL;
	pSL.CoCreateInstance(CLSID_ShellLink);
	CComQIPtr<IPersistFile> pPF = pSL;

	POSITION pos = fns.GetHeadPosition();
	while(pSL && pPF && pos)
	{
		CString& fn = fns.GetNext(pos);
		TCHAR buff[MAX_PATH];
		if(CPath(fn).GetExtension().MakeLower() != _T(".lnk")
		|| FAILED(pPF->Load(CStringW(fn), STGM_READ))
		|| FAILED(pSL->Resolve(NULL, SLR_ANY_MATCH|SLR_NO_UI))
		|| FAILED(pSL->GetPath(buff, countof(buff), NULL, 0)))
			continue;

		fn = buff;
	}

	//	
	CAtlList<CString> sl;
	if(SearchFiles(fns.GetHead(), sl))
	{
		if(sl.GetCount() > 1) subs = NULL;
		POSITION pos = sl.GetHeadPosition();
		while(pos) ParsePlayList(sl.GetNext(pos), subs);
		return;
	}
	
  std::vector<std::wstring> redir;
  std::wstring ct = ContentType::Get(fns.GetHead(), &redir);
  if (!redir.empty())
  {
    for (std::vector<std::wstring>::iterator iter = redir.begin();
      iter != redir.end(); iter++)
      ParsePlayList(iter->c_str(), subs);
    return;
  }

  if(ct == L"application/x-cue-playlist")
  {
    ParseCUEPlayList(fns.GetHead());
    return;
  }
  if(ct == L"application/x-mpc-playlist")
  {
    ParseMPCPlayList(fns.GetHead());
    return;
  }
  else if(ct == L"application/x-bdmv-playlist")
  {
    ParseBDMVPlayList(fns.GetHead());
    return;
  }

  AddItem(fns, subs);
}

static int s_int_comp(const void* i1, const void* i2)
{
	return (int)i1 - (int)i2;
}

static CString CombinePath(CPath p, CString fn)
{
	if(fn.Find(':') >= 0 || fn.Find(_T("\\")) == 0) return fn;
	p.Append(CPath(fn));
	return (LPCTSTR)p;
}
bool CPlayerPlaylistBar::ParseCUEPlayList(CString fn)
{
	CString str;
	CAtlMap<int, CPlaylistItem> pli;
	CAtlArray<int> idx;

	CWebTextFile f;
	if(!f.Open(fn) || !f.ReadString(str) )
		return false;

	if(f.GetEncoding() == CTextFile::ASCII) 
		f.SetEncoding(CTextFile::ANSI);

	CPath base(fn);
	base.RemoveFileSpec();

	int idxCurrent = -1;

	CSVPToolBox svpToolBox;

	while(f.ReadString(str))
	{
		str.Trim();
		int pos = str.Find(_T("FILE"));
		if( pos >= 0){
			int pos2 = str.ReverseFind(' ');
			int pos3 = str.ReverseFind('\t');
			pos2 = max(pos2, pos3);

			CString fn = str.Mid(  pos+5, pos2-pos-5);
			fn.Trim();
			fn.Trim('"');

			if (!svpToolBox.ifFileExist( fn ) ){

				CString fnPath(PathFindFileName(fn.GetBuffer()));
				CPath myBase(base);
				myBase.RemoveBackslash();
				myBase.Append(fnPath);

				fn = CString(myBase);

				if (!svpToolBox.ifFileExist( fn ) ){
					continue;
				}
			}


			CPlaylistItem pli;
			pli.m_fns.AddTail( fn);
			m_pl.AddTail(pli );
		}
	}

	
	for(size_t i = 0; i < idx.GetCount(); i++){
		m_pl.AddTail(pli[idx[i]]);
		if(idxCurrent > 0 && idxCurrent == idx[i]){
			m_pl.SetPos( m_pl.GetTailPosition());
		}
	}

	return pli.GetCount() > 0;

}
bool CPlayerPlaylistBar::ParseMPCPlayList(CString fn)
{
	CString str;
	CAtlMap<int, CPlaylistItem> pli;
	CAtlArray<int> idx;

	CWebTextFile f;
	if(!f.Open(fn) || !f.ReadString(str) || str != _T("MPCPLAYLIST"))
		return false;

	if(f.GetEncoding() == CTextFile::ASCII) 
		f.SetEncoding(CTextFile::ANSI);

	CPath base(fn);
	base.RemoveFileSpec();

	int idxCurrent = -1;

	while(f.ReadString(str))
	{
		CAtlList<CString> sl;
		Explode(str, sl, ',', 3);
		if(sl.GetCount() != 3) continue;

		if(int i = _ttoi(sl.RemoveHead()))
		{
			CString key = sl.RemoveHead();
			CString value = sl.RemoveHead();

			if(key == _T("type")) {pli[i].m_type = (CPlaylistItem::type_t)_ttol(value); idx.Add(i);}
			else if(key == _T("label")) pli[i].m_label = value;
			else if(key == _T("iscurrent") ) { idxCurrent = i; }
			else if(key == _T("filename")) {value = CombinePath(base, value); pli[i].m_fns.AddTail(value);}
			else if(key == _T("subtitle")) {value = CombinePath(base, value); pli[i].m_subs.AddTail(value);}
			else if(key == _T("video")) {while(pli[i].m_fns.GetCount() < 2) pli[i].m_fns.AddTail(_T("")); pli[i].m_fns.GetHead() = value;}
			else if(key == _T("audio")) {while(pli[i].m_fns.GetCount() < 2) pli[i].m_fns.AddTail(_T("")); pli[i].m_fns.GetTail() = value;}
			else if(key == _T("vinput")) pli[i].m_vinput = _ttol(value);
			else if(key == _T("vchannel")) pli[i].m_vchannel = _ttol(value);
			else if(key == _T("ainput")) pli[i].m_ainput = _ttol(value);
			else if(key == _T("country")) pli[i].m_country = _ttol(value);			
		}
	}

	qsort(idx.GetData(), idx.GetCount(), sizeof(int), s_int_comp);
	for(size_t i = 0; i < idx.GetCount(); i++){
		m_pl.AddTail(pli[idx[i]]);
		if(idxCurrent > 0 && idxCurrent == idx[i]){
			m_pl.SetPos( m_pl.GetTailPosition());
		}
	}

	return pli.GetCount() > 0;
}

bool CPlayerPlaylistBar::SaveMPCPlayList(CString fn, CTextFile::enc e, bool fRemovePath)
{
	CTextFile f;
	if(!f.Save(fn, e))
		return false;

	f.WriteString(_T("MPCPLAYLIST\n"));

	POSITION posCur = m_pl.GetPos();
	
	POSITION pos = m_pl.GetHeadPosition(), pos2;
	for(int i = 1; pos; i++)
	{
		BOOL bCurrentFile = false;
		if(posCur && posCur == pos){
			bCurrentFile = true;
		}
		CPlaylistItem& pli = m_pl.GetNext(pos);

		CString idx;
		idx.Format(_T("%d"), i);

		CString str;
		str.Format(_T("%d,type,%d"), i, pli.m_type);
		f.WriteString(str + _T("\n"));
		
		if(!pli.m_label.IsEmpty()) 
			f.WriteString(idx + _T(",label,") + pli.m_label + _T("\n"));

		if(bCurrentFile){
			f.WriteString(idx + _T(",iscurrent,1\n"));
		}
		if(pli.m_type == CPlaylistItem::file)
		{
			pos2 = pli.m_fns.GetHeadPosition();
			while(pos2)
			{
				CString fn = pli.m_fns.GetNext(pos2);
				if(fRemovePath) {CPath p(fn); p.StripPath(); fn = (LPCTSTR)p;}
				f.WriteString(idx + _T(",filename,") + fn + _T("\n"));
			}

			pos2 = pli.m_subs.GetHeadPosition();
			while(pos2)
			{
				CString fn = pli.m_subs.GetNext(pos2);
				if(fRemovePath) {CPath p(fn); p.StripPath(); fn = (LPCTSTR)p;}
				f.WriteString(idx + _T(",subtitle,") + fn + _T("\n"));
			}
		}
		else if(pli.m_type == CPlaylistItem::device && pli.m_fns.GetCount() == 2)
		{
			f.WriteString(idx + _T(",video,") + pli.m_fns.GetHead() + _T("\n"));
			f.WriteString(idx + _T(",audio,") + pli.m_fns.GetTail() + _T("\n"));
			str.Format(_T("%d,vinput,%d"), i, pli.m_vinput);
			f.WriteString(str + _T("\n"));
			str.Format(_T("%d,vchannel,%d"), i, pli.m_vchannel);
			f.WriteString(str + _T("\n"));
			str.Format(_T("%d,ainput,%d"), i, pli.m_ainput);
			f.WriteString(str + _T("\n"));
			str.Format(_T("%d,country,%d"), i, pli.m_country);
			f.WriteString(str + _T("\n"));
		}
	}

	return true;
}
void CPlayerPlaylistBar::CheckForPlaylistSubtitle(){
	
	if( m_pl.GetCount() > 1 ){ //playlist 包含多文件
		BOOL bHasSubtitles = 0 , bAllInSameDir = 1; //检查是否有playlist字幕
		int iFileTotal = 0;
		CString szDir, szBuf;
		POSITION pos = m_pl.GetHeadPosition();
		CSVPToolBox svpTool;
		while(pos){
			CPlaylistItem pli = m_pl.GetNext(pos);
			POSITION pos1 = pli.m_fns.GetHeadPosition();
			while(pos1){
				iFileTotal++;
				szBuf = pli.m_fns.GetNext(pos1);
				szBuf = svpTool.GetDirFromPath( szBuf );
				if(szDir.IsEmpty()){
					szDir = szBuf;
				}else if(szDir != szBuf){
					//Not all in same dir
					bAllInSameDir = 0;
					break;
				}
			}

			if ( pli.m_subs.GetCount() > 0 ){
				bHasSubtitles = 1;
				break;
			}
		}    
/* 不好用
		if(!bHasSubtitles && bAllInSameDir && iFileTotal > 1){ 
			//Playlist的所在文件目录中是否有字幕文件

			CAtlArray<CString> paths;
			paths.Add(_T("."));
			
			CAtlArray<SubFile> ret;
			GetSubFileNames( szDir,paths, ret, 1);

			if(ret.GetCount() >= 1){
				//Set As Playlist Sub;
				SubFile xSub =  ret.GetAt(0);
				m_pl.szPlayListSub = xSub.fn;
			}
		}
*/
	}
}
void CPlayerPlaylistBar::OnUpdateButtonAddSubForPlayList(CCmdUI* pCmdUI){
	pCmdUI->Enable((m_pl.GetCount() > 1));
}
void CPlayerPlaylistBar::OnButtonAddSubForPlayList(){
	static TCHAR BASED_CODE szFilter[] = 
		_T(".srt .sub .ssa .ass .smi .psb .txt .idx .usf .xss .ssf|")
		_T("*.srt;*.sub;*.ssa;*.ass;*smi;*.psb;*.txt;*.idx;*.usf;*.xss;*.ssf|")
		_T("All files (*.*)|")
		_T("*.*||");

	CFileDialog fd(TRUE, NULL, NULL, 
		OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY, 
		szFilter, this, 0, 1);

	if(fd.DoModal() != IDOK) return;

	m_pl.szPlayListSub = fd.GetPathName();

	m_pl.SetPos(m_pl.GetHeadPosition());
	m_list.Invalidate();
	((CMainFrame*)AfxGetMainWnd())->OpenCurPlaylistItem(); 
}
void CPlayerPlaylistBar::Refresh()
{
	SetupList();
	ResizeListColumn();

	
}
	
void CPlayerPlaylistBar::Empty()
{
	m_pl.RemoveAll();
	m_list.DeleteAllItems();
	m_pl.szPlayListSub.Empty();
	SavePlaylist();

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if(pFrame){
		__try{
			pFrame->m_bAllowVolumeSuggestForThisPlaylist = true;
		}__except(EXCEPTION_EXECUTE_HANDLER){} 
	//pFrame->m_fLastIsAudioOnly = false;
	}
}
void CPlayerPlaylistBar::RealFindMoreFileFromOneFileAndPutIntoPlaylist(CString szMediaFile , CAtlList<CString>& szaIn ){
	
    SVP_LogMsg5(L"RealFindMoreFileFromOneFileAndPutIntoPlaylist");

	//check if dir has more files
	CAtlList<CString> szaRet;
	CAtlArray<CString> mask;
	bool noAudio = !AfxGetAppSettings().Formats.IsAudioFile(szMediaFile);

	AfxGetAppSettings().Formats.GetExtsArray(mask, noAudio);
	CSVPToolBox svptool;
	svptool.findMoreFileByFile(szMediaFile, szaRet, mask);

	//check is szaIn still in playlist
	BOOL bPlayListHavntChanged = true;
	POSITION pos = szaIn.GetHeadPosition();
	while(pos){
		CString szFile = szaIn.GetNext(pos);
		if(!szFile.IsEmpty() && FindPosByFilename(szFile) == NULL ){
			bPlayListHavntChanged = false;
		}
	}
	if(bPlayListHavntChanged){
		{
			//delete m_csDataLock;
			//m_csDataLock = new CCritSec();
			//CAutoLock dataLock(m_csDataLock);
			svptool.MergeAltList(szaRet, szaIn);

			POSITION pos = szaRet.GetHeadPosition();
			while(pos){
				POSITION cur = pos;
				if( FindPosByFilename(szaRet.GetNext(pos) , false) ){
					szaRet.RemoveAt(cur);
				}
			}
            
            if(szaRet.GetCount() > 0){
                m_pl.szPlayListSub.Empty();

                
                if(szaRet.GetCount() > 1)
                {
                    POSITION pos = szaRet.GetHeadPosition();
                
                    while(pos){
                        CString szFPath = szaRet.GetNext(pos);
                        if(!FindPosByFilename(szFPath))
                            ParsePlayList(szFPath, NULL);
                
                    }
                }
                else
                {
                    ParsePlayList(szaRet, NULL);
                }

            }

			  //add them all
			//POSITION posx = szaRet.GetHeadPosition();
			//while(posx) ParsePlayList(szaRet.GetNext(posx), NULL); 

		}
        
		POSITION pos = FindPosByFilename(szMediaFile);
		m_pl.SetPos(pos);
		m_pl.SortByName();
		Refresh();
		SavePlaylist();
        
	}
}

class CFFindMoreFiles{
public:
	CPlayerPlaylistBar* wndPlaylist;
	CString szMediaFile;
	CAtlList<CString> szaIn;
};


UINT __cdecl Thread_FindMoreFileFromOneFileAndPutIntoPlaylist( LPVOID lpParam ) 
{ 
	CFFindMoreFiles * ma =(CFFindMoreFiles*) lpParam;
	// Detect If File is already opened
	Sleep(1000);
	AfxSocketInit();
	ma->wndPlaylist->RealFindMoreFileFromOneFileAndPutIntoPlaylist( ma->szMediaFile, ma->szaIn);
	delete ma;
	return 0; 
}
void CPlayerPlaylistBar::FindMoreFileFromOneFileAndPutIntoPlaylist(CString szMediaFile , CAtlList<CString>& szaIn ){
	//check if dir has more files
	CFFindMoreFiles * mFFiles = new CFFindMoreFiles( );
	mFFiles->wndPlaylist = this;
	mFFiles->szMediaFile = szMediaFile;
	CSVPToolBox svptool;
	svptool.MergeAltList(mFFiles->szaIn , szaIn);

	AfxBeginThread( Thread_FindMoreFileFromOneFileAndPutIntoPlaylist , mFFiles, THREAD_PRIORITY_LOWEST);
	
}
void CPlayerPlaylistBar::Open(CAtlList<CString>& fns, bool fMulti, CAtlList<CString>* subs, int smartAddMorefile)
{
	BOOL bAppened = false;
	CString szFirstMedia = fns.GetHead();
	if(GetCount() > 0){

		POSITION pos = fns.GetHeadPosition();
		while(pos){
			POSITION cur =  pos;
			CString szFN = fns.GetNext(pos);
			POSITION pos2 = m_pl.GetHeadPosition();
			while(pos2){
				CPlaylistItem pli =  m_pl.GetNext(pos2);
				if ( pli.FindFile(szFN ) ){
					//AfxMessageBox(_T("Empty2"));
					fns.RemoveAt(cur);
					bAppened = true;
					break;
				}
			}
			
		}

		if(bAppened){
			if(fns.GetCount() > 0)
				Append(fns, fMulti);

			FindPosByFilename( szFirstMedia);
		}
	}
	if(!bAppened && fns.GetCount() > 0){
		//AfxMessageBox(_T("Empty"));
		Empty();
		Append(fns, fMulti, subs);
		
	}
	
	if(smartAddMorefile){

		FindMoreFileFromOneFileAndPutIntoPlaylist( szFirstMedia, fns);
	}

}

class CFFindByDir{
public:
	CPlayerPlaylistBar* wndPlaylist;
	CString szFolderPath;
	
};

UINT __cdecl Thread_FindMoreFileByDirAndPutIntoPlaylist( LPVOID lpParam ) 
{ 
	CFFindByDir * ma =(CFFindByDir*) lpParam;
	// Detect If File is already opened
	Sleep(1000);
	ma->wndPlaylist->AddFolder( ma->szFolderPath , true);
	delete ma;
	return 0; 
}


void CPlayerPlaylistBar::AddFolder(CString szFPath, BOOL bWithSubDirByDefault ){
	CAtlList<CString> szaRet;
	CAtlArray<CString> mask;
	AfxGetAppSettings().Formats.GetExtsArray(mask, 0);

	CString szMediaFile ;
	if(bWithSubDirByDefault){
		szMediaFile = GetCur();
	}
	CSVPToolBox svpTool;
	CPath szFolderPath(szFPath);
	szFolderPath.AddBackslash();
	szFolderPath.Append(_T("*"));
	CAtlList<CString> fns;
	svpTool.findMoreFileByDir( szFolderPath , fns, mask ,bWithSubDirByDefault);
	BOOL bUseThreadForSubDir = false;
	if(fns.GetCount() <= 0 && !bWithSubDirByDefault){
		svpTool.findMoreFileByDir( szFolderPath , fns, mask , true);
	}else{
		bUseThreadForSubDir = true;
	}
	{
		CAutoLock dataLock(m_csDataLock);
		Empty();
		POSITION posx = fns.GetHeadPosition();
		while(posx) ParsePlayList(fns.GetNext(posx), NULL); 

	}
	

	if(bWithSubDirByDefault  ){
		if(szMediaFile){
			POSITION pos = FindPosByFilename(szMediaFile);
			m_pl.SetPos(pos);
		}else{
			m_pl.SetPos(0);
		}
	}

	Refresh();
	SavePlaylist();
	if(bUseThreadForSubDir && !bWithSubDirByDefault ){
		CFFindByDir* ma = new CFFindByDir();
		ma->szFolderPath = szFPath;
		ma->wndPlaylist = this;
		AfxBeginThread( Thread_FindMoreFileByDirAndPutIntoPlaylist , ma, THREAD_PRIORITY_LOWEST);
	}
}

void CPlayerPlaylistBar::Append(CAtlList<CString>& fns, bool fMulti, CAtlList<CString>* subs)
{
    
	m_pl.szPlayListSub.Empty();

    
	if(fMulti)
	{
		ASSERT(subs == NULL || subs->GetCount() == 0);
		POSITION pos = fns.GetHeadPosition();
        
        while(pos){
            
            ParsePlayList(fns.GetNext(pos), NULL);
            
        }
	}
	else
	{
		ParsePlayList(fns, subs);
	}
	/*if(GetCount() < 7){
		CString szFile = fns.GetTail();
		if(szFile.Right(3).CompareNoCase(_T(".rm")) == 0 || szFile.Right(5).CompareNoCase(_T(".rmvb")) == 0){

		}else{
			CheckForPlaylistSubtitle();
		}
	}*/

	Refresh();
	SavePlaylist();
}

void CPlayerPlaylistBar::Open(CStringW vdn, CStringW adn, int vinput, int vchannel, int ainput)
{
	Empty();
	Append(vdn, adn, vinput, vchannel, ainput);
}

void CPlayerPlaylistBar::Append(CStringW vdn, CStringW adn, int vinput, int vchannel, int ainput)
{
	m_pl.szPlayListSub.Empty();

	CPlaylistItem pli;
	pli.m_type = CPlaylistItem::device;
	pli.m_fns.AddTail(CString(vdn));
	pli.m_fns.AddTail(CString(adn));
	pli.m_vinput = vinput;
	pli.m_vchannel = vchannel;
	pli.m_ainput = ainput;
	CAtlList<CStringW> sl;
	CStringW vfn = GetFriendlyName(vdn);
	CStringW afn = GetFriendlyName(adn);
	if(!vfn.IsEmpty()) sl.AddTail(vfn);
	if(!afn.IsEmpty()) sl.AddTail(afn);
	CStringW label = Implode(sl, '|');
	label.Replace(L"|", L" - ");
	pli.m_label = CString(label);
	m_pl.AddTail(pli);

	Refresh();
	SavePlaylist();
}

void CPlayerPlaylistBar::SetupList()
{
	
	m_list.DeleteAllItems();

	POSITION pos = m_pl.GetHeadPosition();
	for(int i = 0; pos; i++)
	{
		CPlaylistItem& pli = m_pl.GetAt(pos);
		m_list.SetItemData(m_list.InsertItem(i, pli.GetLabel()), (DWORD_PTR)pos);
		m_list.SetItemText(i, COL_TIME, pli.GetLabel(1));
		m_pl.GetNext(pos);
	}
}

void CPlayerPlaylistBar::UpdateList()
{
	CAutoLock dataLock(m_csDataLock);
	POSITION pos = m_pl.GetHeadPosition();
	for(int i = 0, j = m_list.GetItemCount(); pos && i < j; i++)
	{
		CPlaylistItem& pli = m_pl.GetAt(pos);
		m_list.SetItemData(i, (DWORD_PTR)pos);
		m_list.SetItemText(i, COL_NAME, pli.GetLabel(0));
		m_list.SetItemText(i, COL_TIME, pli.GetLabel(1));
		m_pl.GetNext(pos);
	}
	
}

void CPlayerPlaylistBar::EnsureVisible(POSITION pos)
{
	int i = FindItem(m_pl.GetPos());
	if(i < 0) return;
	m_list.EnsureVisible(i, TRUE);
	m_list.Invalidate();
}

int CPlayerPlaylistBar::FindItem(POSITION pos)
{
	for(int i = 0; i < m_list.GetItemCount(); i++)
		if((POSITION)m_list.GetItemData(i) == pos)
			return(i);
	return(-1);
}
POSITION CPlayerPlaylistBar::FindPosByFilename(CString fn, BOOL movePos){
	POSITION pos = m_pl.GetHeadPosition();
	while(pos){	
		POSITION ret = pos;
		CPlaylistItem pi = m_pl.GetNext(pos);
		
		POSITION pos1 = pi.m_fns.GetHeadPosition();
		while(pos1){
			CString szBuf = pi.m_fns.GetNext(pos1);
            if(szBuf.CompareNoCase( fn ) == 0){
                //SVP_LogMsg5(L"%s %s",szBuf,fn );
				if(movePos){
					m_pl.SetPos(ret);
					EnsureVisible(ret);
				}
				return ret;
			}
		}
		
	}
	return NULL;
}
POSITION CPlayerPlaylistBar::FindPos(int i)
{
	if(i < 0) return(NULL);
	return((POSITION)m_list.GetItemData(i));
}

int CPlayerPlaylistBar::GetCount()
{
	return(m_pl.GetCount()); // TODO: n - .fInvalid
}

int CPlayerPlaylistBar::GetSelIdx()
{
	return(FindItem(m_pl.GetPos()));
}

void CPlayerPlaylistBar::SetSelIdx(int i)
{
	m_pl.SetPos(FindPos(i));
}

bool CPlayerPlaylistBar::IsAtEnd()
{
	return(m_pl.GetPos() && m_pl.GetPos() == m_pl.GetTailPosition());
}
int CPlayerPlaylistBar::GetTotalTimeBeforeCur(){
	POSITION pos = m_pl.GetHeadPosition();
	int totalDelay_ms = 0;
	while(pos){
		if( pos != m_pl.GetPos()){
			CPlaylistItem& pli = m_pl.GetAt(pos);
			
			totalDelay_ms += (int)(pli.m_duration/10000);
		}else{
			break;
		}
		m_pl.GetNext(pos);
	}
	return totalDelay_ms;
}
bool CPlayerPlaylistBar::GetCur(CPlaylistItem& pli)
{
	if(!m_pl.GetPos()) return(false);
	pli = m_pl.GetAt(m_pl.GetPos());
	return(true);
}

CString CPlayerPlaylistBar::GetCur()
{
	CString fn;
	CPlaylistItem pli;
	if(GetCur(pli) && !pli.m_fns.IsEmpty()) fn = pli.m_fns.GetHead();
	return(fn);
}

void CPlayerPlaylistBar::SetRandom(){
	POSITION posCur = m_pl.GetPos();
	POSITION pos = NULL;

	for(int i = 0; i < 4; i++){
		pos = m_pl.Shuffle();
		if(pos != posCur)
			break;
	}

	if(pos){
		m_pl.SetPos(pos);
		EnsureVisible(pos);
	}
	return ;
}
void CPlayerPlaylistBar::SetNext()
{
	POSITION pos = m_pl.GetPos(), org = pos;
	if(pos){
		__try{
			while(m_pl.GetNextWrap(pos).m_fInvalid && pos != org);
		//UpdateList();
			m_pl.SetPos(pos);
			EnsureVisible(pos);
		}__except(EXCEPTION_EXECUTE_HANDLER) {  }
	}
}

void CPlayerPlaylistBar::SetPrev()
{
	POSITION pos = m_pl.GetPos(), org = pos;
	if(pos){
		__try{
			while(m_pl.GetPrevWrap(pos).m_fInvalid && pos != org);
			m_pl.SetPos(pos);
			EnsureVisible(pos);
		}__except(EXCEPTION_EXECUTE_HANDLER) {  }
	}
}

void CPlayerPlaylistBar::SetFirst()
{
	POSITION pos = m_pl.GetTailPosition(), org = pos;
	if(pos){
		__try{
			while(m_pl.GetNextWrap(pos).m_fInvalid && pos != org);
		//UpdateList();
			m_pl.SetPos(pos);
			EnsureVisible(pos);
		}__except(EXCEPTION_EXECUTE_HANDLER) {  }
	}
}

void CPlayerPlaylistBar::SetLast()
{
	POSITION pos = m_pl.GetHeadPosition(), org = pos;
	if(pos){
		__try{
			while(m_pl.GetPrevWrap(pos).m_fInvalid && pos != org);
			m_pl.SetPos(pos);
			EnsureVisible(pos);
		}__except(EXCEPTION_EXECUTE_HANDLER) {  }
	}
}

void CPlayerPlaylistBar::SetCurValid(bool fValid)
{
	if(POSITION pos = m_pl.GetPos())
	{
		if(m_pl.GetAt(pos).m_fInvalid = !fValid)
		{
			int i = FindItem(pos);
			m_list.RedrawItems(i, i);
		}
	}
}

void CPlayerPlaylistBar::SetCurTime(REFERENCE_TIME rt)
{
	if(POSITION pos = m_pl.GetPos())
	{
		CPlaylistItem& pli = m_pl.GetAt(pos);
		pli.m_duration = rt;
		m_list.SetItemText(FindItem(pos), COL_TIME, pli.GetLabel(1));
	}
}

OpenMediaData* CPlayerPlaylistBar::GetCurOMD(REFERENCE_TIME rtStart)
{
	CPlaylistItem pli;
	if(!GetCur(pli)) return NULL;

	CString fn = CString(pli.m_fns.GetHead()).MakeLower();

	if(fn.Find(_T("video_ts.ifo")) >= 0
	|| fn.Find(_T(".ratdvd")) >= 0)
	{
		if(OpenDVDData* p = new OpenDVDData())
		{
			p->path = pli.m_fns.GetHead(); 
			p->subs.AddTailList(&pli.m_subs);
			return p;
		}
	}

	if(pli.m_type == CPlaylistItem::device)
	{
		if(OpenDeviceData* p = new OpenDeviceData())
		{
			POSITION pos = pli.m_fns.GetHeadPosition();
			for(int i = 0; i < countof(p->DisplayName) && pos; i++)
				p->DisplayName[i] = pli.m_fns.GetNext(pos);
			p->vinput = pli.m_vinput;
			p->vchannel = pli.m_vchannel;
			p->ainput = pli.m_ainput;
			return p;
		}
	}
	else
	{
		if(OpenFileData* p = new OpenFileData())
		{
			p->fns.AddTailList(&pli.m_fns);
			//p->subs.AddTailList(&pli.m_subs);
			p->rtStart = rtStart;
			return p;
		}
	}

	return NULL;
}

void CPlayerPlaylistBar::LoadPlaylist()
{
	CString base;
	if(AfxGetMyApp()->GetAppDataPath(base))
	{
		CPath p;
		p.Combine(base, _T("default.mpcpl"));

		if(!AfxGetMyApp()->GetProfileInt(ResStr(IDS_R_SETTINGS), _T("RememberPlaylistItems"), TRUE))
		{
			DeleteFile(p);
		}
		else
		{
			ParseMPCPlayList(p);
			Refresh();
		}
	}
}

void CPlayerPlaylistBar::SavePlaylist(BOOL bDeletePlayList)
{
	CString base;
	if(AfxGetMyApp()->GetAppDataPath(base))
	{
		CPath p;
		p.Combine(base, _T("default.mpcpl"));

		if(!AfxGetMyApp()->GetProfileInt(ResStr(IDS_R_SETTINGS), _T("RememberPlaylistItems"), TRUE) || bDeletePlayList)
		{
			DeleteFile(p);
		}
		else
		{
			SaveMPCPlayList(p, CTextFile::UTF8, false);
		}
	}
	
}

BEGIN_MESSAGE_MAP(CPlayerPlaylistBar, CSizingControlBarG)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_PLAYLIST, OnNMDblclkList)
//	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PLAYLIST, OnCustomdrawList)
	ON_WM_DRAWITEM()
	ON_COMMAND_EX(ID_FILE_CLOSEPLAYLIST, OnFileClosePlaylist)
	ON_COMMAND_EX(ID_PLAY_PLAY, OnPlayPlay)
    ON_COMMAND_EX(ID_PLAYLIST_DELETEITEM, OnPlaylistDeleteItem)
	ON_WM_DROPFILES()
	ON_NOTIFY(LVN_BEGINDRAG, IDC_PLAYLIST, OnBeginDrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_PLAYLIST, OnLvnEndlabeleditList)
	ON_BN_CLICKED(IDC_BUTTONCLEARALL, OnButtonClearAll)
	ON_BN_CLICKED(IDC_BUTTONADDSUBFORPLAYLIST, OnButtonAddSubForPlayList)
	ON_UPDATE_COMMAND_UI(IDC_BUTTONADDSUBFORPLAYLIST, OnUpdateButtonAddSubForPlayList)
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()


void CPlayerPlaylistBar::OnSetFocus(CWnd* pOldWnd )
{
	//pOldWnd->SetFocus();
	this->m_pMaindFrame->SetFocus();
	
}
// CPlayerPlaylistBar message handlers
void CPlayerPlaylistBar::OnButtonClearAll(){
	Empty();
}

void CPlayerPlaylistBar::ResizeListColumn()
{
	if(::IsWindow(m_list.m_hWnd))
	{
		CRect r;
		GetClientRect(r);
		r.DeflateRect(2, 2);
		r.bottom = r.bottom - CBUTTONHEIGHT - 5;
    r.top += m_caption_height/2;
		m_list.SetRedraw(FALSE);
		m_list.MoveWindow(r);
		m_clearall.MoveWindow(CRect(r.right - CBUTTONWIDTH , r.bottom + 2 , r.right , (r.bottom + 2 + CBUTTONHEIGHT ) ) );
		m_addsubforplaylist.MoveWindow(CRect( r.left + CBUTTONWIDTH +4 , r.bottom + 2 , r.right - 4 , (r.bottom + 2 + CBUTTONHEIGHT ) ) );
		
		m_list.GetClientRect(r);
		m_list.SetColumnWidth(COL_NAME, r.Width()-m_nTimeColWidth); //LVSCW_AUTOSIZE_USEHEADER
		m_list.SetRedraw(TRUE);
		
	}
}

void CPlayerPlaylistBar::OnSize(UINT nType, int cx, int cy)
{
	CSizingControlBarG::OnSize(nType, cx, cy);

	ResizeListColumn();
}

void CPlayerPlaylistBar::OnNMDblclkList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

	if(lpnmlv->iItem >= 0 && lpnmlv->iSubItem >= 0)
	{
			CAutoLock dataLock(m_csDataLock);
			POSITION pt = FindPos(lpnmlv->iItem);
			POSITION pos = m_pl.GetHeadPosition();
			while (pos)
			{
				if(pos == pt){
					m_pl.SetPos(pos);
					break;
				}
				m_pl.GetNext(pos);
			}
			
			m_list.Invalidate();
			((CMainFrame*)AfxGetMainWnd())->OpenCurPlaylistItem(); //should we use recent rStart time
		
	}

	*pResult = 0;
}
/*
void CPlayerPlaylistBar::OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

	*pResult = CDRF_DODEFAULT;

	if(CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYPOSTPAINT|CDRF_NOTIFYITEMDRAW;
	}
	else if(CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		pLVCD->nmcd.uItemState &= ~CDIS_SELECTED;
		pLVCD->nmcd.uItemState &= ~CDIS_FOCUS;

		pLVCD->clrText = (pLVCD->nmcd.dwItemSpec == m_playList.m_idx) ? 0x0000ff : CLR_DEFAULT;
		pLVCD->clrTextBk = m_list.GetItemState(pLVCD->nmcd.dwItemSpec, LVIS_SELECTED) ? 0xf1dacc : CLR_DEFAULT;

		*pResult = CDRF_NOTIFYPOSTPAINT;
	}
	else if(CDDS_ITEMPOSTPAINT == pLVCD->nmcd.dwDrawStage)
	{
        int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);

		if(m_list.GetItemState(pLVCD->nmcd.dwItemSpec, LVIS_SELECTED))
		{
			CRect r, r2;
			m_list.GetItemRect(nItem, &r, LVIR_BOUNDS);
			m_list.GetItemRect(nItem, &r2, LVIR_LABEL);
			r.left = r2.left;
			FrameRect(pLVCD->nmcd.hdc, &r, CBrush(0xc56a31));
		}

		*pResult = CDRF_SKIPDEFAULT;
	}
	else if(CDDS_POSTPAINT == pLVCD->nmcd.dwDrawStage)
	{
	}
}
*/

void CPlayerPlaylistBar::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	
	if(nIDCtl != IDC_PLAYLIST) return;

	int nItem = lpDrawItemStruct->itemID;
	CRect rcItem = lpDrawItemStruct->rcItem;
	POSITION pos = FindPos(nItem);
	bool fSelected = pos == m_pl.GetPos();
	
	AppSettings& s = AfxGetAppSettings();

// 	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

  RECT& rc = lpDrawItemStruct->rcItem;
  WTL::CDCHandle dc(lpDrawItemStruct->hDC);

	if(!!m_list.GetItemState(nItem, LVIS_SELECTED))
	{
    WTL::CPen pen;
    pen.CreatePen(PS_SOLID, 1, m_basecolor4);
    HPEN old_pen = dc.SelectPen(pen);
    PlaylistView::_DrawRectNoCorner(dc, rc, 1);
    dc.SelectPen(old_pen);
    rc.left++;
    rc.top++;
    rc.bottom--;
    rc.right--;
    PlaylistView::_FillGradient(dc, rc, m_basecolor3, m_basecolor);
// 		FillRect(pDC->m_hDC, rcItem, CBrush(s.GetColorFromTheme(_T("PlayListItemSelectedBG"), 0xf1dacc)));
// 		FrameRect(pDC->m_hDC, rcItem, CBrush(s.GetColorFromTheme(_T("PlayListItemSelectedBorder"), 0xc56a31)));
	}
// 	else
// 	{
// 		FillRect(pDC->m_hDC, rcItem, CBrush(s.GetColorFromTheme(_T("PlayListItemNormalBG"), 0xdddddd)));
// 	}
	CString time = _T("Invalid");
	COLORREF textcolor = fSelected?m_textcolor_hilite:m_textcolor;
	{
		CAutoLock dataLock(m_csDataLock);
		try 
		{
			POSITION pt = m_pl.GetHeadPosition();
			bool bHasPos = false;
			while(pt){
				if(pt == pos){
					bHasPos = m_pl.GetAt(pos).m_fInvalid;
					break;
				}
				m_pl.GetNext(pt);
			}
			//CPlaylistItem& pli = ; // not sure is this a proper way to avoid crash after this
			if(bHasPos) {
				textcolor |= s.GetColorFromTheme(_T("PlayListItemInvalidTextMaskColor"),0xA0A0A0);
			}
			else time = m_list.GetItemText(nItem, COL_TIME);
			
		}
		catch(...){}
	}

	

	
  SIZE timesize = {0, 0};
	CPoint timept(rcItem.right, 0);
	if(time.GetLength() > 0)
	{
    dc.GetTextExtent(time, -1, &timesize);
/*		timesize = pDC->GetTextExtent(time);*/
		if((3+timesize.cx+3) < rcItem.Width()/2)
		{
			timept = CPoint(rcItem.right-(3+timesize.cx+3), (rcItem.top+rcItem.bottom-timesize.cy)/2);
      dc.SetTextColor(textcolor);
      dc.TextOut(timept.x, timept.y, time);

// 			pDC->SetTextColor(textcolor);
// 			pDC->TextOut(timept.x, timept.y, time);
		}
	}

	CString fmt, file;
	fmt.Format(_T("%%0%dd. %%s"), (int)log10(0.1+m_pl.GetCount())+1);
	file.Format(fmt, nItem+1, m_list.GetItemText(nItem, COL_NAME));
	SIZE filesize;
  HFONT old_font = dc.SelectFont(fSelected?m_font_bold:m_font_normal);
  dc.GetTextExtent(file, -1, &filesize);
	while(3+filesize.cx+6 > timept.x && file.GetLength() > 3)
	{
		file = file.Left(file.GetLength()-4) + _T("...");
    dc.GetTextExtent(file, -1, &filesize);
// 		filesize = pDC->GetTextExtent(file);
	}
  dc.SelectFont(old_font);

	if (file.GetLength() > 3)
	{
    dc.SetTextColor(textcolor);
    HFONT old_font = dc.SelectFont(fSelected?m_font_bold:m_font_normal);
    dc.TextOut(rcItem.left + m_entry_padding*7, (rcItem.top+rcItem.bottom-filesize.cy)/2, file);
// 		pDC->SetTextColor(textcolor);
// 		pDC->TextOut(rcItem.left+3, (rcItem.top+rcItem.bottom-filesize.cy)/2, file);
    dc.SelectFont(old_font);
	}
  if (fSelected)
  {
    HFONT old_font = dc.SelectFont(m_font_symbol);
    dc.DrawText(L"4", -1, &rcItem, DT_LEFT|DT_SINGLELINE|DT_VCENTER);
    dc.SelectFont(old_font);
  }
}

BOOL CPlayerPlaylistBar::OnFileClosePlaylist(UINT nID)
{
	//Empty();
	return FALSE;
}

BOOL CPlayerPlaylistBar::OnPlayPlay(UINT nID)
{
	m_list.Invalidate();
	return FALSE;
}

void CPlayerPlaylistBar::OnDropFiles(HDROP hDropInfo)
{
	SetActiveWindow();

	CAtlList<CString> sl;
	BOOL setCurToHead = FALSE;

	UINT nFiles = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	for(UINT iFile = 0; iFile < nFiles; iFile++)
	{
		TCHAR szFileName[_MAX_PATH];
		::DragQueryFile(hDropInfo, iFile, szFileName, _MAX_PATH);
		if(isSubtitleFile(szFileName) ){
			m_pl.szPlayListSub = szFileName;
			setCurToHead = TRUE;
		}else{
			sl.AddTail(szFileName);
		}
	}
	::DragFinish(hDropInfo);

	Append(sl, true);

	if(setCurToHead){
		m_pl.SetPos(m_pl.GetHeadPosition());
		m_list.Invalidate();
		((CMainFrame*)AfxGetMainWnd())->OpenCurPlaylistItem(); 
	}
}

void CPlayerPlaylistBar::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	ModifyStyle(WS_EX_ACCEPTFILES, 0);

	m_nDragIndex = ((LPNMLISTVIEW)pNMHDR)->iItem;

	CPoint p(0, 0);
	m_pDragImage = m_list.CreateDragImageEx(&p);

	CPoint p2 = ((LPNMLISTVIEW)pNMHDR)->ptAction;

	m_pDragImage->BeginDrag(0, p2 - p);
	m_pDragImage->DragEnter(GetDesktopWindow(), ((LPNMLISTVIEW)pNMHDR)->ptAction);

	m_bDragging = TRUE;
	m_nDropIndex = -1;

	SetCapture();
}

void CPlayerPlaylistBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_bDragging)
	{
		m_ptDropPoint = point;
		ClientToScreen(&m_ptDropPoint);
		
		m_pDragImage->DragMove(m_ptDropPoint);
		m_pDragImage->DragShowNolock(FALSE);

		WindowFromPoint(m_ptDropPoint)->ScreenToClient(&m_ptDropPoint);
		
		m_pDragImage->DragShowNolock(TRUE);

		{
			int iOverItem = m_list.HitTest(m_ptDropPoint);
			int iTopItem = m_list.GetTopIndex();
			int iBottomItem = m_list.GetBottomIndex();

			if(iOverItem == iTopItem && iTopItem != 0) // top of list
				SetTimer(1, 100, NULL); 
			else
				KillTimer(1); 

			if(iOverItem >= iBottomItem && iBottomItem != (m_list.GetItemCount() - 1)) // bottom of list
				SetTimer(2, 100, NULL); 
			else 
				KillTimer(2); 
		}
	}

	__super::OnMouseMove(nFlags, point);
}

void CPlayerPlaylistBar::OnTimer(UINT nIDEvent)
{
	int iTopItem = m_list.GetTopIndex();
	int iBottomItem = iTopItem + m_list.GetCountPerPage() - 1;

	if(m_bDragging)
	{
		m_pDragImage->DragShowNolock(FALSE);

		if(nIDEvent == 1)
		{
			m_list.EnsureVisible(iTopItem - 1, false);
			m_list.UpdateWindow();
			if(m_list.GetTopIndex() == 0) KillTimer(1); 
		}
		else if(nIDEvent == 2)
		{
			m_list.EnsureVisible(iBottomItem + 1, false);
			m_list.UpdateWindow();
			if(m_list.GetBottomIndex() == (m_list.GetItemCount() - 1)) KillTimer(2); 
		} 

		m_pDragImage->DragShowNolock(TRUE);
	}

	__super::OnTimer(nIDEvent);
}

void CPlayerPlaylistBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_bDragging)
	{
		::ReleaseCapture();

		m_bDragging = FALSE;
		m_pDragImage->DragLeave(GetDesktopWindow());
		m_pDragImage->EndDrag();

		delete m_pDragImage;
		m_pDragImage = NULL;

		KillTimer(1);
		KillTimer(2);

		CPoint pt(point);
		ClientToScreen(&pt);

		if(WindowFromPoint(pt) == &m_list)
			DropItemOnList();
	}

	ModifyStyle(0, WS_EX_ACCEPTFILES); 

	__super::OnLButtonUp(nFlags, point);
}

void CPlayerPlaylistBar::DropItemOnList()
{
	m_ptDropPoint.y += 10; //
	m_nDropIndex = m_list.HitTest(CPoint(10, m_ptDropPoint.y));

	TCHAR szLabel[MAX_PATH];
	LV_ITEM lvi;
	ZeroMemory(&lvi, sizeof(LV_ITEM));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
	lvi.stateMask = LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;
	lvi.pszText = szLabel;
	lvi.iItem = m_nDragIndex;
	lvi.cchTextMax = MAX_PATH;
	m_list.GetItem(&lvi);

	if(m_nDropIndex < 0) m_nDropIndex = m_list.GetItemCount();
	lvi.iItem = m_nDropIndex;
	m_list.InsertItem(&lvi);

	CHeaderCtrl* pHeader = (CHeaderCtrl*)m_list.GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();
	lvi.mask = LVIF_TEXT;
	lvi.iItem = m_nDropIndex;
	//INDEX OF DRAGGED ITEM WILL CHANGE IF ITEM IS DROPPED ABOVE ITSELF
	if(m_nDropIndex < m_nDragIndex) m_nDragIndex++;
	for(int col=1; col < nColumnCount; col++)
	{
		_tcscpy(lvi.pszText, (LPCTSTR)(m_list.GetItemText(m_nDragIndex, col)));
		lvi.iSubItem = col;
		m_list.SetItem(&lvi);
	}

	m_list.DeleteItem(m_nDragIndex);

	CList<CPlaylistItem> tmp;
	UINT id = -1;
	
	CAutoLock dataLock(m_csDataLock);

	for(int i = 0; i < m_list.GetItemCount(); i++)
	{
		POSITION pos = (POSITION)m_list.GetItemData(i);
		POSITION pt = m_pl.GetHeadPosition();
		while (pt)
		{
			if(pos == pt){
				CPlaylistItem& pli = m_pl.GetAt(pos);
				tmp.AddTail(pli);
				if(pos == m_pl.GetPos()) id = pli.m_id;
				break;
			}
			m_pl.GetNext(pt);
		}
		
		
		
	}
	m_pl.RemoveAll();
	POSITION pos = tmp.GetHeadPosition();
	for(int i = 0; pos; i++)
	{
		CPlaylistItem& pli = tmp.GetNext(pos);
		m_pl.AddTail(pli);
		if(pli.m_id == id) m_pl.SetPos(m_pl.GetTailPosition());
		m_list.SetItemData(i, (DWORD_PTR)m_pl.GetTailPosition());
	}

	ResizeListColumn();
}

BOOL CPlayerPlaylistBar::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

	if((pNMHDR->code == TTN_NEEDTEXTA && (HWND)pTTTA->lParam != m_list.m_hWnd)
	|| (pNMHDR->code == TTN_NEEDTEXTW && (HWND)pTTTW->lParam != m_list.m_hWnd))
		return FALSE;

	int row = ((pNMHDR->idFrom-1) >> 10) & 0x3fffff;
	int col = (pNMHDR->idFrom-1) & 0x3ff;

	if(row < 0 || row >= m_pl.GetCount())
		return FALSE;

	CAutoLock dataLock(m_csDataLock);

	POSITION pt = FindPos(row);
	POSITION pos = m_pl.GetHeadPosition();
	bool bMatch = false;
	while (pos)
	{
		if(pos == pt){
			bMatch = true;
			break;
		}
		m_pl.GetNext(pos);
	}
	if(bMatch){
		CPlaylistItem& pli = m_pl.GetAt(pos);

		CString strTipText;

		if(col == COL_NAME)
		{
			POSITION pos = pli.m_fns.GetHeadPosition();
			while(pos) strTipText += _T("\n") + pli.m_fns.GetNext(pos);
			strTipText.Trim();
			
			if(pli.m_type == CPlaylistItem::device)
			{
				CString str;
				str.Format(_T("Video Input %d"), pli.m_vinput);
				if(pli.m_vinput >= 0) strTipText += _T("\n") + str;
				str.Format(_T("Video Channel %d"), pli.m_vchannel);
				if(pli.m_vchannel >= 0) strTipText += _T("\n") + str;
				str.Format(_T("Audio Input %d"), pli.m_ainput);
				if(pli.m_ainput >= 0) strTipText += _T("\n") + str;
			}

			::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, (LPARAM)(INT)1000);
		}
		else if(col == COL_TIME)
		{
			return FALSE;
		}
	
		static CStringA m_strTipTextA;
		static CStringW m_strTipTextW;

		if(pNMHDR->code == TTN_NEEDTEXTA)
		{
			m_strTipTextA = strTipText;
			pTTTA->lpszText = (LPSTR)(LPCSTR)m_strTipTextA;
		}
		else
		{
			m_strTipTextW = strTipText;
			pTTTW->lpszText = (LPWSTR)(LPCWSTR)m_strTipTextW;
		}
	}	
	*pResult = 0;

	return TRUE;    // message was handled
}

void CPlayerPlaylistBar::OnContextMenu(CWnd* /*pWnd*/, CPoint p)
{
	LVHITTESTINFO lvhti;
	lvhti.pt = p;
	m_list.ScreenToClient(&lvhti.pt);
	m_list.SubItemHitTest(&lvhti);

  PlayerPreference* pref = PlayerPreference::GetInstance();

	POSITION pos = FindPos(lvhti.iItem);
//	bool fSelected = (pos == m_pl.GetPos());
	bool fOnItem = !!(lvhti.flags&LVHT_ONITEM);

	CMenu m;
	m.CreatePopupMenu();

	enum 
	{
		M_OPEN=1, M_ADD, M_REMOVE, M_CLIPBOARD, M_SAVEAS, 
		M_SORTBYNAME, M_SORTBYPATH, M_RANDOMIZE, M_SORTBYID,
		M_REMEMBERPLAYLIST, M_SHUFFLE , M_ADDFILE , M_CLEARALL
	};

	m.AppendMenu(MF_STRING|(!fOnItem?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_OPEN, ResStr(IDS_PLAYLIST_OPEN));
	if(((CMainFrame*)AfxGetMainWnd())->m_iPlaybackMode == PM_CAPTURE) m.AppendMenu(MF_STRING|MF_ENABLED, M_ADD, ResStr(IDS_PLAYLIST_ADD));
	m.AppendMenu(MF_STRING|(/*fSelected||*/!fOnItem?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_REMOVE, ResStr(IDS_PLAYLIST_REMOVE));
	m.AppendMenu(MF_STRING|MF_ENABLED, M_CLEARALL, ResStr(IDS_PLAYLIST_MENU_REMOVE_ALL));
	m.AppendMenu(MF_STRING|MF_ENABLED, M_ADDFILE, ResStr(IDS_CONVERT_ADDFILE));
	m.AppendMenu(MF_SEPARATOR);
	m.AppendMenu(MF_STRING|(!fOnItem?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_CLIPBOARD, ResStr(IDS_PLAYLIST_COPYTOCLIPBOARD));
	m.AppendMenu(MF_STRING|(!m_pl.GetCount()?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_SAVEAS, ResStr(IDS_PLAYLIST_SAVEAS));
	m.AppendMenu(MF_SEPARATOR);
	m.AppendMenu(MF_STRING|(!m_pl.GetCount()?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_SORTBYNAME, ResStr(IDS_PLAYLIST_SORTBYLABEL));
	m.AppendMenu(MF_STRING|(!m_pl.GetCount()?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_SORTBYPATH, ResStr(IDS_PLAYLIST_SORTBYPATH));
	m.AppendMenu(MF_STRING|(!m_pl.GetCount()?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_RANDOMIZE, ResStr(IDS_PLAYLIST_RANDOMIZE));
	m.AppendMenu(MF_STRING|(!m_pl.GetCount()?(MF_DISABLED|MF_GRAYED):MF_ENABLED), M_SORTBYID, ResStr(IDS_PLAYLIST_RESTORE));
	m.AppendMenu(MF_SEPARATOR);
	m.AppendMenu(MF_STRING|MF_ENABLED|(pref->GetIntVar(INTVAR_SHUFFLEPLAYLISTITEMS)?MF_CHECKED:0), M_SHUFFLE, ResStr(IDS_PLAYLIST_SHUFFLE));
	m.AppendMenu(MF_STRING|MF_ENABLED|(AfxGetMyApp()->GetProfileInt(ResStr(IDS_R_SETTINGS), _T("RememberPlaylistItems"), TRUE)?MF_CHECKED:0), M_REMEMBERPLAYLIST, ResStr(IDS_PLAYLIST_REMEBERITEMS));

	CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
	CAutoLock dataLock(m_csDataLock);

		
		int nID = (int)m.TrackPopupMenu(TPM_LEFTBUTTON|TPM_RETURNCMD, p.x, p.y, this);
		switch(nID)
		{
		case M_OPEN:
			m_pl.SetPos(pos);
			m_list.Invalidate();
			pMainFrm->OpenCurPlaylistItem();
			break;
		case M_ADD:
			pMainFrm->AddCurDevToPlaylist();
			m_pl.SetPos(m_pl.GetTailPosition());
			break;
		case M_REMOVE:
			if(m_pl.RemoveAt(pos)) pMainFrm->CloseMedia();
			m_list.DeleteItem(lvhti.iItem);
			SavePlaylist();
			break;
		case M_SORTBYID:
			m_pl.SortById();
			SetupList();
			SavePlaylist();
			break;
		case M_SORTBYNAME:
			m_pl.SortByName();
			SetupList();
			SavePlaylist();
			break;
		case M_SORTBYPATH:
			m_pl.SortByPath();
			SetupList();
			SavePlaylist();
			break;
		case M_RANDOMIZE:
			m_pl.Randomize();
			SetupList();
			SavePlaylist();
			break;
		case M_CLEARALL:
			Empty();
			break;
		case M_ADDFILE:
			{
				CRecentFileList& MRU = AfxGetAppSettings().MRU;
				CString szLastFile;
				MRU.ReadList();
				if(MRU.GetSize() > 0){
					szLastFile = MRU[0];
				}

				CString filter;
				CAtlArray<CString> mask;
				AfxGetAppSettings().Formats.GetFilter(filter, mask);

				COpenFileDlg fd(mask, true, NULL, szLastFile, 
					OFN_EXPLORER|OFN_ENABLESIZING|OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT|OFN_ENABLEINCLUDENOTIFY, 
					filter, this);
				if(fd.DoModal() != IDOK) break;

				CAtlList<CString> fns;

				POSITION pos = fd.GetStartPosition();
				while(pos) fns.AddTail(fd.GetNextPathName(pos));

				bool fMultipleFiles = false;

				if(fns.GetCount() > 1 
					|| fns.GetCount() == 1 
					&& (fns.GetHead()[fns.GetHead().GetLength()-1] == '\\'
					|| fns.GetHead()[fns.GetHead().GetLength()-1] == '*'))
				{
					fMultipleFiles = true;
				}

				this->Append(fns, fMultipleFiles);
			}
			break;
		case M_CLIPBOARD:
			if(OpenClipboard() && EmptyClipboard())
			{
				CString str;

				CPlaylistItem& pli = m_pl.GetAt(pos);
				POSITION pos = pli.m_fns.GetHeadPosition();
				while(pos) str += _T("\r\n") + pli.m_fns.GetNext(pos);
				str.Trim();

				if(HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, (str.GetLength()+1)*sizeof(TCHAR)))
				{
					if(TCHAR* s = (TCHAR*)GlobalLock(h))
					{
						_tcscpy(s, str);
						GlobalUnlock(h);
	#ifdef UNICODE
						SetClipboardData(CF_UNICODETEXT, h);
	#else
						SetClipboardData(CF_TEXT, h);
	#endif
					}
				}
				CloseClipboard(); 
			}
			break;
		case M_SAVEAS:
			{
				CSaveTextFileDialog fd(
					CTextFile::ASCII, NULL, NULL,
					_T("Media Player Classic playlist (*.mpcpl)|*.mpcpl|Playlist (*.pls)|*.pls|WinAmp playlist (*.m3u)|*.m3u|Windows Media Playlist (*.asx)|*.asx||"), 
					this);
		
				if(fd.DoModal() != IDOK)
					break;

				CTextFile::enc encoding = (CTextFile::enc)fd.GetEncoding();
				if(encoding == CTextFile::ASCII) encoding = CTextFile::ANSI;

				int idx = fd.m_pOFN->nFilterIndex;

				CPath path(fd.GetPathName());

				switch(idx)
				{
				case 1: path.AddExtension(_T(".mpcpl")); break;
				case 2: path.AddExtension(_T(".pls")); break;
				case 3: path.AddExtension(_T(".m3u")); break;
				case 4: path.AddExtension(_T(".asx")); break;
				default: break;
				}

				bool fRemovePath = true;

				CPath p(path);
				p.RemoveFileSpec();
				CString base = (LPCTSTR)p;

				pos = m_pl.GetHeadPosition();
				while(pos && fRemovePath)
				{
					CPlaylistItem& pli = m_pl.GetNext(pos);

					if(pli.m_type != CPlaylistItem::file) fRemovePath = false;
					else
					{
						POSITION pos;

						pos = pli.m_fns.GetHeadPosition();
						while(pos && fRemovePath)
						{
							CString fn = pli.m_fns.GetNext(pos);

							CPath p(fn);
							p.RemoveFileSpec();
							if(base != (LPCTSTR)p) fRemovePath = false;
						}

						pos = pli.m_subs.GetHeadPosition();
						while(pos && fRemovePath)
						{
							CString fn = pli.m_subs.GetNext(pos);

							CPath p(fn);
							p.RemoveFileSpec();
							if(base != (LPCTSTR)p) fRemovePath = false;
						}
					}
				}

				if(idx == 1)
				{
					SaveMPCPlayList(path, encoding, fRemovePath);
					break;
				}

				CTextFile f;
				if(!f.Save(path, encoding))
					break;

				if (idx == 2)
				{
					f.WriteString(_T("[playlist]\n"));
				}
				else if (idx == 4)
				{
					f.WriteString(_T("<ASX version = \"3.0\">\n"));
				}

				pos = m_pl.GetHeadPosition();
				CString str;
				int i;
				for(i = 0; pos; i++)
				{
					CPlaylistItem& pli = m_pl.GetNext(pos);

					if(pli.m_type != CPlaylistItem::file) 
						continue;

					CString fn = pli.m_fns.GetHead();

					/*
					if(fRemovePath)
					{
						CPath p(path);
						p.StripPath();
						fn = (LPCTSTR)p;
					}
					*/

					switch(idx)
					{
					case 2: str.Format(_T("File%d=%s\n"), i+1, fn); break;
					case 3: str.Format(_T("%s\n"), fn); break;
					case 4: str.Format(_T("<Entry><Ref href = \"%s\"/></Entry>\n"), fn); break;
					default: break;
					}
					f.WriteString(str);
				}

				if (idx == 2)
				{
					str.Format(_T("NumberOfEntries=%d\n"), i);
					f.WriteString(str);
					f.WriteString(_T("Version=2\n"));
				}
				else if (idx == 4)
				{
					f.WriteString(_T("</ASX>\n"));
				}
			}
			break;
		case M_REMEMBERPLAYLIST:
			AfxGetMyApp()->WriteProfileInt(ResStr(IDS_R_SETTINGS), _T("RememberPlaylistItems"), 
				!AfxGetMyApp()->GetProfileInt(ResStr(IDS_R_SETTINGS), _T("RememberPlaylistItems"), TRUE));
			break;
		case M_SHUFFLE:
			pref->SetIntVar(INTVAR_SHUFFLEPLAYLISTITEMS, !pref->GetIntVar(INTVAR_SHUFFLEPLAYLISTITEMS));
			break;
		default:
			break;
		}
	
}

void CPlayerPlaylistBar::OnLvnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if(pDispInfo->item.iItem >= 0 && pDispInfo->item.pszText)
	{
		CAutoLock dataLock(m_csDataLock);
		POSITION pt = (POSITION)m_list.GetItemData(pDispInfo->item.iItem);
		POSITION pos = m_pl.GetHeadPosition();
		bool bMatch = false;
		while (pos)
		{
			if(pos == pt){
				bMatch = true;
				break;
			}
			m_pl.GetNext(pos);
		}
		if(bMatch){
			CPlaylistItem& pli = m_pl.GetAt(pos);
			pli.m_label = pDispInfo->item.pszText;
			m_list.SetItemText(pDispInfo->item.iItem, 0, pDispInfo->item.pszText);
		}
		
	}

	*pResult = 0;
}

BOOL CPlayerPlaylistBar::OnPlaylistDeleteItem( UINT nID )
{
	
    if ( IsWindowVisible() && m_list.GetSelectedCount() > 0 && AfxMessageBox(ResStr(IDS_CONFIRM_AREYOUSURETODELETEPLAYLISTITEM), MB_OKCANCEL) == IDOK)
    {
        CAutoLock dataLock(m_csDataLock);

        CList<int> items;
        POSITION pos = m_list.GetFirstSelectedItemPosition();
        while(pos) items.AddHead(m_list.GetNextSelectedItem(pos));

        pos = items.GetHeadPosition();
        while(pos) 
        {
            int i = items.GetNext(pos);
            if(m_pl.RemoveAt(FindPos(i))) ((CMainFrame*)AfxGetMainWnd())->CloseMedia();
            m_list.DeleteItem(i);
        }

        m_list.SetItemState(-1, 0, LVIS_SELECTED);
        m_list.SetItemState(
            max(min(items.GetTail(), m_list.GetItemCount()-1), 0), 
            LVIS_SELECTED, LVIS_SELECTED);

        ResizeListColumn();
        SavePlaylist();
        return TRUE;
    }
    else
        return FALSE;
}

//////////////////////////////////////////////////////////////////////////

void CPlayerPlaylistBar::OnPaint()
{
  WTL::CPaintDC pdc(m_hWnd);

  RECT rc_wnd, rc_client;
  ::GetWindowRect(m_hWnd, &rc_wnd);
  ::GetClientRect(m_hWnd, &rc_client);
  ClientToScreen(&rc_client);

  pdc.SetViewportOrg(-2*(rc_client.left - rc_wnd.left), -2*(rc_client.top - rc_wnd.top));

  pdc.m_ps.rcPaint.left   += rc_client.left - rc_wnd.left;
  pdc.m_ps.rcPaint.top    += rc_client.top - rc_wnd.top;
  pdc.m_ps.rcPaint.right  += 2*(rc_client.left - rc_wnd.left);
  pdc.m_ps.rcPaint.bottom += 2*(rc_client.top - rc_wnd.top);

  WTL::CMemoryDC mdc(pdc, pdc.m_ps.rcPaint);
  _PaintWorker(mdc, pdc.m_ps.rcPaint);
}

void CPlayerPlaylistBar::OnNcPaint()
{
  WTL::CWindowDC dc(m_hWnd);

  RECT rc_paint, rc_wnd, rc_client;
  ::GetWindowRect(m_hWnd, &rc_paint);
  rc_paint.right  = rc_paint.right - rc_paint.left;
  rc_paint.bottom = rc_paint.bottom - rc_paint.top;
  rc_paint.left   = 0;
  rc_paint.top    = 0;
  ::GetWindowRect(m_hWnd, &rc_wnd);
  ::GetClientRect(m_hWnd, &rc_client);
  ClientToScreen(&rc_client);
  RECT rc_exclude = {rc_client.left - rc_wnd.left, rc_client.top - rc_wnd.top,
    rc_client.right - rc_wnd.left, rc_client.bottom - rc_wnd.top};

  dc.ExcludeClipRect(&rc_exclude);

  WTL::CMemoryDC mdc(dc, rc_paint);
  _PaintWorker(mdc, rc_paint);
}

void CPlayerPlaylistBar::_PaintWorker(HDC hdc, RECT rc)
{
  WTL::CDCHandle dc(hdc);
  WTL::CBrush bkgnd;
  bkgnd.CreateSolidBrush(m_basecolor);
  dc.FillRect(&rc, bkgnd);

  RECT rc_grad1 = {rc.left, rc.top, rc.right, rc.top + m_caption_height - m_padding/2};
  RECT rc_grad2 = {rc.left, rc.top + m_caption_height - m_padding/2, rc.right,
    rc.top + m_caption_height};
  PlaylistView::_FillGradient(dc, rc_grad1, m_basecolor3, m_basecolor);
  PlaylistView::_FillGradient(dc, rc_grad2, m_basecolor2, m_basecolor);

  HFONT old_font = dc.SelectFont(m_font_normal);
  RECT rc_text = {rc.left + m_padding, rc.top, rc.right - m_padding, rc.top + m_caption_height - m_padding/2};
  dc.SetBkMode(TRANSPARENT);
  dc.SetTextColor(m_textcolor);
  dc.DrawText(m_texts[0].c_str(), -1, &rc_text, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
  dc.SelectFont(old_font);
}
