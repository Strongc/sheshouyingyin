#include "StdAfx.h"
#include "MediaCenterScrollBar.h"
#include <ResLoader.h>

#define  TIMER_OFFSET 11
#define  TIMER_SLOWOFFSET 12

#define  ScrollBarClick 21
#define  ScrollBarHit   22
#define  NoScrollBarHit 23

MediaCenterScrollBar::MediaCenterScrollBar(void):
 m_lastlbtstate(FALSE)
,m_preoffset(0)
{
  m_prepos.x = 0;
  m_prepos.y = 0;
}

MediaCenterScrollBar::~MediaCenterScrollBar(void)
{
}

void MediaCenterScrollBar::CreatScrollBar(std::wstring respath)
{
  ResLoader rs;
  m_hbitmap = rs.LoadBitmap(respath);

  m_hbitmap.GetBitmap(&m_bm);

  if(m_bm.bmBitsPixel == 32)
  {
    for (int y=0; y<m_bm.bmHeight; y++)
    {
      BYTE * pPixel = (BYTE *) m_bm.bmBits + m_bm.bmWidth * 4 * y;
      for (int x=0; x<m_bm.bmWidth; x++)
      {
        pPixel[0] = pPixel[0] * pPixel[3] / 255; 
        pPixel[1] = pPixel[1] * pPixel[3] / 255; 
        pPixel[2] = pPixel[2] * pPixel[3] / 255; 
        pPixel += 4;
      }
    }
  }
}

void MediaCenterScrollBar::SetPosition(POINT pt)
{
  m_pos = pt;
  m_prepos = m_pos;
  UpdataHittest(pt);
}

void  MediaCenterScrollBar::SetDisPlay(BOOL dp)
{
  m_display = dp;
}

BOOL  MediaCenterScrollBar::GetDisPlay()
{
  return m_display;
}

BOOL MediaCenterScrollBar::DoPaint(WTL::CDC& dc)
{
  if (!m_display)
    return FALSE;

  WTL::CDC dcMem;
  HBITMAP  oldHbm;

  dcMem.CreateCompatibleDC(dc);
  oldHbm = dcMem.SelectBitmap(m_hbitmap);

  BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  dc.AlphaBlend(m_pos.x, m_pos.y, m_bm.bmWidth, m_bm.bmHeight,
    dcMem, 0, 0, m_bm.bmWidth, m_bm.bmHeight, bf);

  dcMem.SelectBitmap(oldHbm);
  dcMem.DeleteDC();

  return TRUE;
}

BOOL MediaCenterScrollBar::OnHittest(POINT pt, int bLbtdown, int& offsetspeed, HWND hwnd)
{
  int bhit = 0;
  if (bLbtdown == 1)
    m_prelbtpos = pt;
    
  if (bLbtdown == -1)
    bLbtdown = m_lastlbtstate;
  else
    m_lastlbtstate = bLbtdown;

  if (bLbtdown && PtInRect(&m_hittest, pt))
  {
    m_pos.y = m_prepos.y + pt.y - m_prelbtpos.y;
    m_pos.y = max(0, m_pos.y);
    m_pos.y = min(m_pos.y, m_winh - m_bm.bmHeight);

    int offset = m_pos.y - m_prepos.y;
    int i = (m_winh - m_bm.bmHeight) / 2 / 20;
/*    int j = (m_winh - m_bm.bmHeight) / 2 / 20;*/
    if (offset > 0)
      offsetspeed = (offset + i - 1) / i;
    else
      offsetspeed = (offset - i + 1) / i;
/*    DWORD timer = max(63 - (abs(offset) + j - 1) / j * 3, 1);*/
    DWORD timer = 42 - (abs(offset) + i - 1) / i * 2;
    if (m_preoffset != offset)
    {
      SetTimer(hwnd, TIMER_SLOWOFFSET, 1, 0);
      m_preoffset = offset;
    }
    else
      SetTimer(hwnd, TIMER_OFFSET, timer, NULL);
    
    bhit = ScrollBarClick;
  }
  else if (PtInRect(&m_hittest, pt) && ((m_pos.x != m_prepos.x) || (m_pos.y != m_prepos.y)))
  { 
    KillTimer(hwnd, TIMER_OFFSET);
    KillTimer(hwnd, TIMER_SLOWOFFSET);
    m_pos = m_prepos;
    bhit = ScrollBarHit; 
  }
  else
    bhit = NoScrollBarHit;

  UpdataHittest(m_pos);

  return bhit;
}

void  MediaCenterScrollBar::UpdataHittest(POINT pt)
{
  RECT rc = {pt.x - 2, pt.y - 2, pt.x + m_bm.bmWidth + 2, pt.y + m_bm.bmHeight + 2};
  m_hittest = rc;
}

RECT MediaCenterScrollBar::GetRect()
{
  RECT rc = {0, 0, m_bm.bmWidth, m_bm.bmHeight};
  return rc;
}
POINT MediaCenterScrollBar::GetPosition()
{
  return m_prepos;
}
float MediaCenterScrollBar::GetOffset()
{
  return m_offset;
}

void  MediaCenterScrollBar::SetScrollBarRange(float winh)
{
  m_winh = winh;
}

RECT  MediaCenterScrollBar::GetHittest()
{
  return m_hittest;
}