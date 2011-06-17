#pragma once

#include "LazyInstance.h"
#include "../Model/MediaComm.h"
#include "../Model/MediaModel.h"
#include "../Model/MediaTreeModel.h"
#include "MediaSpiderFolderTree.h"
#include <map>
//#include "..\UserInterface\Renderer\MCStatusBar.h"
#include "CoverController.h"
#include "..\UserInterface\Renderer\MCList.h"
#include "..\UserInterface\Renderer\MCStatusBar.h"

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

   media_tree::model& GetMediaTree();
   CoverController& GetCoverDownload();
   
   HRGN CalculateUpdateRgn(WTL::CRect& rc);

   HFONT GetFilmTextFont();
   void SetFilmTextFont(int height, const std::wstring &family);
   void InitTextEdit();
   HWND GetFilmNameEdit();
   void OnSetFilmName();  // set filmname by the edit control
   void ShowFilmNameEdit(MCDBSource::BUPOINTER it, const CRect &rc);
   void HideFilmNameEdit();

   void SetStatusText(const std::wstring &str);

   void SetCursor(LPWSTR flag = IDC_HAND);

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

  // ****************************** //
public:
  void SetFrame(HWND hwnd);

  void ShowMC();
  void HideMC();

  BOOL GetPlaneState();

  void OnTimer(UINT_PTR nIDEvent);

  void DoPaint(HDC hdc, RECT rcClient);

  void Render();
  void StopUpdate();
  void Update();

  BOOL ActMouseMove(const POINT& pt);
  BOOL ActMouseLeave();
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
  MCStatusBar m_mcstatusbar;
  int m_nstatusbarheight;

private:
  // datas for film name edit
  boost::shared_ptr<TextEdit> m_pFilmNameEdit;
  HACCEL m_hOldAccel;
  MCDBSource::BUPOINTER m_itCurEdit;  // the block unit we current edit it
  
private: 
  BOOL m_isupdate;
  BOOL m_isrender;

  CoverController m_cover;
  BOOL m_initiablocklist;
  HFONT m_hFilmTextFont;
  
  // Data
  MediaModel            m_model;
  media_tree::model     m_treeModel;
  MediaSpiderFolderTree m_spider;
};