#pragma once

#include <list>
#include <vector>
#include "MediaCenterScrollBar.h"
#include "UILayerBlock.h"
#include "../../Model/MediaComm.h"
#include "../../Model/MCBlockModel.h"
#include "../../Model/MediaTreeModel.h"
#include <boost/signals.hpp>

/* 
 * Class BlockUnit implements block UI and UnitData
 */
class BlockUnit
{
public:
  typedef media_tree::model::FileIterator FileIterator;

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
  FileIterator m_itFile;  // file iterator

private:
  UILayerBlock* m_layer;
  POINT m_pt;
};


class BlockList
{
public:
  typedef boost::shared_ptr<MCBlockModel::abstract_model<BlockUnit> > ModelPtr;

public:
  BlockList();
  ~BlockList();

  void SetModel(ModelPtr ptr);  // using various model to sort the items in the list
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

  ModelPtr m_pModel;
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