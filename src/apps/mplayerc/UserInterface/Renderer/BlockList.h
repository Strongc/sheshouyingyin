#pragma once

#include <list>
#include <vector>
#include "MediaCenterScrollBar.h"
#include "UILayerBlock.h"
#include "../../Model/MediaComm.h"
#include "../../Model/MediaTreeModel.h"
#include <boost/signals.hpp>
#include <boost/shared_ptr.hpp>
#include "TextEdit.h"
#include "MCStatusBar.h"

/* 
 * Class BlockUnit implements block UI and UnitData
 */
class BlockUnit
{
public:
  BlockUnit();
  ~BlockUnit();

  void DefLayer();
  void AddLayer(std::wstring tag, std::wstring Texture, BOOL display = TRUE);
  void DeleteLayer();
  void DoPaint(WTL::CDC& dc, POINT& pt);
  BOOL OnHittest(POINT pt, BOOL blbtndown);
  RECT GetHittest();
  void ResetCover();

  CRect GetTextRect();
  BOOL ActMouseOver(POINT pt);
  BOOL ActMouseOut(POINT pt);
  int ActMouseDown(POINT pt);

public:
  MediaData m_mediadata;

private:
  UILayerBlock* m_layer;
  POINT m_pt;
  HBITMAP m_cove;

  CRect m_rcText;
};

class BlockList
{
public:
  BlockList();
  ~BlockList();

  void AddBlock(BlockUnit* unit);  
  void DeleteBlock(BlockUnit* unit);
  void DeleteBlock(std::list<BlockUnit*>::iterator it);
  void DeleteBlock(int i);
  BOOL AddScrollBar();
  void DoPaint(HDC hdc, RECT rcclient);
  void DoPaint(WTL::CDC& dc);
  int OnScrollBarHittest(POINT pt, BOOL blbtndown, int& offsetdirection, float& offsetspeed, HWND hwnd);
  int OnHittest(POINT pt, BOOL blbtndown, BlockUnit** unit);
  void OnLButtonDblClk(POINT pt);
  void OnSetFilmName();  // set filmname by the edit control
  void HideFilmNameEditor();

  // logic
  void SetOffset(float offset);
  BOOL SetStartOffset(float offset, int state);
  void SetYOffset(float offset, int result);
  void AlignColumnBlocks();
  void AlignRowBlocks();
  void BlockRanges();
  void Update(float winw, float winh);
  void AlignScrollBar();
  void UpdateScrollBar(POINT pt);
  RECT GetScrollBarHittest();
  void AlignStatusBar();
  void SetStatusBarTip(const std::wstring& str);
  RECT GetStatusBarHittest();
  BOOL ContinuePaint();
  void GetLastBlockPosition(RECT& rc);
  int  GetEnableShowAmount();
  void SetScrollBarInitializeFlag(BOOL bl);
  BOOL GetScrollBarInitializeFlag();
  void SetScrollBarDragDirection(int offset);
  BOOL BeDirectionChange();
  std::list<BlockUnit*>* GetEmptyList();
  std::list<BlockUnit*>* GetIdleList();
  std::vector<MediaData> GetCurrentMediaDatas();
  void SwapListBuff(std::list<BlockUnit*>::iterator& it, BOOL upordown);
  void ClearList(std::list<BlockUnit*>* list);
  void NewStartIterator();
  void CalculateViewCapacity();
  void CalculateLogicalListEnd();
  int GetViewCapacity();
  int GetListRemainItem();
  int GetListCapacity();
  void SetSizeChanged();
  BOOL IsEmpty();
  BOOL NeedRepaintScrollbar();
  void SetClearStat();
  BOOL GetClearStat();
  void SetListBuffIterator(std::list<BlockUnit*>::iterator it);
  void ResetOffsetTotal();
  BOOL BeScrollBarOffseting(); 
  void ResetListLogicalEnd(BOOL bl);

// signals
public:
  boost::signal<void (const MediaData &md)> m_sigPlayback;
  std::wstring m_tipstring;
  int m_maxviewcapacity;
  int m_minviewcapacity;

private:
  int m_topdistance;
  int m_bottomdistance;

  float m_spacing;
  float m_top;
  float m_scrollbarwidth;

  float m_blockh;
  float m_blockw;

  float m_winw;
  float m_winh;

  float m_offsettotal;

  int m_margin;

  int m_maxrow;
  int m_maxcolumnpre;
  int m_maxcolumn;

  std::list<BlockUnit*>::iterator m_start;
  std::list<BlockUnit*>::iterator m_end;
  std::list<BlockUnit*>::iterator m_paintstart;
  std::list<BlockUnit*>::iterator m_paintend;
  std::list<BlockUnit*>::iterator m_logicalbegin;
  std::list<BlockUnit*>::iterator m_logicalend;
  int m_remainitem;
  int m_itemscount;

  std::vector<float> m_x;
  std::vector<float> m_y;
  std::vector<float> m_paintx;
  std::vector<float> m_painty;
  std::list<BlockUnit*>* m_list;
  std::list<BlockUnit*> m_listHide;  // store hidden items
  // dual buff
  std::list<BlockUnit*> m_list1;
  std::list<BlockUnit*> m_list2;
  int m_listsize;
  std::list<BlockUnit*>::iterator m_buffit;

  MediaCenterScrollBar* m_scrollbar;
  int m_scrollbardirection;
  int m_scrollbardirectionpre;

  MCStatusBar m_statusbar;

  BOOL m_DBendstate;
  BOOL m_DBbeginstate;

  BOOL m_scrollbarinitialize;

  bool m_bSizeChanged;

  BOOL m_clearstat;

  float m_preoffsettotal;
  float m_waitoffset;

  int m_state;
  int m_yoffsetmin;

  BOOL m_bepainting;
  
protected:
  HWND m_hwnd;
  TextEdit *m_pFilmNameEditor;
  std::list<BlockUnit*>::iterator m_itCurEdit;
  HACCEL m_hOldAccel;
};

class BlockListView : public BlockList
{
public:
  BlockListView();
  ~BlockListView();

  void HandleLButtonDown(POINT pt, BlockUnit** unit);
  void HandleLButtonUp(POINT pt, BlockUnit** unit);
  void HandleMouseMove(POINT pt, BlockUnit** unit);
  void HandleLButtonDblClk(POINT pt);
  BOOL HandleRButtonUp(POINT pt);

  void SetFrameHwnd(HWND hwnd);
  void CreateTextEdit();
  void SetScrollDirection(int* dr);
  void SetScrollSpeed(float* sd);
  void SetOffsetBool(BOOL* bl);

  HWND GetFilmNameEdit();

  BlockUnit* GetCurrentUnit();
private:
  int*  m_scrolldirection;
  float*  m_scrollspeed;
  BOOL* m_boffset; 
  BOOL m_lbtndown;
  RECT m_prehittest;

  BlockUnit* m_curUnit;
};