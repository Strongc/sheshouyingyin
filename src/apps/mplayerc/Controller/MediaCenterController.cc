#include "StdAfx.h"
#include "MediaCenterController.h"
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include "../MainFrm.h"

////////////////////////////////////////////////////////////////////////////////
// normal part
MediaCenterController::MediaCenterController()
: m_planestate(FALSE)
 ,m_initiablocklist(FALSE)
{
  // connect signals and slots
  m_blocklist.m_sigPlayback.connect(boost::bind(&MediaCenterController::HandlePlayback, this, _1));
}

MediaCenterController::~MediaCenterController()
{
  m_loaddata._Stop();
}

void MediaCenterController::Playback(std::wstring file)
{
  if (!m_spider.IsSupportExtension(file))
    return;

  m_treeModel.addFolder(file, true);
  m_treeModel.save2DB();
  m_treeModel.delTree();
}

void MediaCenterController::SetFrame(HWND hwnd)
{
  m_hwnd = hwnd;
  m_coverdown.SetFrame(hwnd);
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


////////////////////////////////////////////////////////////////////////////////
// data control
void MediaCenterController::SpiderStart()
{
  m_spider._Stop();
  
  m_spider._Start();
}

void MediaCenterController::SpiderStop()
{
   m_spider._Stop();
  
   m_coverdown._Stop();
}

void MediaCenterController::LoadMediaData(int direction, std::list<BlockUnit*>* list,
                                          int viewcapacity, int listcapacity, 
                                          int remain, int times)
{
  m_loaddata._Stop();
  m_loaddata.SetList(list);
  m_loaddata.SetDirection(direction);
  m_loaddata.SetWindownCapacity(viewcapacity);
  m_loaddata.SetAmount(listcapacity);
  m_loaddata.SetListRemainItem(remain);
  m_loaddata.SetExecuteTime(times);
  m_loaddata._Start();
}

HANDLE MediaCenterController::GetMediaDataThreadHandle()
{
  return m_loaddata._GetThreadHandle();
}

BOOL MediaCenterController::LoadMediaDataAlive()
{
  return m_loaddata._Is_alive();
}

//////////////////////////////////////////////////////////////////////////////
//  GUI control

BOOL MediaCenterController::GetPlaneState()
{
  return m_planestate;
}

void MediaCenterController::SetPlaneState(BOOL bl)
{
  m_planestate = bl;
}

BlockListView& MediaCenterController::GetBlockListView()
{
  return m_blocklist;
}

void MediaCenterController::UpdateBlock(RECT rc)
{
  // update the view
  m_blocklist.Update(rc.right - rc.left, rc.bottom - rc.top);
}

void MediaCenterController::DelBlock(int index)
{
  m_blocklist.DeleteBlock(index);
  ::InvalidateRect(m_hwnd, 0, FALSE);
}

void MediaCenterController::SetCover(BlockUnit* unit, std::wstring orgpath)
{
  m_coverup.SetCover(unit, orgpath);
  
  m_treeModel.addFile(unit->m_mediadata);
  m_treeModel.save2DB();
  m_treeModel.delTree();

  CRect rc;
  rc = unit->GetHittest();
  InvalidateRect(m_hwnd, &rc, FALSE);
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

void MediaCenterController::HandleDelBlock(const BlockUnit *pBlock)
{
  //typedef media_tree::model::TreeIterator TreeIterator;
  //TreeIterator it = m_treeModel.findFolder(pBlock->m_itFile->file_data.path);
  //TreeIterator itEnd;
  //if (it != itEnd)
  //{
  //  MediaTreeFiles::iterator itFindFile = it->lsFiles.begin();
  //  while (itFindFile != it->lsFiles.end())
  //  {
  //    if (itFindFile->file_data.filename == pBlock->m_itFile->file_data.filename)
  //    {
  //      itFindFile->file_data.bHide = pBlock->m_itFile->file_data.bHide;
  //      break;
  //    }

  //    ++itFindFile;
  //  }
  //}
}