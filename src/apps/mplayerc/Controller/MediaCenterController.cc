#include "StdAfx.h"
#include "MediaCenterController.h"
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include "../MainFrm.h"
#include "ResLoader.h"
#include "../Model/MediaDB.h"
#include "Strings.h"
#include "HashController.h"

////////////////////////////////////////////////////////////////////////////////
// normal part
MediaCenterController::MediaCenterController():
  m_hwnd(NULL),
  m_isupdate(FALSE),
  m_isrender(FALSE),
  m_planestate(FALSE),
  m_updatetime(0),
  m_initiablocklist(FALSE),
  m_hFilmTextFont(NULL),
  m_nstatusbarheight(20),
  m_hOldAccel(0)
{
  // create film text font
  SetFilmTextFont(12, L"宋体");
}

MediaCenterController::~MediaCenterController()
{
  if (m_hFilmTextFont)
  {
    ::DeleteObject(m_hFilmTextFont);
    m_hFilmTextFont = 0;
  }
}

void MediaCenterController::Playback(std::wstring file)
{
  if (!m_spider.IsSupportExtension(file))
    return;

  m_treeModel.addFolder(file, true);
  m_treeModel.save2DB();
  m_treeModel.delTree();
}

////////////////////////////////////////////////////////////////////////////////
// Maintenance for MC folder
bool MediaCenterController::IsMCFolderExist()
{
  using namespace boost::filesystem;

  CSVPToolBox toolbox;
  std::wstring sPath;
  toolbox.GetAppDataPath(sPath);
  return exists(sPath + L"\\mc\\cover");
}

void MediaCenterController::CreateMCFolder()
{
  using namespace boost::filesystem;

  CSVPToolBox toolbox;
  std::wstring sPath;
  toolbox.GetAppDataPath(sPath);
  create_directories(sPath + L"\\mc\\cover");
}

std::wstring MediaCenterController::GetMCFolder()
{
  // create mc folder if needed
  if (!IsMCFolderExist())
    CreateMCFolder();

  // return mc folder
  CSVPToolBox toolbox;
  std::wstring sPath;
  toolbox.GetAppDataPath(sPath);
  sPath += L"\\mc\\cover\\";

  return sPath;
}

std::wstring MediaCenterController::GetCoverPath(const std::wstring &sFilePath)
{
  std::wstring szJpgName = MediaCenterController::GetMediaHash(sFilePath);

  std::wstring sCoverPath;
  sCoverPath = GetMCFolder();
  sCoverPath += szJpgName + L".jpg";

  return sCoverPath;
}

std::wstring MediaCenterController::GetMediaHash(const std::wstring &sFilePath)
{
  std::string szFileHash = Strings::WStringToUtf8String(HashController::GetInstance()->GetSPHash(sFilePath.c_str()));
  std::wstring szJpgName = HashController::GetInstance()->GetMD5Hash(szFileHash.c_str(), szFileHash.length());

  return szJpgName;
}

////////////////////////////////////////////////////////////////////////////////
// data control
void MediaCenterController::SpiderThreadStart()
{
   m_spider._Stop();
   m_spider._Start();
}

void MediaCenterController::SpiderThreadStop()
{
   m_spider._Stop(500);
}

void MediaCenterController::CoverThreadStart()
{
   //m_cover._Stop();
   //m_cover._Start();
}

void MediaCenterController::CoverThreadStop()
{
  m_cover._Stop(500);
}

void MediaCenterController::SaveTreeDataToDB()
{
  m_treeModel.save2DB();
  m_treeModel.delTree();
}

//////////////////////////////////////////////////////////////////////////////
//  GUI control
void MediaCenterController::SetPlaneState(BOOL bl)
{
  m_planestate = bl;
}

media_tree::model& MediaCenterController::GetMediaTree()
{
  return m_treeModel;
}

CoverController& MediaCenterController::GetCoverDownload()
{
  return m_cover;
}

void MediaCenterController::UpdateBlock(RECT rc)
{
  // update the view
  //m_blocklist.Update(rc.right - rc.left, rc.bottom - rc.top);
}

void MediaCenterController::DelBlock(int index)
{
  //m_blocklist.DeleteBlock(index);
  //::InvalidateRect(m_hwnd, 0, FALSE);
}

HFONT MediaCenterController::GetFilmTextFont()
{
  return m_hFilmTextFont;
}

void MediaCenterController::SetFilmTextFont(int height, const std::wstring &family)
{
  if (m_hFilmTextFont)
  {
    ::DeleteObject(m_hFilmTextFont);
    m_hFilmTextFont = 0;
  }

  LOGFONT lf = {0};
  lf.lfHeight = height;
  ::wcscpy(lf.lfFaceName, family.c_str());

  CFont fnTemp;
  fnTemp.CreateFontIndirect(&lf);
  m_hFilmTextFont = (HFONT)fnTemp;
  fnTemp.Detach();
}

void MediaCenterController::InitTextEdit()
{
  if (!m_pFilmNameEdit)
  {
    // create new editor
    m_pFilmNameEdit.reset(new TextEdit);
    m_pFilmNameEdit->Create(WS_CHILD | ES_AUTOHSCROLL | ES_MULTILINE | ES_CENTER, CRect(0, 0, 0, 0)
      , CWnd::FromHandle(m_hwnd), 1111);

    static CFont *pFont = CFont::FromHandle(GetFilmTextFont());
    m_pFilmNameEdit->SetFont(pFont);
    m_pFilmNameEdit->SetBKColor(0xe7, 0xe7, 0xe7);
  }
}

HWND MediaCenterController::GetFilmNameEdit()
{
  if (m_pFilmNameEdit)
    return m_pFilmNameEdit->GetSafeHwnd();
  else
    return 0;
}

void MediaCenterController::OnSetFilmName()  // set filmname by the edit control
{
  if (m_pFilmNameEdit && (m_pFilmNameEdit->IsWindowVisible()))
  {
    // restore the accelerate table
    CMainFrame *pFrame = (CMainFrame *)(AfxGetMyApp()->GetMainWnd());
    pFrame->m_hAccelTable = m_hOldAccel;

    // set filmname
    CString sNewFilmName;
    m_pFilmNameEdit->GetWindowText(sNewFilmName);
    (*m_itCurEdit)->m_mediadata.filmname = sNewFilmName;

    // rename file
    std::wstring sPath = (*m_itCurEdit)->m_mediadata.path;
    std::wstring sOldFilename = (*m_itCurEdit)->m_mediadata.filename;
    std::wstring sExt;
    size_t nPos = sOldFilename.find_last_of('.');
    if (nPos != std::wstring::npos)
      sExt = sOldFilename.substr(nPos);

    boost::system::error_code err;
    boost::filesystem::rename(sPath + sOldFilename, sPath + (LPCTSTR)sNewFilmName + sExt, err);

    if (err == boost::system::errc::success)
    {
      // set new filename
      (*m_itCurEdit)->m_mediadata.filename = sNewFilmName;
    }

    // store info to database
    media_tree::model &tree_model = GetMediaTree();
    tree_model.addFile((*m_itCurEdit)->m_mediadata);
  }
}

void MediaCenterController::ShowFilmNameEdit(MCDBSource::BUPOINTER it, const CRect &rc)
{
  if (m_pFilmNameEdit)
  {
    m_pFilmNameEdit->MoveWindow(rc);
    m_pFilmNameEdit->ShowWindow(SW_SHOW);

    CMainFrame *pFrame = (CMainFrame *)(AfxGetMyApp()->GetMainWnd());
    m_hOldAccel = pFrame->m_hAccelTable;
    pFrame->m_hAccelTable = 0;        // temp destroy the accelerate table

    std::wstring sFilmName = (*it)->m_mediadata.filmname;
    if (sFilmName.empty())
      sFilmName = (*it)->m_mediadata.filename;

    int pos = sFilmName.find_last_of('.');
    if (pos != std::wstring::npos)
      sFilmName = sFilmName.substr(0, pos);

    m_pFilmNameEdit->SetTextVCenter();

    m_pFilmNameEdit->SetWindowText(sFilmName.c_str());
    m_pFilmNameEdit->SetFocus();

    m_pFilmNameEdit->SetSel(0, -1); // only focus on filename, exclude the ext

    m_itCurEdit = it;  // sp is defined in macro MCLoopList
  }
}

void MediaCenterController::HideFilmNameEdit()
{
  // modify the block's filmname
  OnSetFilmName();

  // hide the editor
  m_pFilmNameEdit->SetWindowText(L"");
  m_pFilmNameEdit->ShowWindow(SW_HIDE);
}

void MediaCenterController::SetStatusText(const std::wstring &str)
{
  m_mcstatusbar.SetText(str);
}

void MediaCenterController::SetCursor(LPWSTR flag /* = IDC_HAND */)
{
  ::SetClassLong(m_hwnd, GCL_HCURSOR, (LONG)::LoadCursor(0, flag));
}

////////////////////////////////////////////////////////////////////////////////
// slots to handle user events
void MediaCenterController::HandlePlayback(const MediaData &md)
{
  CMainFrame *pMainWnd = (CMainFrame *)(AfxGetApp()->GetMainWnd());
  if (pMainWnd)
  {
    pMainWnd->SendMessage(WM_COMMAND, ID_FILE_CLOSEMEDIA);
    pMainWnd->ShowWindow(SW_SHOW);
    pMainWnd->SetForegroundWindow();

    CAtlList<CString> fns;
    fns.AddTail((md.path + md.filename).c_str());
    pMainWnd->m_wndPlaylistBar.Open(fns, false);

    if(pMainWnd->m_wndPlaylistBar.GetCount() == 1 && 
       pMainWnd->m_wndPlaylistBar.IsWindowVisible() && 
      !pMainWnd->m_wndPlaylistBar.IsFloating())
        pMainWnd->ShowControlBar(&pMainWnd->m_wndPlaylistBar, FALSE, TRUE);

    pMainWnd->OpenCurPlaylistItem();
  }
}

// void MediaCenterController::HandleDelBlock(const BlockUnit *pBlock)
// {
//   //typedef media_tree::model::TreeIterator TreeIterator;
//   //TreeIterator it = m_treeModel.findFolder(pBlock->m_itFile->file_data.path);
//   //TreeIterator itEnd;
//   //if (it != itEnd)
//   //{
//   //  MediaTreeFiles::iterator itFindFile = it->lsFiles.begin();
//   //  while (itFindFile != it->lsFiles.end())
//   //  {
//   //    if (itFindFile->file_data.filename == pBlock->m_itFile->file_data.filename)
//   //    {
//   //      itFindFile->file_data.bHide = pBlock->m_itFile->file_data.bHide;
//   //      break;
//   //    }
// 
//   //    ++itFindFile;
//   //  }
//   //}
// }


void MediaCenterController::ShowMC()
{
  if (!m_hwnd)
    return;

  SetTimer(m_hwnd, TIMER_MC_RENDER, 33, NULL);
  SetTimer(m_hwnd, TIMER_MC_UPDATE, 13, NULL);

  SetClassLong(m_hwnd, GCL_HCURSOR, (LONG)::LoadCursor(NULL, IDC_ARROW));
  m_planestate = TRUE;

  RECT rc;
  ::GetClientRect(m_hwnd, &rc);
  m_mclist.InitMCList(rc.right-rc.left, rc.bottom-rc.top - m_nstatusbarheight);
  //m_mclist.InitMCList(rc.right-rc.left, rc.bottom-rc.top);

  m_mcstatusbar.SetRect(CRect(0, rc.bottom - m_nstatusbarheight, rc.right, rc.bottom));
  m_mcstatusbar.SetBKColor(RGB(0xb7, 0xb7, 0xb7));
  m_mcstatusbar.SetVisible(true);

  ::InvalidateRect(m_hwnd, NULL, TRUE);

//   CString log(L"");
//   MCDEBUG(log);
}

void MediaCenterController::HideMC()
{
  m_planestate = FALSE;

  KillTimer(m_hwnd, TIMER_MC_RENDER);
  KillTimer(m_hwnd, TIMER_MC_UPDATE);
  
  m_mclist.ReleaseList();

  // restore defaults cursor
  SetClassLong(m_hwnd, GCL_HCURSOR, (LONG)::LoadCursor(NULL, IDC_HAND));
  ::InvalidateRect(m_hwnd, NULL, TRUE);
}

void MediaCenterController::SetFrame(HWND hwnd)
{
  m_hwnd = hwnd;
  InitTextEdit();  // create the text edit
}

BOOL MediaCenterController::GetPlaneState()
{
  return m_planestate;
}

void MediaCenterController::Update()
{
  m_isupdate = TRUE;
  m_updatetime = timeGetTime();
}

void MediaCenterController::StopUpdate()
{
  m_isupdate = FALSE;
  m_updatetime = 0;
}

void MediaCenterController::Render()
{
  m_isrender = TRUE;
}

void MediaCenterController::DoPaint(HDC hdc, RECT rcClient)
{
  WTL::CMemoryDC dc(hdc, rcClient);
  HBRUSH hbrush = ::CreateSolidBrush(RGB(0xb7, 0xb7, 0xb7));
  dc.FillRect(&rcClient, hbrush);

  RECT rcTemp = rcClient;
  rcTemp.bottom -= m_nstatusbarheight;
  m_mclist.DoPaint(dc, rcTemp);
  m_mcstatusbar.Update(dc);
  DeleteObject(hbrush);
}

void MediaCenterController::OnTimer(UINT_PTR nIDEvent)
{
  if (m_isrender && nIDEvent == TIMER_MC_RENDER)
  {
    m_isrender = FALSE;
    ::InvalidateRect(m_hwnd, NULL, TRUE);
  }
  else if (m_isupdate && nIDEvent == TIMER_MC_UPDATE && m_updatetime)
  {
    Render();
    DWORD deltatime = timeGetTime() - m_updatetime;
    m_mclist.Update(deltatime);
    m_updatetime = timeGetTime();
    Render();
  }
}

BOOL MediaCenterController::ActMouseMove(const POINT& pt)
{
  if (!m_planestate)
    return FALSE;

  m_mclist.ActMouseMove(pt);

  return TRUE;
}

BOOL MediaCenterController::ActMouseLeave()
{
  // set status message to default
  m_mcstatusbar.SetText(L"");
  Render();

  return TRUE;
}

BOOL MediaCenterController::ActMouseLBDown(const POINT& pt)
{
  if (!m_planestate)
    return FALSE;

  m_mclist.ActMouseLBDown(pt);
  if (m_mclist.GetScrollBar()->IsDragBar())
    ::SetCapture(m_hwnd);

  // determine whether destroy the filmname editor
  CRect rcEditor;
  if (m_pFilmNameEdit)
  {
    m_pFilmNameEdit->GetClientRect(&rcEditor);
    if (!rcEditor.PtInRect(pt))
      HideFilmNameEdit();
  }

  return TRUE;
}

BOOL MediaCenterController::ActMouseLBUp(const POINT& pt)
{
  if (!m_planestate)
    return FALSE;

  if (m_mclist.GetScrollBar()->IsDragBar())
    ::ReleaseCapture();
  m_mclist.ActMouseLBUp(pt);

  return TRUE;
}

BOOL MediaCenterController::ActWindowChange(int w, int h)
{
  if (!m_planestate)
    return FALSE;

  m_mclist.ActWindowChange(w, h - m_nstatusbarheight);

  RECT rc;
  ::GetClientRect(m_hwnd, &rc);
  m_mcstatusbar.SetRect(CRect(0, rc.bottom - m_nstatusbarheight, rc.right, rc.bottom));

  // set filmname if the child view's size is changed
  // determine whether destroy the filmname editor
  if (m_pFilmNameEdit)
    HideFilmNameEdit();

  return TRUE;
}

BOOL MediaCenterController::ActLButtonDblClick(const POINT& pt)
{
  if (!m_planestate)
    return FALSE;

  m_mclist.ActLButtonDblClick(pt);

  return TRUE;
}

BOOL MediaCenterController::ActRButtonUp(const POINT &pt)
{
  if (!m_planestate)
    return FALSE;

  if (m_pFilmNameEdit)
    HideFilmNameEdit();

  return m_mclist.ActRButtonUp(pt);
}