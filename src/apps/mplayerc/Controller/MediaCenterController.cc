﻿#include "StdAfx.h"
#include "MediaCenterController.h"
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include "../MainFrm.h"
#include "ResLoader.h"
#include "../Model/MediaDB.h"

////////////////////////////////////////////////////////////////////////////////
// normal part
MediaCenterController::MediaCenterController()
: m_planestate(FALSE)
 ,m_initiablocklist(FALSE)
{
  // load default mc cover
  SetMCCover();
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

  MediaDB<>::exec(L"begin transaction");

  m_treeModel.addFolder(file, true);
  m_treeModel.save2DB();
  m_treeModel.delTree();

  MediaDB<>::exec(L"end transaction");
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
   m_spider._Stop(1000);
  
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

void MediaCenterController::DoPaint(HDC hdc, RECT rcClient)
{
  if (m_blocklist.IsEmpty())
  {
    if (!m_mccover)
      return;

    WTL::CDC dcmem;
    HBITMAP  hold;

    dcmem.CreateCompatibleDC(hdc);
    hold = dcmem.SelectBitmap(m_mccover);

    SetStretchBltMode(hdc, HALFTONE);
    SetBrushOrgEx(hdc, 0, 0, NULL);
    StretchBlt(hdc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom,
               dcmem, 0, 0, m_mccoverbm.bmWidth, m_mccoverbm.bmHeight, SRCCOPY);

    dcmem.SelectBitmap(hold);
    dcmem.DeleteDC();
  }
  else
    m_blocklist.DoPaint(hdc, rcClient);
}
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

media_tree::model& MediaCenterController::GetMediaTree()
{
  return m_treeModel;
}

HWND MediaCenterController::GetFilmNameEdit()
{
  return m_blocklist.GetFilmNameEdit();
}

void MediaCenterController::SetMCCover()
{
  ResLoader resLoader;
  HBITMAP mccover = resLoader.LoadBitmap(L"skin\\mccover.jpg");

  if (mccover)
  {
    m_mccover.Attach(mccover);

    m_mccover.GetBitmap(&m_mccoverbm);

    if(m_mccoverbm.bmBitsPixel != 32)
      return;

    for (int y=0; y<m_mccoverbm.bmHeight; y++)
    {
      BYTE * pPixel = (BYTE *) m_mccoverbm.bmBits + m_mccoverbm.bmWidth * 4 * y;
      for (int x=0; x<m_mccoverbm.bmWidth; x++)
      {
        pPixel[0] = pPixel[0] * pPixel[3] / 255; 
        pPixel[1] = pPixel[1] * pPixel[3] / 255; 
        pPixel[2] = pPixel[2] * pPixel[3] / 255; 
        pPixel += 4;
      }
    }
    
  }
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
  
  MediaDB<>::exec(L"begin transaction");

  m_treeModel.addFile(unit->m_mediadata);
  m_treeModel.save2DB();
  m_treeModel.delTree();

  MediaDB<>::exec(L"end transaction");

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