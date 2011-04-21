#include "stdafx.h"
#include "BlockList.h"

// BlockOne

#define  BEHITTEST 11
#define  BEDELETE   12
#define  BEPLAY    13

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
  m_layer->AddUILayer(L"play", new UILayer(L"\\skin\\play.png"));
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

  layer->SetDisplay(TRUE);
  def->SetDisplay(TRUE);

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

  dc.SetBkMode(TRANSPARENT);

  RECT rc;
  layer->GetTextureRect(rc);
  rc.left = pt.x;
  rc.top = pt.y+140;
  rc.bottom = rc.top+20;

  dc.DrawText(m_data.filename.c_str(), m_data.filename.size(), &rc, DT_END_ELLIPSIS|DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}

void BlockUnit::DeleteLayer()
{
  m_layer->DeleteAllLayer();
}

int BlockUnit::OnHittest(POINT pt, BOOL blbtndown)
{
  return m_layer->OnHittest(pt, blbtndown);
}


// BlockList

#define DOWNOFFSETNO 1
#define UPOFFSETNO 2
#define DOWNOFFSETSUCCESS 3
#define UPOFFSETSUCCESS 4
#define TIMER_TIPS 15

BlockList::BlockList()
{
  m_blockw = 138;
  m_blockh = 138;
  m_spacing = 10;
  m_scrollbarwidth = 20;
  m_top = 30;
  m_start = m_list.begin();
  m_end = m_list.begin();
  m_x.clear();
  m_y.clear();
  m_offsettotal = 0;
  m_scrollbar = 0;
  AddScrollBar();
}

BlockList::~BlockList()
{

}

void BlockList::DoPaint(HDC hdc, RECT rcclient)
{
  WTL::CMemoryDC dc(hdc, rcclient);
  HBRUSH hbrush = ::CreateSolidBrush(RGB(231, 231, 231));
  dc.FillRect(&rcclient, hbrush);
  DoPaint(dc);
}

void BlockList::DoPaint(WTL::CDC& dc)
{
  if (m_x.empty() || m_y.empty())
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

bool BlockList::IsBlockExist(const MediaData &md)
{
  std::list<BlockUnit*>::iterator it = m_list.begin();
  while (it != m_list.end())
  {
    if (((*it)->m_data.path == md.path) && 
        ((*it)->m_data.filename == md.filename))
      return true;

    ++it;
  }
  
  return false;
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
  while (x + m_blockw + m_spacing  + m_scrollbarwidth < m_winw)
  {
    m_x.push_back(x);
    // next block
    if (x + m_blockw + m_spacing + m_scrollbarwidth < m_winw)
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
  float y;
  if (m_y.empty())
    y = 30;
  else
    y = m_y.front();
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
  pt.x = m_winw - rc.right - 1;
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
  int height = (int)m_blockh + (int)m_top;
  int minitem;
  if ((int)m_winh % height == 0)
    minitem = (int)m_winh / height * m_maxcolumn;
  else
    minitem = ((int)m_winh / height + 1) * m_maxcolumn;
  
  while (minitem--)
  {
    ++it;
    if (it == m_list.end())
      return DOWNOFFSETNO;
  }
  return DOWNOFFSETSUCCESS;
}

int BlockList::IsListBegin(std::list<BlockUnit*>::iterator it)
{
  if (it == m_list.begin())
    return UPOFFSETNO;
  int column = m_maxcolumn;
  while (column-- && it != m_list.begin())
    --it;
  return UPOFFSETSUCCESS;
}

int BlockList::SetStartOffset(float offset)
{
  std::list<BlockUnit*>::iterator start = m_start;
  int listState = 0;
  int height = (int)m_blockh + (int)m_top;
  int column = m_maxcolumn;
  int distance;
  if (m_start == m_list.begin())
    distance = m_top;
  else
    distance = 0;
  
  int minoffset = (int)(m_winh - distance) % height == 0? 0:height - (int)(m_winh - distance) % height; 
  m_offsettotal += offset;
  
  if (offset > 0)
  {
    listState = IsListEnd(start);

    if (listState == DOWNOFFSETNO)
      m_offsettotal = min(m_offsettotal, minoffset);
    
    if (listState == DOWNOFFSETSUCCESS && m_offsettotal >= height + distance)
    { 
      while (column--)
        ++start;
      m_offsettotal -= (height + distance);
    }
  }
  if (offset < 0)
  {
    listState = IsListBegin(start);
   
    if (listState == UPOFFSETNO)
      m_offsettotal = max(m_offsettotal, 0);

    if (listState == UPOFFSETSUCCESS && m_offsettotal < 0)
    {
       while (column--)
         --start;
       m_offsettotal += height; 
       if (start == m_list.begin())
         distance = m_top;
       else
         distance = 0;
       m_offsettotal += distance;
    }
  }

  m_start = start;
  return listState;
}

void BlockList::SetYOffset(float offset, int result)
{
  float y = m_y.front();
  int height = (int)m_blockh + (int)m_top;
  int distance;
  if (m_start == m_list.begin())
    distance = m_top;
  else
    distance = 0;
  int ymin =  ((int)(m_winh - distance) % height == 0? 0 : (int)(m_winh - distance) % height - height);
  y = y - offset;

  switch (result)
  {
  case DOWNOFFSETNO:
    y = max(y, ymin);
    break;
  case UPOFFSETNO:
    y = min(y, distance);
    break;
  case DOWNOFFSETSUCCESS:
    if (y <= -height)
      y += height;
    break;
  case UPOFFSETSUCCESS:
    if (y > 0)
      y -= height;
    break;
  }

  m_y.front() = y;
}

BOOL BlockList::OnScrollBarHittest(POINT pt, BOOL blbtndown, int& offsetspeed, HWND hwnd)
{
  if (m_scrollbar->GetDisPlay())
    return m_scrollbar->OnHittest(pt, blbtndown, offsetspeed, hwnd);
  else
    return FALSE;
}

int BlockList::OnHittest(POINT pt, BOOL blbtndown)
{
  int state = 0;
  std::list<BlockUnit*>::iterator it = m_start;
  for (; it != m_end; ++it)
  {
    state = (*it)->OnHittest(pt, blbtndown);
    switch (state)
    {
    case BEDELETE:
      DeleteBlock(it);
      return state;
    case BEPLAY:
      {
        if (!m_sigPlayback.empty())
          m_sigPlayback((*it)->m_data); // emit a signal
      }
      return state;
    case BEHITTEST:
      m_tipstring = (*it)->m_data.filename;
      return state;
    }
  }
  m_tipstring = L"";
  return state;
}

RECT BlockList::GetScrollBarHittest()
{
  return m_scrollbar->GetHittest();
}

void BlockList::DeleteBlock(std::list<BlockUnit*>::iterator it)
{
  (*it)->DeleteLayer();

  if (it == m_start)
  {
    if (m_start == m_list.begin())
    {
      m_list.erase(it);
      m_start = m_list.begin();
    }
    else
    {
      --m_start;
      m_list.erase(it);
    }
  }
  else
    m_list.erase(it);
}

void BlockList::DeleteBlock(int index)
{
  std::list<BlockUnit*>::iterator it = m_start;
  while(index--)
    ++it;
  DeleteBlock(it);
}

BOOL BlockList::ContiniuPaint()
{
  int count = 0;
  std::list<BlockUnit*>::iterator it = m_start;
  for (; it != m_end; ++it)
    ++count;
  return count <= m_y.size() * m_x.size()? TRUE:FALSE;
}

//BlockListView

BlockListView::BlockListView():
m_lbtndown(FALSE)
{

}

BlockListView::~BlockListView()
{

}

void BlockListView::SetFrameHwnd(HWND hwnd)
{
  m_hwnd = hwnd;
}

void BlockListView::SetScrollSpeed(int* speed)
{
  m_scrollspeed = speed;
}

void BlockListView::HandleLButtonDown(POINT pt, RECT rcclient)
{
  BOOL bscroll = OnScrollBarHittest(pt, TRUE, *m_scrollspeed, m_hwnd);
  int layerstate = OnHittest(pt, TRUE);
  
  if (bscroll)
  {
    ::SetCapture(m_hwnd);
    m_lbtndown = TRUE;
  }
  
  if (layerstate == BEPLAY)
    SendMessage(m_hwnd, WM_LBUTTONUP, 0, 0);

  if (layerstate == BEDELETE)
  {
    Update(rcclient.right, rcclient.bottom);
    InvalidateRect(m_hwnd, &rcclient, FALSE);
  }
}

void BlockListView::HandleLButtonUp(POINT pt, RECT rcclient)
{

  BOOL bscroll = OnScrollBarHittest(pt, FALSE, *m_scrollspeed, m_hwnd);
  int blayer = OnHittest(pt, FALSE);

  if (m_lbtndown)
  {
    ::ReleaseCapture();
    m_lbtndown = FALSE;
  }
  
  if (!bscroll)
  {
    RECT rc = GetScrollBarHittest();
    rc.top = 0;
    rc.bottom = rcclient.bottom;
    InvalidateRect(m_hwnd, &rcclient, FALSE);
  }
}

void BlockListView::HandleMouseMove(POINT pt, RECT rcclient)
{
  BOOL bscroll = OnScrollBarHittest(pt, -1, *m_scrollspeed, m_hwnd);
  int blayer = OnHittest(pt, -1);
  if (bscroll)
    ::InvalidateRect(m_hwnd, &rcclient, FALSE);
  if (!bscroll && m_lbtndown)
    PostMessage(m_hwnd, WM_LBUTTONUP, 0, 0);

  if (blayer == BEHITTEST)
  {
    SetTimer(m_hwnd, TIMER_TIPS, 500, NULL);
    ::InvalidateRect(m_hwnd, &rcclient, FALSE);
  }
}