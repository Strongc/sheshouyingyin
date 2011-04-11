#include "StdAfx.h"
#include "MediaCenterController.h"
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include "../MainFrm.h"

////////////////////////////////////////////////////////////////////////////////
// normal part
MediaCenterController::MediaCenterController()
: m_planestate(FALSE)
{
  MediaPaths mps;
  MediaPaths::iterator it;
  m_model.FindAll(mps);
  for (it = mps.begin(); it != mps.end(); ++it)
    m_treeModel.addFolder(*it);

  // connect signals and slots
  m_blocklist.m_sigPlayback.connect(boost::bind(&MediaCenterController::HandlePlayback, this, _1));
}

MediaCenterController::~MediaCenterController()
{
}

void MediaCenterController::Playback(std::wstring file)
{
  if (!m_spider.IsSupportExtension(file))
    return;

  MediaPath mp;
  mp.path = file;
  m_treeModel.addFolder(mp);
  m_treeModel.increaseMerit(mp.path);

  std::wstring name(::PathFindFileName(file.c_str()));
  MediaFindCondition mc = {0, name};
  MediaData mdc;
  m_model.FindOne(mdc, mc);
  if (mdc.uniqueid == 0)
  {
    MediaData md;
    md.path = file;
    md.filename = name;

    m_treeModel.addFile(md);
  }
}

void MediaCenterController::SetFrame(HWND hwnd)
{
  m_hwnd = hwnd;
}


////////////////////////////////////////////////////////////////////////////////
// data control
void MediaCenterController::SpiderStart()
{
  m_spider._Stop();
  m_checkDB._Stop();

  m_spider._Start();
  m_checkDB._Start();  // check the media.db, clean invalid records
}

void MediaCenterController::SpiderStop()
{
  m_spider._Stop();
  m_checkDB._Stop();
  m_treeModel.saveToDB();
}

void MediaCenterController::AddNewFoundData(const MediaData &md)
{
  m_csSpiderNewDatas.lock();

  m_vtSpiderNewDatas.push_back(md);

  m_csSpiderNewDatas.unlock();
}

void MediaCenterController::AddBlock()
{
  m_csSpiderNewDatas.lock();

  // add new found data to gui and then remove them
  std::vector<MediaData>::iterator it = m_vtSpiderNewDatas.begin();
  while (it != m_vtSpiderNewDatas.end())
  {
    // add block units
    if (!m_blocklist.IsBlockExist(*it))
    {
      BlockUnit* one = new BlockUnit;
      one->m_data = *it;
      m_blocklist.AddBlock(one);
    }

    ++it;
  }

  m_vtSpiderNewDatas.clear();

  m_csSpiderNewDatas.unlock();
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
  ::InvalidateRect(m_hwnd, 0, FALSE);
}

void MediaCenterController::DelBlock(int index)
{
  m_blocklist.DeleteBlock(index);
  ::InvalidateRect(m_hwnd, 0, FALSE);
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