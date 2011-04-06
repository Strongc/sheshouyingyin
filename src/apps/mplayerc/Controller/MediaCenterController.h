#pragma once

#include "LazyInstance.h"
#include "../Model/MediaComm.h"
#include "../Model/MediaModel.h"
#include "../Model/MediaTreeModel.h"
#include "MediaCheckDB.h"
#include "MediaSpiderFolderTree.h"
#include <map>
#include "..\UserInterface\Renderer\BlockList.h"

class MediaCenterController:
  public LazyInstanceImpl<MediaCenterController>
{
public:
  MediaCenterController();
  ~MediaCenterController();

public:
  // Gui control, should not for other use

   void AddBlock(RECT rc);
   void DelBlock(int i);

   void SetFrame(HWND hwnd);

   void Playback(std::wstring file);

   BOOL GetPlaneState();

   void SetPlaneState(BOOL bl);

   BlockListView& GetBlockListView();

public:
  // Data control
  void SpiderStart();
  void SpiderStop();

private:
  // GUI
  HWND m_hwnd;
  BOOL m_planestate;
  MediaDatas m_mediadata;
  BlockListView m_blocklist;

  // Data
  MediaModel            m_model;
  MediaTreeModel        m_treeModel;
  MediaCheckDB          m_checkDB;
  MediaSpiderFolderTree m_spider;
};