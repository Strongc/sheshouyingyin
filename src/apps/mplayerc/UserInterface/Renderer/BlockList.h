#pragma once

#include <list>
#include <vector>
#include "MediaCenterScrollBar.h"
#include "UILayerBlock.h"
#include "../../Model/MediaComm.h"
#include "../../Model/MediaTreeModel.h"
#include <boost/signals.hpp>
#include <boost/shared_ptr.hpp>

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

  void ActMouseOver();
  void ActMouseOut();

public:
  MediaData m_mediadata;

private:
  UILayerBlock* m_layer;
  POINT m_pt;
  HBITMAP m_cove;
};

class BlockList
{
public:
  BlockList();
  ~BlockList();

  void AddBlock(BlockUnit* unit);  
  void DeleteBlock(std::list<BlockUnit*>::iterator it);
  void DeleteBlock(int i);
  BOOL AddScrollBar();
  void DoPaint(HDC hdc, RECT rcclient);
  void DoPaint(WTL::CDC& dc);
  int OnScrollBarHittest(POINT pt, BOOL blbtndown, int& offsetspeed, HWND hwnd);
  int OnHittest(POINT pt, BOOL blbtndown, BlockUnit** unit);

  // logic
  void SetOffset(float offset);
  BOOL SetStartOffset(float offset);
  void SetYOffset(float offset, BOOL result);
  void AlignColumnBlocks();
  void AlignRowBlocks();
  void BlockRanges();
  void Update(float winw, float winh);
  int IsListEnd(std::list<BlockUnit*>::iterator it);
  int IsListBegin(std::list<BlockUnit*>::iterator it);
  void AlignScrollBar();
  void UpdateScrollBar(POINT pt);
  RECT GetScrollBarHittest();
  BOOL ContiniuPaint();
  void GetLastBlockPosition(RECT& rc);
  int  GetEnableShowAmount();
  void SetScrollBarInitializeFlag(BOOL bl);
  BOOL GetScrollBarInitializeFlag();
  void SetScrollBarDragDirection(int offset);
  BOOL BeDirectionChange();
  std::list<BlockUnit*>* GetEmptyList();
  std::list<BlockUnit*>* GetIdleList();
  void SwapListBuff(std::list<BlockUnit*>::iterator& it, BOOL upordown);
  void ClearList(std::list<BlockUnit*>* list);
  void NewStartIterator();
  void CalculateViewCapacity();
  void CalculateLogicalListEnd();
  int GetViewCapacity();
  int GetListRemainItem();
  int GetListCapacity();
  void SetSizeChanged();

  // signals
public:
  boost::signal<void (const MediaData &md)> m_sigPlayback;
  std::wstring m_tipstring;
  int m_viewcapacity;
private:
  float m_spacing;
  float m_top;
  float m_scrollbarwidth;

  float m_blockh;
  float m_blockw;

  float m_winw;
  float m_winh;

  float m_offsettotal;

  int m_maxrow;
  int m_maxcolumnpre;
  int m_maxcolumn;

  std::list<BlockUnit*>::iterator m_start;
  std::list<BlockUnit*>::iterator m_end;
  std::list<BlockUnit*>::iterator m_logicalbegin;
  std::list<BlockUnit*>::iterator m_logicalend;
  int m_remainitem;

  std::vector<float> m_x;
  std::vector<float> m_y;
  std::list<BlockUnit*>* m_list;
  std::list<BlockUnit*> m_listHide;  // store hidden items
  // dual buff
  std::list<BlockUnit*> m_list1;
  std::list<BlockUnit*> m_list2;
  int m_listsize;

  MediaCenterScrollBar* m_scrollbar;
  int m_scrollbardirection;
  int m_scrollbardirectionpre;

  int m_listendstate;
  int m_listbeginstate;

  BOOL m_scrollbarinitialize;

  bool m_bSizeChanged;
};

class BlockListView : public BlockList
{
public:
  BlockListView();
  ~BlockListView();

  void HandleLButtonDown(POINT pt, BlockUnit** unit);
  void HandleLButtonUp(POINT pt, BlockUnit** unit);
  void HandleMouseMove(POINT pt, BlockUnit** unit);
  BOOL HandleRButtonUp(POINT pt, BlockUnit** unit, CMenu* menu);
  void HandleMouseLeave();

  void SetFrameHwnd(HWND hwnd);
  void SetScrollSpeed(int* speed);
  void SetOffsetBool(BOOL* bl);

private:
  HWND m_hwnd;
  int*  m_scrollspeed;
  BOOL* m_boffset; 
  BOOL m_lbtndown;
  RECT m_prehittest;

  BlockUnit* m_curUnit;
};