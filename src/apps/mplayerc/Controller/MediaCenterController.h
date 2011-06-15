#pragma once

#include "LazyInstance.h"
#include "../Model/MediaComm.h"
#include "../Model/MediaModel.h"
#include "../Model/MediaTreeModel.h"
#include "MediaSpiderFolderTree.h"
#include <map>
//#include "..\UserInterface\Renderer\BlockList.h"
//#include "LoadMediaDataFromDB.h"
//#include "..\UserInterface\Renderer\MCStatusBar.h"
#include "CoverController.h"
#include "..\UserInterface\Renderer\MCList.h"

#define TIMER_MC_RENDER 12
#define TIMER_MC_UPDATE 13

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
  static std::wstring GetMediaHash(const std::wstring &sFilePath);

public:
  // Gui control, should not for other use

   void UpdateBlock(RECT rc);
   void DelBlock(int i);

   void Playback(std::wstring file);

   void SetPlaneState(BOOL bl);

   //BlockListView& GetBlockListView();
   media_tree::model& GetMediaTree();
   HWND GetFilmNameEdit();
   CoverController& GetCoverDownload();
   
   HRGN CalculateUpdateRgn(WTL::CRect& rc);

   //void SetCover(BlockUnit* unit, std::wstring orgpath);
   HFONT GetFilmTextFont();
   void SetFilmTextFont(int height, const std::wstring &family);

// slots
public:
  void HandlePlayback(const MediaData &md);

  //void HandleDelBlock(const BlockUnit *pBlock);

public:
  // Data control
  void SpiderThreadStart();
  void SpiderThreadStop();

  void CoverThreadStart();
  void CoverThreadStop();

  void SaveTreeDataToDB();

//   void LoadMediaData(int direction, std::list<BlockUnit*>* list, int viewcapacity, 
//                      int listcapacity, int remain, int times = 1);
  
//   HANDLE GetMediaDataThreadHandle();
// 
//   BOOL LoadMediaDataAlive();

  // ****************************** //
public:
  void SetFrame(HWND hwnd);
  HWND GetFrame();

  void ShowMC();
  void HideMC();

  BOOL GetPlaneState();

  void OnTimer(UINT_PTR nIDEvent);

  void DoPaint(HDC hdc, RECT rcClient);

  void Render();
  void StopUpdate();
  void Update();

  BOOL ActMouseMove(const POINT& pt);
  BOOL ActMouseLBDown(const POINT& pt);
  BOOL ActMouseLBUp(const POINT& pt);
  BOOL ActWindowChange(int w, int h);
  BOOL ActLButtonDblClick(const POINT& pt);
  BOOL ActRButtonUp(const POINT &pt);

private:
  HWND m_hwnd;
  BOOL m_planestate;

  DWORD m_updatetime;

  SPMCList m_mclist;

  // ****************************** //
  
private: 
  BOOL m_isupdate;
  BOOL m_isrender;
  //BlockListView m_blocklist;

  CoverController m_cover;
  BOOL m_initiablocklist;
  //LoadMediaDataFromDB m_loaddata;
  HFONT m_hFilmTextFont;
  
  // Data
  MediaModel            m_model;
  media_tree::model     m_treeModel;
  MediaSpiderFolderTree m_spider;
};