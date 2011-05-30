#pragma once

#include "LazyInstance.h"
#include "../Model/MediaComm.h"
#include "../Model/MediaModel.h"
#include "../Model/MediaTreeModel.h"
#include "MediaSpiderFolderTree.h"
#include <map>
#include "..\UserInterface\Renderer\BlockList.h"
#include "LoadMediaDataFromDB.h"
#include "..\UserInterface\Renderer\MCStatusBar.h"
#include "CoverController.h"

class MediaCenterController:
  public LazyInstanceImpl<MediaCenterController>
{
public:
  MediaCenterController();
  ~MediaCenterController();

public:
  // Maintenance for MC folder
  static bool IsMCFolderExist();
  static void CreateMCFolder();
  static std::wstring GetMCFolder();

  static std::wstring GetCoverPath(const std::wstring &sFilePath);

public:
  // Gui control, should not for other use

   void SetMCCover();

   void DoPaint(HDC hdc, RECT rcClient);

   void UpdateBlock(RECT rc);
   void DelBlock(int i);

   void SetFrame(HWND hwnd);

   void Playback(std::wstring file);

   BOOL GetPlaneState();

   void SetPlaneState(BOOL bl);

   BlockListView& GetBlockListView();
   media_tree::model& GetMediaTree();
   HWND GetFilmNameEdit();
   CoverController& GetCoverDownload();
   
   HRGN CalculateUpdateRgn(WTL::CRect& rc);

   void SetCover(BlockUnit* unit, std::wstring orgpath);

// slots
public:
  void HandlePlayback(const MediaData &md);

  void HandleDelBlock(const BlockUnit *pBlock);

public:
  // Data control
  void SpiderThreadStart();
  void SpiderThreadStop();

  void CoverThreadStart();
  void CoverThreadStop();

  void LoadMediaData(int direction, std::list<BlockUnit*>* list, int viewcapacity, 
                     int listcapacity, int remain, int times = 1);
  
  HANDLE GetMediaDataThreadHandle();

  BOOL LoadMediaDataAlive();
  
private:
  // GUI
  HWND m_hwnd;
  BOOL m_planestate;
  
  BlockListView m_blocklist;
  WTL::CBitmap m_mccover;
  BITMAP  m_mccoverbm;
  CoverController m_cover;
  BOOL m_initiablocklist;
  LoadMediaDataFromDB m_loaddata;
  
  // Data
  MediaModel            m_model;
  media_tree::model     m_treeModel;
  MediaSpiderFolderTree m_spider;
};