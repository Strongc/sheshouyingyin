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
  // add path to media tree
  MediaPaths mps;
  MediaPaths::iterator it;
  m_model.FindAll(mps);
  for (it = mps.begin(); it != mps.end(); ++it)
  {
    m_treeModel.addFolder(it->path);
    m_treeModel.initMerit(it->path, it->merit);
  }

  // add files to media tree
  MediaDatas mds;
  MediaDatas::iterator itFile;
  m_model.FindAll(mds);
  for (itFile = mds.begin(); itFile != mds.end(); ++itFile)
  {
    m_treeModel.addFile(itFile->path, itFile->filename);
    m_treeModel.initHide(itFile->path, itFile->filename, itFile->bHide);

    // add file to media center gui
    media_tree::model::tagFileInfo fileInfo;
    fileInfo = m_treeModel.findFile(itFile->path, itFile->filename);
    AddNewFoundData(fileInfo.itFile);

    // do not notify this change to main frame window
    // because the main window is not created now
    //CMPlayerCApp *pApp = AfxGetMyApp();
    //if (pApp)
    //{
    //  CWnd *pWnd = pApp->GetMainWnd();
    //  if (pWnd)
    //    pWnd->PostMessage(WM_COMMAND, ID_SPIDER_NEWFILE_FOUND);
    //}
  }

  // post message to main frame to add new data

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

  m_treeModel.addFolder(file, true);

  std::wstring name(::PathFindFileName(file.c_str()));
  MediaFindCondition mc = {0, name};
  MediaData mdc;
  m_model.FindOne(mdc, mc);
  if (mdc.uniqueid == 0)
    m_treeModel.addFile(file, name);
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
  m_cover._Stop();
  m_treeModel.save2DB();

  m_csSpiderNewDatas.lock();
  m_vtSpiderNewDatas.clear();
  m_csSpiderNewDatas.unlock();
}

void MediaCenterController::AddNewFoundData(media_tree::model::FileIterator fileIterator)
{
  m_csSpiderNewDatas.lock();

  m_vtSpiderNewDatas.push_back(fileIterator);

  m_csSpiderNewDatas.unlock();
}

void MediaCenterController::AddBlock()
{
  m_csSpiderNewDatas.lock();
  
  // add new found data to gui and then remove them
  std::vector<media_tree::model::FileIterator>::iterator it = m_vtSpiderNewDatas.begin();
  while (it != m_vtSpiderNewDatas.end())
  {
    // add block units
    MediaData md;
    md.path = (*it)->file_data.path;
    md.filename = (*it)->file_data.filename;
    md.bHide = (*it)->file_data.bHide;
    if (!m_blocklist.IsBlockExist(md))
    {
      media_tree::model::tagFileInfo fileInfo = m_treeModel.findFile(md.path, md.filename);
      BlockUnit* one = new BlockUnit;
      one->m_itFile = fileInfo.itFile;
      m_blocklist.AddBlock(one);

      m_cover.SetBlockUnit(one);
      
      RECT rc;
      GetClientRect(m_hwnd, &rc);
      m_blocklist.Update(rc.right - rc.left, rc.bottom - rc.top);
      if (m_blocklist.ContiniuPaint())
        InvalidateRect(m_hwnd, 0, FALSE);
    }

    ++it;
  }

  if (!m_vtSpiderNewDatas.empty())
    m_cover._Start();
 
  m_vtSpiderNewDatas.clear();

  m_csSpiderNewDatas.unlock();
}

void MediaCenterController::DelNotAddedBlock()
{
  m_csSpiderNewDatas.lock();

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

void MediaCenterController::HandleDelBlock(const BlockUnit *pBlock)
{
  typedef media_tree::model::TreeIterator TreeIterator;
  TreeIterator it = m_treeModel.findFolder(pBlock->m_itFile->file_data.path);
  TreeIterator itEnd;
  if (it != itEnd)
  {
    MediaTreeFiles::iterator itFindFile = it->lsFiles.begin();
    while (itFindFile != it->lsFiles.end())
    {
      if (itFindFile->file_data.filename == pBlock->m_itFile->file_data.filename)
      {
        itFindFile->file_data.bHide = pBlock->m_itFile->file_data.bHide;
        break;
      }

      ++itFindFile;
    }
  }
}