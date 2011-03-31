
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
  m_layer->AddUILayer(L"play", new UILayer(L"\\skin\\play.png", FALSE));
  m_layer->AddUILayer(L"del", new UILayer(L"\\skin\\del.png", FALSE));
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
  dc.SetBkMode(TRANSPARENT);
  dc.TextOut(pt.x, pt.y+140, str);
}

BOOL BlockUnit::OnHittest(POINT pt, BOOL blbtndown)
{
  m_layer->OnHittest(pt, blbtndown);
  
  RECT rc;
  UILayer* mark;
  m_layer->GetUILayer(L"mark", &mark);
  mark->GetTextureRect(rc);
  
  BOOL bl = PtInRect(&rc, pt);

  return bl;
}


// BlockList

#define DOWNOFFSETNO 1
#define UPOFFSETNO 2
#define OFFSETSUCCESS 3

BlockList::BlockList()
{
  m_blockw = 138;
  m_blockh = 138;
  m_spacing = 10;
  m_top = 30;
  m_y.push_back(0.0);
  m_start = m_list.begin();
  m_offsettotal = 0;
  m_scrollbar = 0;
  AddScrollBar();
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

  float height = m_blockh + m_top;
  BOOL bScrollShow = (m_list.size() + m_maxcolumn - 1) / m_maxcolumn * height > m_winh? TRUE:FALSE; 
  m_scrollbar->SetDisPlay(bScrollShow);
  m_scrollbar->DoPaint(dc);

  std::list<BlockUnit*>::iterator it = m_start;
  std::vector<float>::iterator itx =  m_x.begin();
  std::vector<float>::iterator ity =  m_y.begin();

  for (; it != m_end; ++it)
  {
    if (itx == m_x.end())
    {
      itx = m_x.begin();
      ++ity;
    }

    POINT pt = {*itx, *ity};
    (*it)->DoPaint(dc, pt);
    ++itx;
  }
}

void BlockList::AddBlock(BlockUnit* unit)
{
  unit->DefLayer();
  m_list.push_back(unit);
}

BOOL BlockList::AddScrollBar()
{
  if (m_scrollbar)
    return FALSE;

  m_scrollbar = new MediaCenterScrollBar;
  m_scrollbar->CreatScrollBar(L"\\skin\\rol.png");
  return TRUE;
}

void BlockList::BlockRanges()
{
  int maxitems = m_maxcolumn * m_maxrow;
  int column = m_maxcolumn;
  if (m_start == m_list.end())
    m_start = m_list.begin();

  m_end = m_start;
  while(maxitems-- && (m_end != m_list.end()))
    ++m_end;
}

void BlockList::AlignColumnBlocks()
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

void BlockList::AlignRowBlocks()
{
  float y = m_y.front();
  m_y.clear();
  
  while (y + m_blockh +m_top < m_winh + m_blockh + m_top)
  {
    m_y.push_back(y);
    if (y + m_blockh + m_top < m_winh + m_blockh + m_top)
      y += m_blockh + m_top;
    else
      break;
  }

  m_maxrow = m_y.size();
}

void BlockList::AlignScrollBar()
{
  POINT pt;
  POINT ptcurrt;
  RECT rc = m_scrollbar->GetRect();
  pt.x = max(m_x.back() + m_blockw + 1, m_winw - rc.right - 1);
  pt.y = (m_winh - rc.bottom) / 2;
  ptcurrt = m_scrollbar->GetPosition();
  if (ptcurrt.y != pt.y || ptcurrt.x != pt.x)
  {
    m_scrollbar->SetPosition(pt);
    m_scrollbar->SetScrollBarRange(m_winh);
  }
}

void BlockList::Update(float winw, float winh)
{
  m_winw = winw;
  m_winh = winh;

  AlignColumnBlocks();
  AlignRowBlocks();
  AlignScrollBar();
  BlockRanges();
}

void BlockList::SetOffset(float offset)
{
  int result = SetStartOffset(offset);
  SetYOffset(offset, result);
}

int BlockList::IsListEnd(std::list<BlockUnit*>::iterator it)
{
  int height = m_blockh + m_top;
  int minitem;
  minitem = m_winh % height == 0? m_winh / height * m_maxcolumn:(m_winh / height + 1) * m_maxcolumn;
  
  while (minitem--)
  {
    ++it;
    if (it == m_list.end())
      return DOWNOFFSETNO;
  }
  return OFFSETSUCCESS;
}

int BlockList::IsListBegin(std::list<BlockUnit*>::iterator it)
{
  if (it == m_list.begin())
    return UPOFFSETNO;
  int column = m_maxcolumn;
  while (column-- && it != m_list.begin())
    --it;
  return OFFSETSUCCESS;
}

int BlockList::SetStartOffset(float offset)
{
  std::list<BlockUnit*>::iterator start = m_start;
  int listState = 0;
  int height = m_blockh + m_top;
  int column = m_maxcolumn;
  int minoffset = (int)m_winh % height == 0? 0:height - (int)m_winh % height;
  m_offsettotal += offset;
  
  if (offset > 0)
  {
    listState = IsListEnd(start);

    if (listState == DOWNOFFSETNO)
      m_offsettotal = min(m_offsettotal, minoffset);
    
    if (listState == OFFSETSUCCESS && m_offsettotal >= height)
    { 
      while (column--)
        ++start;
      m_offsettotal -= height;
    }
  }
  if (offset < 0)
  {
    listState = IsListBegin(start);
   
    if (listState == UPOFFSETNO)
      m_offsettotal = max(m_offsettotal, 0);

    if (listState == OFFSETSUCCESS && m_offsettotal <= 0)
    {
       while (column--)
         --start;
       m_offsettotal += height; 
    }
  }

  m_start = start;

  return listState;
}

void BlockList::SetYOffset(float offset, int result)
{
  float y = m_y.front();
  int height = m_blockh + m_top;
  int ymin =  ((int)m_winh % height == 0? 0 : (int)m_winh % height - height);
  y = y - offset;

  switch (result)
  {
  case DOWNOFFSETNO:
    y = max(y, ymin);
    break;
  case UPOFFSETNO:
    y = min(y, 0);
    break;
  case OFFSETSUCCESS:
    if (y > 0)
      y -= height;
    if (y <= -height)
      y += height;
    break;
  }

  m_y.front() = y;
}

BOOL BlockList::OnScrollBarHittest(POINT pt, BOOL blbtndown, int& offsetspeed, HWND hwnd)
{
  return m_scrollbar->OnHittest(pt, blbtndown, offsetspeed, hwnd);
}

BOOL BlockList::OnHittest(POINT pt, BOOL blbtndown)
{
  BOOL bl = FALSE;
  std::list<BlockUnit*>::iterator it = m_start;
  for (; it != m_end; ++it)
  {
    bl = (*it)->OnHittest(pt, blbtndown);
    if (bl)
      break;
  }
  return bl;
}

RECT BlockList::GetScrollBarHittest()
{
  return m_scrollbar->GetHittest();
}