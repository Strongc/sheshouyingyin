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
  // add path to media tree
  MediaPaths mps;
  m_model.FindAll(mps);
  MediaPaths::iterator it = mps.begin();
  while (it != mps.end())
  {
    //m_treeModel.addFolder(it->path);
    //m_treeModel.initMerit(it->path, it->merit);

    ++it;
  }

  // add files to media tree
  MediaDatas mds;
  m_model.FindAll(mds);
  MediaDatas::iterator itFile = mds.begin();
  while (itFile != mds.end())
  {
    //m_treeModel.addFile(*itFile);
    //m_treeModel.initHide(itFile->path, itFile->filename, itFile->bHide);

    // add file to media center gui
    media_tree::model::tagFileInfo fileInfo;
    //fileInfo = m_treeModel.findFile(itFile->path, itFile->filename);
    AddNewFoundData(fileInfo.itFile);

    // do not notify this change to main frame window
    // because the main window is not created now

    ++itFile;
  }

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
  {
    MediaData md;
    md.path = file;
    md.filename = name;
    //m_treeModel.addFile(md);
  }
}

void MediaCenterController::SetFrame(HWND hwnd)
{
  m_hwnd = hwnd;
  m_coverdown.SetFrame(hwnd);
}

HRGN MediaCenterController::CalculateUpdateRgn(WTL::CRect& rc)
{
  WTL::CRect clientrc;
  GetClientRect(m_hwnd, &clientrc);
  
  int left1 = min(rc.right, clientrc.Width());
  int top1 = rc.top;
  int right1 = left1 == clientrc.Width()? 0:clientrc.Width();
  int bottom1 = left1 == clientrc.Width()? 0:clientrc.Height();
  HRGN hrgn1 = CreateRectRgn(left1, top1, right1, bottom1);
  
  int left2 = 0;
  int top2 = min(rc.bottom, clientrc.Height());
  int right2 = top2 == clientrc.Height()? 0:clientrc.Width();
  int bottom2 = top2 == clientrc.Height()? 0:clientrc.Height();
  HRGN hrgn2 = CreateRectRgn(left2, top2, right2, bottom2);
  
  HRGN hrgntotal = CreateRectRgn(0, 0, 0, 0);
  CombineRgn(hrgntotal, hrgn1, hrgn2, RGN_OR);
  return hrgntotal;

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
  //m_checkDB._Stop();

  m_spider._Start();
  //m_checkDB._Start();  // check the media.db, clean invalid records

  CMPlayerCApp *pApp = AfxGetMyApp();
  if (pApp)
  {
    CWnd *pWnd = pApp->GetMainWnd();
    if (pWnd)
      pWnd->PostMessage(WM_COMMAND, ID_SPIDER_NEWFILE_FOUND);
  }
}

void MediaCenterController::SpiderStop()
{
  m_spider._Stop();
  //m_checkDB._Stop();
  m_coverdown._Stop();
  //m_treeModel.save2DB();

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
  
  //// add new found data to gui and then remove them
  //int count = 1;
  //WTL::CRect rc;
  //m_blocklist.GetLastBlockPosition(rc);
  //HRGN rgn = CalculateUpdateRgn(rc);

  //std::vector<media_tree::model::FileIterator>::iterator it = m_vtSpiderNewDatas.begin();
  //while (it != m_vtSpiderNewDatas.end())
  //{
  //  // add block units
  //  
  //  BlockUnit* one = new BlockUnit;
  //  one->m_itFile = *it;
  //  m_blocklist.AddBlock(one);

  //  m_coverdown.SetBlockUnit(one);
  //  
  //  GetClientRect(m_hwnd, &rc);
  //  m_blocklist.Update(rc.right - rc.left, rc.bottom - rc.top);

  //  if (!m_initiablocklist)
  //  {
  //    int size = m_vtSpiderNewDatas.size() > m_blocklist.GetEnableShowAmount()? \
  //      m_blocklist.GetEnableShowAmount() : m_vtSpiderNewDatas.size();
  //    
  //    if (count == size)
  //    {
  //      m_initiablocklist = TRUE;
  //      InvalidateRect(m_hwnd, 0, FALSE);
  //      UpdateWindow(m_hwnd);
  //    }

  //    ++count;
  //  }
  //  
  //  ++it;
  //}

  //if (m_initiablocklist && m_blocklist.ContiniuPaint())
  //  if (m_blocklist.GetScrollBarInitializeFlag())
  //  {
  //    InvalidateRect(m_hwnd, 0, FALSE);
  //    m_blocklist.SetScrollBarInitializeFlag(FALSE);
  //  }
  //  else
  //    InvalidateRgn(m_hwnd, rgn, FALSE);

  //if (!m_vtSpiderNewDatas.empty())
  //  m_coverdown._Start();
 
  //m_vtSpiderNewDatas.clear();

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

void MediaCenterController::SetCover(BlockUnit* unit, std::wstring orgpath)
{
  m_coverup.SetCover(unit, orgpath);

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