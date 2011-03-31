
#pragma once

#include <list>
#include <vector>
#include "MediaCenterScrollBar.h"

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
  void DoPaint(WTL::CDC& dc, POINT& pt);
  BOOL OnHittest(POINT pt, BOOL blbtndown);

private:
  UILayerBlock* m_layer;
  POINT m_pt;
  // UnitData is a object which element of tree-node data struct.
  // UnitData data;
};


class BlockList
{
public:
  BlockList();
  ~BlockList();

  void AddBlock(BlockUnit* unit);  
  BOOL AddScrollBar();
  void DoPaint(WTL::CDC& dc);
  BOOL OnScrollBarHittest(POINT pt, BOOL blbtndown, int& offsetspeed, HWND hwnd);
  BOOL OnHittest(POINT pt, BOOL blbtndown);
  
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

private:
  float m_spacing;
  float m_top;

  float m_blockh;
  float m_blockw;

  float m_winw;
  int m_winh;

  float m_offsettotal;

  int m_maxrow;
  int m_maxcolumn;

  std::list<BlockUnit*>::iterator m_start;
  std::list<BlockUnit*>::iterator m_end;

  std::vector<float> m_x;
  std::vector<float> m_y;
  std::list<BlockUnit*> m_list;

  MediaCenterScrollBar* m_scrollbar;

  int m_listendstate;
  int m_listbeginstate;

};