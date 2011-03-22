
#pragma once

#include <list>
#include <vector>

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
  void DoPaint(WTL::CDC& dc);

  // logic
  void AlignBlocks();
  void BlockRanges();
  void Update(float winw, float winh);

private:
  float m_spacing;
  float m_top;

  float m_blockh;
  float m_blockw;

  float m_winw;
  float m_winh;

  int m_maxrow;
  int m_maxcolumn;

  std::list<BlockUnit*>::iterator m_start;
  std::list<BlockUnit*>::iterator m_end;

  std::vector<float> m_x;
  std::list<BlockUnit*> m_list;
};