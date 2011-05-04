#pragma once

#include "LazyInstance.h"
#include "../Model/MediaComm.h"
#include "../Model/MediaModel.h"
#include "../Model/MediaTreeModel.h"
#include "MediaCheckDB.h"
#include "MediaSpiderFolderTree.h"
#include <map>
#include "..\UserInterface\Renderer\BlockList.h"
#include "CoverDownloadController.h"

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

public:
  // Gui control, should not for other use

   void UpdateBlock(RECT rc);
   void DelBlock(int i);

   void SetFrame(HWND hwnd);

   void Playback(std::wstring file);

   BOOL GetPlaneState();

   void SetPlaneState(BOOL bl);

   BlockListView& GetBlockListView();

   HRGN CalculateUpdateRgn(WTL::CRect& rc);

// slots
public:
  void HandlePlayback(const MediaData &md);

  void HandleDelBlock(const BlockUnit *pBlock);

public:
  // Data control
  void SpiderStart();
  void SpiderStop();

  void AddNewFoundData(media_tree::model::FileIterator fileIterator);
  void AddBlock();
  void DelNotAddedBlock();   // delete new datas when the app exit

private:
  // GUI
  HWND m_hwnd;
  BOOL m_planestate;
  MediaDatas m_mediadata;
  BlockListView m_blocklist;
  CoverDownloadController m_cover;
  BOOL m_initiablocklist;
  
  // Data
  MediaModel            m_model;
  media_tree::model     m_treeModel;
  MediaCheckDB          m_checkDB;
  MediaSpiderFolderTree m_spider;

  std::vector<media_tree::model::FileIterator> m_vtSpiderNewDatas;  // remove when it added
  CriticalSection        m_csSpiderNewDatas;  // protect above vector member
};