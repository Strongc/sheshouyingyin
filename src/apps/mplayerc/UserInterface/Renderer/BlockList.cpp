#include "stdafx.h"
#include "BlockList.h"
#include "..\..\Controller\MediaCenterController.h"
#include "ResLoader.h"

// BlockOne

#define  BENORMAL 0
#define  BEHITTEST 13
#define  BEHIDE   14
#define  BEPLAY    15

BlockUnit::BlockUnit() :
  m_layer(new UILayerBlock)
, m_cove(0)
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
  m_layer->AddUILayer(L"hide", new UILayer(L"\\skin\\hide.png"));
}

void BlockUnit::DoPaint(WTL::CDC& dc, POINT& pt)
{
  UILayer* layer = NULL;
  UILayer* def = NULL;
  UILayer* play = NULL;
  UILayer* hide = NULL;

  BOOL bmark = m_layer->GetUILayer(L"mark", &layer);
  BOOL bdef = m_layer->GetUILayer(L"def", &def);
  BOOL bplay = m_layer->GetUILayer(L"play", &play);
  BOOL bhide = m_layer->GetUILayer(L"hide", &hide);

  if (!bmark || !bdef || !bplay || !bhide)
    return;

  layer->SetDisplay(TRUE);
  def->SetDisplay(TRUE);
  
  POINT play_fixpt = {27, 57};
  POINT def_fixpt = {6, 6};
  POINT hide_fixpt = {65, 6};

  layer->SetTexturePos(pt);

  POINT defpt = {def_fixpt.x+pt.x, def_fixpt.y+pt.y};
  def->SetTexturePos(defpt, 90, 103);

  POINT playpt = {play_fixpt.x+pt.x, play_fixpt.y+pt.y};
  play->SetTexturePos(playpt);

  POINT hidept = {hide_fixpt.x+pt.x, hide_fixpt.y+pt.y};
  hide->SetTexturePos(hidept);

  if (!m_mediadata.thumbnailpath.empty() && !m_cove)
  {
    std::wstring thumbnailpath = m_mediadata.thumbnailpath;
    if (GetFileAttributes(thumbnailpath.c_str()) != INVALID_FILE_ATTRIBUTES || 
        GetLastError() != ERROR_FILE_NOT_FOUND)
    {
      ResLoader resLoad;
      m_cove = resLoad.LoadBitmapFromAppData(thumbnailpath);
      if (m_cove)
        def->SetTexture(m_cove);
    }
  }

  m_layer->DoPaint(dc);

  CRect rcc;
  layer->GetTextureRect(rcc);
  
  dc.SetBkMode(TRANSPARENT);

  RECT rc;
  layer->GetTextureRect(rc);
  rc.left = pt.x;
  rc.top = pt.y+117;
  rc.bottom = rc.top+20;

  std::wstring fmnm;
  if (m_mediadata.filmname.empty())
    fmnm = m_mediadata.filename;
  else
    fmnm = m_mediadata.filmname;

  if (m_mediadata.filmname.empty())
    fmnm = m_mediadata.filename;
  else
    fmnm = m_mediadata.filmname;
  //dc.DrawText(m_itFile.filename.c_str(), m_itFile.filename.size(), &rc, DT_END_ELLIPSIS|DT_CENTER|DT_VCENTER|DT_SINGLELINE);
  CPen pen;
  dc.DrawText(fmnm.c_str(), fmnm.size(), &rc, DT_END_ELLIPSIS|DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}

void BlockUnit::DeleteLayer()
{
  m_layer->DeleteAllLayer();
  delete m_layer;
}

int BlockUnit::OnHittest(POINT pt, BOOL blbtndown)
{
  UILayer* layer = NULL;
  UILayer* def = NULL;
  UILayer* play = NULL;
  UILayer* hide = NULL;

  m_layer->GetUILayer(L"mark", &layer);
  m_layer->GetUILayer(L"def", &def);
  m_layer->GetUILayer(L"play", &play);
  m_layer->GetUILayer(L"hide", &hide);

  RECT rc;
  if (blbtndown)
  {
    play->GetTextureRect(rc);
    if (PtInRect(&rc, pt))
      return BEPLAY;
    
    hide->GetTextureRect(rc);
    if (PtInRect(&rc, pt))
      return BEHIDE;
  }
  else
  {
    layer->GetTextureRect(rc);
    if (PtInRect(&rc, pt))
    {
      play->SetDisplay(TRUE);
      hide->SetDisplay(TRUE);
      return BEHITTEST;
    }
  }
 
  play->SetDisplay(FALSE);
  hide->SetDisplay(FALSE);
  return BENORMAL;
}

RECT BlockUnit::GetHittest()
{
  RECT rc;
  UILayer* mark = NULL;
  m_layer->GetUILayer(L"mark", &mark);
  mark->GetTextureRect(rc);
  return rc;
}

void BlockUnit::ResetCover()
{
  m_cove = NULL;
}

// BlockList

#define DOWNOFFSETONE 1
#define UPOFFSETONE 2
#define DOWNOFFSETSUCCESS 3
#define UPOFFSETSUCCESS 4
#define OFFSETNO 5
#define TIMER_TIPS 15

#define  ScrollBarClick 21
#define  ScrollBarHit   22
#define  NoScrollBarHit 23

BlockList::BlockList()
: m_bSizeChanged(false)
, m_scrollbarinitialize(FALSE)
{
  m_blockw = 102;
  m_blockh = 115;
  m_spacing = 10;
  m_scrollbarwidth = 20;
  m_top = 25;
  m_list = &m_list1;
  m_start = m_list->begin();
  m_end = m_list->begin();
  m_logicalbegin = m_list->begin();
  m_logicalend = m_list->end();
  m_remainitem = 0;
  m_x.clear();
  m_y.clear();
  m_offsettotal = 0;
  m_scrollbar = 0;
  m_scrollbardirection = 0;
  m_scrollbardirectionpre = 0;
  m_listsize = 50;
  m_maxcolumn = 0;
  m_maxcolumnpre = 0;
  m_maxrow = 0;
  AddScrollBar();

  //ModelPtr ptr(new MCBlockModel::added_time_sort_model);
  //ModelPtr ptr(new MCBlockModel::filename_sort_model);
  //SetModel(ptr); // using added time model to sort the files for default
}

BlockList::~BlockList()
{
  delete m_scrollbar;
  ClearList(&m_list1);
  ClearList(&m_list2);
}

void BlockList::SetModel(ModelPtr ptr)
{
  //m_pModel = ptr;
  //m_pModel->set_data(m_list, &m_start);
}

void BlockList::DoPaint(HDC hdc, RECT rcclient)
{
  WTL::CMemoryDC dc(hdc, rcclient);
  HBRUSH hbrush = ::CreateSolidBrush(COLORREF(0x313131));
  dc.FillRect(&rcclient, hbrush);
  DoPaint(dc);
  DeleteObject(hbrush);
}

void BlockList::DoPaint(WTL::CDC& dc)
{
  if (m_x.empty() || m_y.empty())
    return;
  
  int rows = 0;
  float y = .0f;

//   if (m_scrollbar->GetDisPlay() && !m_scrollbar->GetInitializeFlag())
//   {
//     SetScrollBarInitializeFlag(TRUE);
//     m_scrollbar->SetInitializeFlag(TRUE);
//   }
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

  // using model to insert data, because the model will insert data by an order
  // the model is like a adapter
//   if (unit->m_itFile->file_data.bHide)
//     m_listHide.push_back(unit);
//   else
//     m_pModel->insert(unit);

  std::list<BlockUnit*>* list = GetEmptyList();
  if (list)
    list->push_back(unit);

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
  if (m_start == m_list->end())
    m_start = m_list->begin();

  // change start if the window size had changed
  if (m_maxcolumnpre != 0 && m_maxcolumnpre != m_maxcolumn)
  {
    NewStartIterator();
    m_maxcolumnpre = m_maxcolumn;
  }

  m_end = m_start;
  
  while(maxitems-- && (m_end != m_list->end()))
  {
    ++m_end;
  }
}

void BlockList::NewStartIterator()
{
  int maxitems = m_maxcolumn * m_maxrow;
  int count1 = 1;
  int count2 = 0;
  int row = 0;
  int items = 0;
  BOOL bl = FALSE;
  
  std::list<BlockUnit*>::iterator it = m_start;
  while (it != m_list->begin())
  {
    --it;
    ++count1;
  }

  int column = m_list->size() % m_maxcolumn;
  if (column == 0) 
    column = m_maxcolumn;

  count2 = m_list->size() - column - count1 + 1;
  bl = count1 > count2 ? FALSE : TRUE;
  int count = count1 > count2 ? count2 : count1;
  row = count / m_maxcolumnpre;
  items = row * m_maxcolumn;
  int remain = m_list->size() % m_maxcolumn;
  
  if (bl)
  {
    m_start = m_list->begin();
    while (items--)
    {
      ++m_start;
    }
  }
  else
  {
    m_start = m_list->end();

    while (remain--)
    {
      --m_start;
    }
   
    while (items--)
    {
      --m_start;
    }
  }

  m_remainitem = m_listsize % m_maxcolumn;
}

void BlockList::AlignColumnBlocks()
{
  m_maxcolumnpre = m_maxcolumn;
  m_x.clear();
  float x = m_spacing;
  
  int scrollbarwidth = 0;
  if (m_scrollbar->GetDisPlay())
    scrollbarwidth = m_scrollbarwidth;
  int count = (m_winw - 2 * m_spacing - scrollbarwidth) / m_blockw;
  int totalspacing = (int)(m_winw - 2 * m_spacing - scrollbarwidth) % (int)m_blockw;
  int spacing;
  if (totalspacing / (count - 1) >= m_spacing)
    spacing = totalspacing / (count - 1);
  else
    spacing = totalspacing / (count - 2);

  while (count--)
  {
    m_x.push_back(x);
    x += m_blockw + spacing;
  }
  
  m_maxcolumn = m_x.size();
  
}

void BlockList::AlignRowBlocks()
{
  float y;
  if (m_y.empty())
    y = 0;
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

  float height = m_blockh + m_top;
  
  BOOL bScrollShow = (m_list->size() + m_maxcolumn - 1) / m_maxcolumn * height > m_winh? TRUE:FALSE; 
  m_scrollbar->SetDisPlay(bScrollShow);
  if (bScrollShow && !m_scrollbarinitialize)
  {
    AlignColumnBlocks();
    m_scrollbarinitialize = TRUE;
  }
}

void BlockList::Update(float winw, float winh)
{
  m_winw = winw;
  m_winh = winh;

  AlignColumnBlocks();
  AlignRowBlocks();
  AlignScrollBar();
  CalculateViewCapacity();
  CalculateLogicalListEnd();
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
  
  int count = 1;
  while (minitem--)
  {
    ++it;
    ++count;
    if (it == m_logicalend)//m_list->end())
    {
      if (!MediaCenterController::GetInstance()->LoadMediaDataAlive())
        if (!GetIdleList()->empty())
          break;

      int row = (count  + m_x.size() - 1)/ m_x.size();
      float y = m_y.front(); 
      if (y + row * height < m_winh)
        return OFFSETNO;
      else
        return DOWNOFFSETONE;
    }
  }
  return DOWNOFFSETSUCCESS;
}

int BlockList::IsListBegin(std::list<BlockUnit*>::iterator it)
{
  if (it == m_list->begin() && GetIdleList()->empty())
    return UPOFFSETONE;

  return UPOFFSETSUCCESS;
}

int BlockList::SetStartOffset(float offset)
{
  std::list<BlockUnit*>::iterator start = m_start;
  int listState = 0;
  int height = (int)m_blockh + (int)m_top;
  int column = m_maxcolumn;
  int distance;
//   if (m_start == m_list->begin())
//     distance = m_top;
//   else
    distance = 0;
  
  int minoffset = (int)(m_winh - distance) % height == 0? 0:height - (int)(m_winh - distance) % height; 
  m_offsettotal += offset;
  
  if (offset > 0)
  {
    listState = IsListEnd(start);

    if (listState == DOWNOFFSETONE)
      m_offsettotal = min(m_offsettotal, minoffset);
    
    if (listState == DOWNOFFSETSUCCESS && m_offsettotal >= height + distance)
    { 
      while (column--)
        ++start;
      SwapListBuff(start, TRUE);
      m_offsettotal -= (height + distance);
    }

    if (listState == OFFSETNO)
      m_offsettotal -= offset;
  }
  if (offset < 0)
  {
    listState = IsListBegin(start);
   
    if (listState == UPOFFSETONE)
      m_offsettotal = max(m_offsettotal, 0);
    
    if (listState == UPOFFSETSUCCESS && m_offsettotal < 0)
    {
       SwapListBuff(start, FALSE);
       while (column--)
       {
         --start;
         if (start == m_list->begin())
           break;
       }

       m_offsettotal += height; 
//        if (start == m_list->begin())
//          distance = m_top;
//        else
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
//   if (m_start == m_list->begin())
//     distance = m_top;
//   else
    distance = 0;
  int ymin =  ((int)(m_winh - distance) % height == 0? 0 : (int)(m_winh - distance) % height - height);
  y = y - offset;

  switch (result)
  {
  case DOWNOFFSETONE:
    y = max(y, ymin);
    break;
  case UPOFFSETONE:
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
  case OFFSETNO:
    y = m_y.front();
    break;
  }

  m_y.front() = y;
}

int BlockList::OnScrollBarHittest(POINT pt, BOOL blbtndown, int& offsetspeed, HWND hwnd)
{
  if (m_scrollbar->GetDisPlay())
    return m_scrollbar->OnHittest(pt, blbtndown, offsetspeed, hwnd);
  else
    return 0;
}

int BlockList::OnHittest(POINT pt, BOOL blbtndown, BlockUnit** unit)
{
  int stRet = BENORMAL;  // state will return
  
  std::list<BlockUnit*>::iterator it = m_start;
  for (; it != m_end; ++it)
  {
    int state = 0;
    if (*it)
      state = (*it)->OnHittest(pt, blbtndown);
    
    switch (state)
    {
    case BEHIDE:
      if ((*unit) == (*it))
        *unit = 0;
      DeleteBlock(it);
      return state;
    case BEPLAY:
      {
         if (!m_sigPlayback.empty())
           m_sigPlayback((*it)->m_mediadata); // emit a signal
      }
      return state;
    case  BEHITTEST:
      *unit = (*it);
      if ((*it)->m_mediadata.filmname.empty())
        m_tipstring = (*it)->m_mediadata.filename;
      else
        m_tipstring = (*it)->m_mediadata.filmname;
      stRet = BEHITTEST;
    }
  }
  m_tipstring = L"";
  return stRet;
}

RECT BlockList::GetScrollBarHittest()
{
  return m_scrollbar->GetHittest();
}

void BlockList::DeleteBlock(std::list<BlockUnit*>::iterator it)
{
  // add it into hide list
  //(*it)->m_itFile->file_data.bHide = true;
  (*it)->m_mediadata.bHide = TRUE;
  m_listHide.push_back(*it);

  // tell media center
  MediaCenterController::GetInstance()->HandleDelBlock(*it);

  (*it)->DeleteLayer();
  (*it) = NULL;

  if (it == m_start)
  {
    if (m_start == m_list->begin())
    {
      m_list->erase(it);
      m_start = m_list->begin();
    }
    else
    {
      --m_start;
      m_list->erase(it);
    }
  }
  else
    m_list->erase(it);
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

int  BlockList::GetEnableShowAmount()
{
  return m_x.size() * m_y.size();
}

void BlockList::GetLastBlockPosition(RECT& rc)
{
  if (m_start == m_list->end())
  {
    RECT r = {-m_blockw, 0, 0, m_blockh};
    rc = r;  
  }
  else
    rc = (*(--m_end))->GetHittest();
}

void BlockList::SetScrollBarInitializeFlag(BOOL bl)
{
  m_scrollbarinitialize = bl;
}

BOOL BlockList::GetScrollBarInitializeFlag()
{
  return m_scrollbarinitialize;
}

std::list<BlockUnit*>* BlockList::GetEmptyList()
{
  if (m_list1.empty())
    return &m_list1;

  if (m_list2.empty())
    return &m_list2;

  return 0;
}

std::list<BlockUnit*>* BlockList::GetIdleList()
{
  return m_list == &m_list1 ? &m_list2 : &m_list1;
}

void BlockList::SetScrollBarDragDirection(int offset)
{
  m_scrollbardirectionpre = m_scrollbardirection;

  if (offset > 0)
    m_scrollbardirection = 1;
  else if (offset < 0)
    m_scrollbardirection = -1;
  else
    m_scrollbardirection = 0;
}

BOOL BlockList::BeDirectionChange()
{
  return m_scrollbardirection != m_scrollbardirectionpre;
}

void BlockList::SwapListBuff(std::list<BlockUnit*>::iterator& it, BOOL upordown)
{
  if (GetIdleList()->empty())
    return;

  if (MediaCenterController::GetInstance()->LoadMediaDataAlive())
    return;

  int count = 0;

  std::list<BlockUnit*>::iterator ittmp;
  if (upordown)
  {
    ittmp = it;
    while (ittmp != m_logicalend)
    {
      ++count;
      ++ittmp;
    }
    
    if (count <= m_viewcapacity) 
    {
      m_list = GetIdleList();
      if (m_list)
      {
        it = m_list->begin();
        ClearList(GetIdleList());
      }
    }
  }
  else
  {
    if (it != m_list->begin())
      return;
    
    ittmp = it;
    while (ittmp != m_end)
    {
      ++ittmp;
      ++count;
    }
    
    if (count <= m_viewcapacity)
    {
      m_list = GetIdleList();
      if (m_list)
      {
        int maxsize = m_viewcapacity + m_remainitem;
        ittmp = m_list->end();
        while (maxsize--)
          --ittmp;
        it = ittmp;
        ClearList(GetIdleList());
      }
    }
  }
}

void BlockList::ClearList(std::list<BlockUnit*>* list)
{
  std::list<BlockUnit*>::iterator it = list->begin();
  while (it != list->end())
  {
    (*it)->DeleteLayer();
    delete *it;
    ++it;
  }

  list->clear();
}

void BlockList::CalculateViewCapacity()
{
  int height = (int)m_blockh + (int)m_top;
  m_viewcapacity = ((int)m_winh + height - 1) / height * m_x.size();
}

void BlockList::CalculateLogicalListEnd()
{ 
  if (m_list->empty())
    return;

  if (m_list->size() < m_listsize)
  {
    m_logicalend = m_list->end();
    return;
  }

  m_remainitem = m_list->size() % m_maxcolumn;

  std::list<BlockUnit*>::iterator it = m_list->end();
  int remain = m_remainitem;

  while (remain-- && it != m_list->begin())
    --it;

  if (it == m_list->begin())
  {
    it = m_list->end();
    m_remainitem = 0;
  }

  m_logicalend = it;
}

int BlockList::GetViewCapacity()
{
  return m_viewcapacity;
}

int BlockList::GetListRemainItem()
{
  return m_remainitem;
}

int BlockList::GetListCapacity()
{
  return m_listsize;
}

void BlockList::SetSizeChanged()
{
  m_bSizeChanged = true;
}

//BlockListView

BlockListView::BlockListView():
 m_lbtndown(FALSE)
{
  RECT rc = {0,0,0,0};
  m_prehittest = rc;
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

void BlockListView::HandleLButtonDown(POINT pt, BlockUnit** unit)
{
  RECT rcclient;
  GetClientRect(m_hwnd, &rcclient);

  int bscroll = OnScrollBarHittest(pt, TRUE, *m_scrollspeed, m_hwnd);
  int layerstate = OnHittest(pt, TRUE, unit);
  
  if (bscroll == ScrollBarClick)
  {
    ::SetCapture(m_hwnd);
    m_lbtndown = TRUE;
  }
  
  //if (layerstate == BEPLAY)
  //  SendMessage(m_hwnd, WM_LBUTTONUP, 0, 0);

  //if (layerstate == BEHIDE)
  //{
  //  Update(rcclient.right, rcclient.bottom);
  //  InvalidateRect(m_hwnd, &rcclient, FALSE);
  //  RECT rc = {0, 0, 0, 0};
  //  m_prehittest = rc;
  //}
}

void BlockListView::HandleLButtonUp(POINT pt, BlockUnit** unit)
{
  RECT rcclient;
  GetClientRect(m_hwnd, &rcclient);

  int bscroll = OnScrollBarHittest(pt, FALSE, *m_scrollspeed, m_hwnd);
  int blayer = OnHittest(pt, FALSE, unit);

  if (m_lbtndown)
  {
    ::ReleaseCapture();
    m_lbtndown = FALSE;
  }
   
  if (bscroll == ScrollBarHit)
  {
    RECT rc = GetScrollBarHittest();
    rc.top = 0;
    rc.bottom = rcclient.bottom;
    InvalidateRect(m_hwnd, &rc, FALSE);
  }
}

void BlockListView::HandleMouseMove(POINT pt, BlockUnit** unit)
{
  RECT rcclient;
  GetClientRect(m_hwnd, &rcclient);

  int bscroll = OnScrollBarHittest(pt, -1, *m_scrollspeed, m_hwnd);
  int blayer = OnHittest(pt, FALSE, unit);

  if (bscroll == ScrollBarClick)
    ::InvalidateRect(m_hwnd, &rcclient, FALSE);

  if (bscroll == ScrollBarHit && m_lbtndown)
    PostMessage(m_hwnd, WM_LBUTTONUP, 0, 0);

  if (bscroll == NoScrollBarHit && m_lbtndown)
  {
    RECT rc = GetScrollBarHittest();
    rc.top = 0;
    rc.bottom = rcclient.bottom;
    InvalidateRect(m_hwnd, &rc, FALSE);
    m_lbtndown = FALSE;
  }

  if (blayer == BEHITTEST)
  {
    SetTimer(m_hwnd, TIMER_TIPS, 500, NULL);
    ::InvalidateRect(m_hwnd, &m_prehittest, FALSE);
    ::InvalidateRect(m_hwnd, &((*unit)->GetHittest()), FALSE);
    if (*unit)
      m_prehittest = (*unit)->GetHittest();
  }

  HCURSOR hcursor;
  if (blayer == BEHITTEST || bscroll == ScrollBarHit || bscroll == ScrollBarClick)
    hcursor = LoadCursor(NULL, IDC_HAND);
  else
    hcursor = LoadCursor(NULL, IDC_ARROW);

  SetCursor(hcursor);

}

BOOL BlockListView::HandleRButtonUp(POINT pt, BlockUnit** unit, CMenu* menu)
{
  BOOL bhit = FALSE;

  int blayer = OnHittest(pt, FALSE, unit);
  
  if (blayer == BEHITTEST)
  {
    RECT rc;
    GetWindowRect(m_hwnd, &rc);
    pt.x += rc.left;
    pt.y += rc.top;
    menu->GetSubMenu(0)->TrackPopupMenu(TPM_RIGHTBUTTON|TPM_NOANIMATION, pt.x, pt.y, CWnd::FromHandle(m_hwnd));
    bhit = TRUE;
  }

  return bhit;
}