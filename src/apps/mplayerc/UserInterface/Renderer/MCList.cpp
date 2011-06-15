#include "stdafx.h"
#include "MCList.h"
#include <logging.h>
#include "../../Controller/MediaCenterController.h"
#include "../../SUIButton.h"
#include <ResLoader.h>
  
SPMCList::SPMCList():
  m_dbsource(NULL),
  m_lockpaint(FALSE),
  m_selblockunit(NULL),
  m_listempty(TRUE),
  m_deltatime(0)
{
  m_dbsource = new MCDBSource;

  m_blockw = 140;
  m_blockh = 130;

  m_blockspacing = 34;
  m_blocktop = 25;

  m_wndmargin.left = 22;
  m_wndmargin.top = 26;
  m_wndmargin.right = 5;
  m_wndmargin.bottom = 22;

  AddScrollBar();
}

SPMCList::~SPMCList()
{
  delete m_dbsource;
  delete m_sbar;
}

SPScrollBar* SPMCList::GetScrollBar()
{
  return m_sbar;
}

CSize SPMCList::GetMCSize()
{
  return m_wndsize;
}

void SPMCList::AddScrollBar()
{
  m_sbar = new SPScrollBar(L"\\skin\\rol.png", TRUE, 2);
}

void SPMCList::DoPaint(WTL::CDC& dc, RECT& rcclient)
{

  if (m_listempty && !m_cover.IsNull())
  {
    WTL::CDC dcmem;
    HBITMAP  hold;

    dcmem.CreateCompatibleDC(dc);
    hold = dcmem.SelectBitmap(m_cover);

    SetStretchBltMode(dc, HALFTONE);
    SetBrushOrgEx(dc, 0, 0, NULL);
    StretchBlt(dc, rcclient.left, rcclient.top, rcclient.right, rcclient.bottom,
               dcmem, 0, 0, m_coversize.cx, m_coversize.cy, SRCCOPY);

    dcmem.SelectBitmap(hold);
    dcmem.DeleteDC();
    return;
  }

  if (!m_lockpaint)
  {
    if (m_sbar->GetDisplay())
    {
      m_sbar->SetClientRect(rcclient);
      m_sbar->DoPaint(dc);
    }
//     std::vector<int>::iterator itx =  m_x.begin();
//     std::vector<int>::iterator ity =  m_y.begin();
//     std::vector<int>::iterator it;
//     POINT pt;
//     for (;ity != m_y.end(); ity++)
//     {
//       pt.y = *ity;
//       for (it = itx; it != m_x.end(); it++)
//       {
//         pt.x = *it;
//         MCLoopInit(m_dbsource);
//         MCLoopCurData()->DoPaint(dc, pt);
//         MCLoopNextData();
//       }
//     }
    std::vector<int>::iterator itx =  m_x.begin();
    std::vector<int>::iterator ity =  m_y.begin();

    MCLoopList(m_dbsource)
      if (itx == m_x.end())
      {
        itx = m_x.begin();
        ++ity;
      }

      POINT pt = {*itx, *ity};
      MCLoopOne()->DoPaint(dc, pt);
      ++itx;
    MCEndLoop()
  }
}

void SPMCList::BlocksMouseMove(const POINT& pt)
{
  if (!m_selblockunit)
  {
    MCLoopList(m_dbsource)
      if (MCLoopOne()->ActMouseMove(pt))
      {
        m_selblockunit = MCLoopOne();
        if (m_selblockunit->ActMouseOver(pt))
          MediaCenterController::GetInstance()->Render();
        break;
      }
    MCEndLoop()
  }
  else if (m_selblockunit)
  {
    if (m_selblockunit->ActMouseMove(pt))
    {
      if (m_selblockunit->ActMouseOver(pt))
        MediaCenterController::GetInstance()->Render();
    }
    else
    {
      if (m_selblockunit->ActMouseOut(pt))
        MediaCenterController::GetInstance()->Render();
      m_selblockunit = NULL;
    }
  }
}

void SPMCList::Update(DWORD deltatime)
{
  int offset = m_sbar->GetSBarOffset();
  m_deltatime += deltatime;
  if (!offset || !deltatime)
    return;

  int status = m_dbsource->GetReaderStatus();
  if (status == MCDBSource::MCDB_UNKNOW)
    return;

  CString log;
  float v = (offset) / (float)(m_wndsize.cy/2/5) * 0.1f;
  int distance = deltatime * v;

  if (!distance)
    return;

  m_lockpaint = TRUE;
  BOOL dir = m_sbar->GetDirection();
  if (dir) // move up
  {
    if (status == MCDBSource::MCDB_TOSTART)
    {
      int heady = m_y.front();
      if (heady == m_blocktop)
      {
        m_lockpaint = FALSE;
        return;
      }
      
      if (heady + distance > m_blocktop)
        distance = m_blocktop - heady;

      std::vector<int>::iterator it = m_y.begin();
      for (; it != m_y.end(); it++)
        *it += distance;

      m_lockpaint = FALSE;
      return;
    }

    if (m_y.back() >= m_wndsize.cy)
    {
      m_dbsource->RemoveRowDatas(dir, m_x.size());
      m_y.pop_back();
    }

    if (m_y.front() >= m_blocktop)
    {
      status = m_dbsource->LoadRowDatas(dir, m_x.size());
      if (status == MCDBSource::MCDB_MORE)
      {
        m_y.insert(m_y.begin(), m_y.front()-m_blocktop-m_blockh);
      }
      else
      {
        m_lockpaint = FALSE;
        return;
      }
    }

    std::vector<int>::iterator it = m_y.begin();
    for (; it != m_y.end(); it++)
      *it += distance;
  }
  else// move down
  {
    if (status == MCDBSource::MCDB_TOEND)
    {
      m_lockpaint = FALSE;
      return;
    }

    if (m_y.front()+m_blockh <= 0)
    {
      m_dbsource->RemoveRowDatas(dir, m_x.size());
      m_y.erase(m_y.begin());
    }

    int taily = m_y.back()+m_blockh;
    if (m_wndsize.cy-taily >= m_blocktop)
    {
      status = m_dbsource->LoadRowDatas(dir, m_x.size());
      if (status == MCDBSource::MCDB_MORE)
      {
        m_y.push_back(taily + m_blocktop);
      }
      else
      {
        m_lockpaint = FALSE;
        return;
      }
    }

    std::vector<int>::iterator it = m_y.begin();
    for (; it != m_y.end(); it++)
      *it -= distance;
  }
  m_lockpaint = FALSE;
}

BOOL SPMCList::ActMouseMove(const POINT& pt)
{
  BOOL ret = FALSE;
  
  if (m_sbar->ActMouseMove(pt))
  {
    ;
  }
  else
  {
    BlocksMouseMove(pt);
  }

  return ret;
}

BOOL SPMCList::ActMouseLBDown(const POINT& pt)
{
  BOOL ret = FALSE;
  
  ret = m_sbar->ActMouseLBDown(pt);

  return ret;
}

BOOL SPMCList::ActMouseLBUp(const POINT& pt)
{
  BOOL ret = FALSE;

  ret = m_sbar->ActMouseLBUp(pt);

  m_deltatime = 0;
  return ret;
}

void SPMCList::InitMCList(int w, int h)
{
  SetMCRect(w, h);
  if (m_dbsource->PreLoad(m_blockcount))
  {
    m_listempty = FALSE;
    m_sbar->SetDisplay(m_dbsource->IsMoreData());
  }
  else
    SetCover();
}

void SPMCList::SetMCRect(int w, int h)
{
  m_wndsize.cx = w;
  m_wndsize.cy = h;

  AlignColumns();
  AlignRows();

  m_blockcount = m_x.size() * m_y.size();
}

BOOL SPMCList::ActWindowChange(int w, int h)
{
  SetMCRect(w, h);

  m_lockpaint = TRUE;

  if (!m_listempty)
    m_dbsource->AdjustRange(m_blockcount);
  
  m_dbsource->SetReadNums(m_blockcount);

  m_sbar->SetDisplay(m_dbsource->IsMoreData());

  m_lockpaint = FALSE;

  return TRUE;
}

void SPMCList::AlignColumns()
{
  int barwidth = GetScrollBar()->GetBarWidth();

  int w1 = m_wndsize.cx - barwidth - (m_wndmargin.left + m_wndmargin.right) + m_blockspacing;
  int w2 = m_blockw + m_blockspacing;
  int count = w1 / w2;

  int remainspacing = w1 - count * w2;

  int spacing = 0;
  if (remainspacing && count > 1)
    spacing = remainspacing / (count - 1) + m_blockspacing;

  m_x.clear();
  int x = m_wndmargin.left;

  while (count--)
  {
    m_x.push_back(x);
    x += m_blockw + spacing;
  }
}

void SPMCList::AlignRows()
{

  int y = m_y.empty() ? m_blocktop : m_y.front();
  int next = m_blockh + m_blocktop;
  int len = m_wndsize.cy + next;
  
  m_y.clear();

  while(y < len)
  {
    m_y.push_back(y);
    y += next;
  }
}

void SPMCList::SetCover()
{
  if (!m_cover.IsNull())
    return;

  ResLoader resLoader;
  HBITMAP cover = resLoader.LoadBitmap(L"skin\\mccover.jpg");

  if (cover)
  {
    CBitmap tmpcover;
    tmpcover.Attach(cover);
    if (CSUIButton::PreMultiplyBitmap(tmpcover, m_coversize, TRUE))
      m_cover = (HBITMAP)tmpcover.Detach();
    else
      tmpcover.Detach();
  }
}