
#pragma once

#include <list>
#include <vector>
#include "MediaCenterScrollBar.h"
#include "UILayerBlock.h"
#include "../../Model/MediaComm.h"
#include <boost/signals.hpp>
#include "../../Model/MCBlockModel.h"
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
  void ChangeLayer(std::wstring bmppath);
  RECT GetHittest();

public:
  MediaData m_data;

private:
  UILayerBlock* m_layer;
  POINT m_pt;
};


class BlockList
{
public:
  BlockList();
  ~BlockList();

  typedef boost::shared_ptr<MCBlockModel::abstract_model<BlockUnit> > ModelPtr;
  void SetModel(ModelPtr ptr);
  ModelPtr m_pModel;

  bool IsBlockExist(const MediaData &md);
  void AddBlock(BlockUnit* unit);  
  void DeleteBlock(std::list<BlockUnit*>::iterator it);
  void DeleteBlock(int i);
  BOOL AddScrollBar();
  void DoPaint(HDC hdc, RECT rcclient);
  void DoPaint(WTL::CDC& dc);
  BOOL OnScrollBarHittest(POINT pt, BOOL blbtndown, int& offsetspeed, HWND hwnd);
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
  void SetScrollBarInitializeFlag(BOOL bl);
  BOOL GetScrollBarInitializeFlag();
  
// signals
public:
  boost::signal<void (const MediaData &md)> m_sigPlayback;
  std::wstring m_tipstring;

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
  int m_maxcolumn;

  std::list<BlockUnit*>::iterator m_start;
  std::list<BlockUnit*>::iterator m_end;

  std::vector<float> m_x;
  std::vector<float> m_y;
  std::list<BlockUnit*> m_list;
  std::list<BlockUnit*> m_listHide;  // store hidden items

  MediaCenterScrollBar* m_scrollbar;

  int m_listendstate;
  int m_listbeginstate;

  BOOL m_scrollbarinitialize;

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

  void SetFrameHwnd(HWND hwnd);
  void SetScrollSpeed(int* speed);
  void SetOffsetBool(BOOL* bl);

private:
  HWND m_hwnd;
  int*  m_scrollspeed;
  BOOL* m_boffset; 
  BOOL m_lbtndown;
  RECT m_prehittest;
};