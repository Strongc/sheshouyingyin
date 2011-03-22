
#include "stdafx.h"
#include "BlockList.h"

// BlockOne

BlockUnit::BlockUnit() :
  m_layer(new UILayerBlock)
{
}

BlockUnit::~BlockUnit() {}

void BlockUnit::AddLayer(std::wstring tag, std::wstring Texture, BOOL display)
{
  m_layer->AddUILayer(tag, new UILayer(Texture));
}

void BlockUnit::DefLayer()
{
  m_layer->AddUILayer(L"mark", new UILayer(L"\\skin\\mark.png"));
  m_layer->AddUILayer(L"def", new UILayer(L"\\skin\\def.png"));
  m_layer->AddUILayer(L"play", new UILayer(L"\\skin\\play.png", TRUE));
  m_layer->AddUILayer(L"del", new UILayer(L"\\skin\\del.png", TRUE));
}

void BlockUnit::DoPaint(WTL::CDC& dc, POINT& pt)
{
  UILayer* layer = NULL;
  UILayer* def = NULL;
  UILayer* play = NULL;
  UILayer* del = NULL;

  m_layer->GetUILayer(L"mark", &layer);
  m_layer->GetUILayer(L"def", &def);
  m_layer->GetUILayer(L"play", &play);
  m_layer->GetUILayer(L"del", &del);

  POINT play_fixpt = {40, 70};
  POINT def_fixpt = {5, 5};
  POINT del_fixpt = {95, 8};

  layer->SetTexturePos(pt);

  POINT defpt = {def_fixpt.x+pt.x, def_fixpt.y+pt.y};
  def->SetTexturePos(defpt);

  POINT playpt = {play_fixpt.x+pt.x, play_fixpt.y+pt.y};
  play->SetTexturePos(playpt);

  POINT delpt = {del_fixpt.x+pt.x, del_fixpt.y+pt.y};
  del->SetTexturePos(delpt);

  m_layer->DoPaint(dc);

  wchar_t str[80];
  wsprintf(str, L"%d", m_layer);
  dc.TextOut(pt.x, pt.y+140, str);
}


// BlockList

BlockList::BlockList()
{
  m_blockw = 138;
  m_blockh = 138;
  m_spacing = 10;
  m_top = 30;
}

BlockList::~BlockList()
{

}

void BlockList::DoPaint(WTL::CDC& dc)
{
  if (m_maxcolumn == 0)
    return;

  int rows = 0;
  float y = .0f;
  std::vector<float>::iterator itx = m_x.begin();
  std::list<BlockUnit*>::iterator it = m_list.begin();

  for (; it != m_list.end(); ++it)
  {
    if (itx == m_x.end())
    {
      rows++;
      itx = m_x.begin();
    }

    y = rows*(m_blockh + m_top);

    POINT pt = {(*itx), y};
    (*it)->DoPaint(dc, pt);
    itx++;
  }
}

void BlockList::AddBlock(BlockUnit* unit)
{
  unit->DefLayer();
  m_list.push_back(unit);
}

void BlockList::BlockRanges()
{

}

void BlockList::AlignBlocks()
{
  m_x.clear();
  float x = m_spacing;
  while (x + m_blockw + m_spacing < m_winw)
  {
    m_x.push_back(x);
    // next block
    if (x + m_blockw + m_spacing < m_winw)
      x += m_blockw + m_spacing;
    else
      break;
  }

  m_maxcolumn = m_x.size();
  if (m_maxcolumn == 0)
    return;

  // fix each block is position
  float distance = m_winw - x;
  float offset = distance/2;
  std::vector<float>::iterator it = m_x.begin();
  for (; it != m_x.end(); ++it)
    *it += offset;
}

void BlockList::Update(float winw, float winh)
{
  m_winw = winw;
  m_winh = winh;

  AlignBlocks();
  BlockRanges();
}