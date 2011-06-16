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
  m_nstatusbarheight(20)
{
  // create film text font
  SetFilmTextFont(12, L"宋体");

  // load default mc cover
  // SetMCCover();
  // connect signals and slots
  //m_blocklist.m_sigPlayback.connect(boost::bind(&MediaCenterController::HandlePlayback, this, _1));
}

MediaCenterController::~MediaCenterController()
{
  //m_loaddata._Stop();
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

HWND MediaCenterController::GetFilmNameEdit()
{
  return m_mclist.GetFilmNameEdit();
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

// void MediaCenterController::SetCover(BlockUnit* unit, std::wstring orgpath)
// {
//   std::vector<MediaData> vtMD = m_blocklist.GetCurrentMediaDatas();
//   std::vector<MediaData>::iterator it = vtMD.begin();
//   while (it != vtMD.end())
//   {
//     m_cover.SetBlockUnit(*it);
//     
//     ++it;
//   }
// 
//   m_cover._Start();
// }

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

void MediaCenterController::SetStatusText(const std::wstring &str)
{
  m_mcstatusbar.SetText(str);
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
  m_mclist.InitMCList(rc.right-rc.left, rc.bottom-rc.top);

  m_mcstatusbar.SetRect(CRect(0, rc.bottom - m_nstatusbarheight, rc.right, rc.bottom));
  m_mcstatusbar.SetBKColor(RGB(0xb7, 0xb7, 0xb7));
  m_mcstatusbar.SetVisible(true);

  ::InvalidateRect(m_hwnd, NULL, TRUE);

  CString log(L"");
  MCDEBUG(log);
}

void MediaCenterController::HideMC()
{
  m_planestate = FALSE;

  KillTimer(m_hwnd, TIMER_MC_RENDER);
  KillTimer(m_hwnd, TIMER_MC_UPDATE);
  // restore defaults cursor
  SetClassLong(m_hwnd, GCL_HCURSOR, (LONG)::LoadCursor(NULL, IDC_HAND));
  ::InvalidateRect(m_hwnd, NULL, TRUE);
}

void MediaCenterController::SetFrame(HWND hwnd)
{
  m_hwnd = hwnd;
  m_mclist.InitTextEdit();  // create the text edit
}

HWND MediaCenterController::GetFrame()
{
  return m_hwnd;
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

BOOL MediaCenterController::ActMouseLBDown(const POINT& pt)
{
  if (!m_planestate)
    return FALSE;

  m_mclist.ActMouseLBDown(pt);
  if (m_mclist.GetScrollBar()->IsDragBar())
    ::SetCapture(m_hwnd);

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

  return m_mclist.ActRButtonUp(pt);
}