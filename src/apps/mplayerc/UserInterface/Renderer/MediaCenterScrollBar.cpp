﻿#include "StdAfx.h"
#include "MediaCenterScrollBar.h"
#include <ResLoader.h>
#include "..\..\Controller\MediaCenterController.h"
#include "..\..\ChildView.h"

#define  TIMER_OFFSET 11
#define  TIMER_UPDATE 12
#define  TIMER_TIME 13

#define  ScrollBarClick 21
#define  ScrollBarHit   22
#define  NoScrollBarHit 23

MediaCenterScrollBar::MediaCenterScrollBar(void):
 m_lastlbtstate(FALSE)
,m_binitialize(FALSE)
,m_stat(0)
,m_prestat(0)
,m_intpos(0, 0)
,m_prepos(0, 0)
,m_predirction(0)
,m_offsettimeid(0)
,m_currdirction(0)
{
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

  m_bm.bmHeight /= 2;
}

void MediaCenterScrollBar::SetPosition(POINT pt)
{
  m_pos = pt;
  m_prepos = m_pos;
  m_intpos = m_pos;
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
    dcMem, 0, m_stat * m_bm.bmHeight, m_bm.bmWidth, m_bm.bmHeight, bf);

  dcMem.SelectBitmap(oldHbm);
  dcMem.DeleteDC();

  return TRUE;
}

extern DWORD updateTime;
extern DWORD deltaT;
DWORD timerid = 0;
DWORD accuracy;
extern void CALLBACK ProcessOffset(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

BOOL MediaCenterScrollBar::OnHittest(POINT pt, int bLbtdown, int& offsetdirection, float& offsetspeed, HWND hwnd)
{
  if (m_winh - m_bm.bmHeight < 0)
    return FALSE;

  m_prestat = m_stat;

  int bhit = 0;
  if (bLbtdown == 1)
    m_prelbtpos = pt;

  if (bLbtdown == -1)
    bLbtdown = m_lastlbtstate;
  else
    if (PtInRect(&m_hittest, pt))
      m_lastlbtstate = bLbtdown;

  if (bLbtdown && PtInRect(&m_hittest, pt))
  {
    m_prepos = m_pos;
    m_pos.y = m_intpos.y + pt.y - m_prelbtpos.y;
    m_pos.y = max(0, m_pos.y);
    m_pos.y = min(m_pos.y, m_winh - m_bm.bmHeight);

    int offset = m_pos.y - m_intpos.y;

    int i = (m_winh - m_bm.bmHeight) / 2 / 5;

    if (offset == 0)
      return 0;

    if (offset > 0)
      offsetdirection = 1;
    else
      offsetdirection = -1;

    m_currdirction = offsetdirection;
      
    offsetspeed = (abs(offset) + i -1) / i * 0.1;
    
    if (offsetdirection != m_predirction)
      deltaT = MAXINT;

    m_predirction = offsetdirection;
    
    updateTime = timeGetTime();

    m_offat.direction = offsetdirection;
    m_offat.speed = offsetspeed;
    
    TIMECAPS tc;
    if(timeGetDevCaps(&tc,sizeof(TIMECAPS))==TIMERR_NOERROR) 
    {
      accuracy=min(max(tc.wPeriodMin, 1),tc.wPeriodMax); 
      timeBeginPeriod(accuracy); 
      timeKillEvent(m_offsettimeid);
      m_offsettimeid = timeSetEvent(10, accuracy,(LPTIMECALLBACK)ProcessOffset, (DWORD)&m_offat, TIME_PERIODIC);
    }
    
    SetTimer(hwnd, TIMER_UPDATE, 1, NULL);
    
    bhit = ScrollBarClick;

    m_stat = 1;
  }
  else //if (PtInRect(&m_hittest, pt) && ((m_pos.x != m_prepos.x) || (m_pos.y != m_prepos.y)))
  { 
    if (PtInRect(&m_hittest, pt))//&& ((m_pos.x != m_prepos.x) || (m_pos.y != m_prepos.y)))
    {
      bhit = ScrollBarHit;
      m_stat = 1;
    }
    else
    {
      bhit = NoScrollBarHit;
      m_stat = 0;
    }
    
    timeKillEvent(m_offsettimeid);
    timeEndPeriod(accuracy);
    KillTimer(hwnd, TIMER_UPDATE);

    m_pos = m_intpos;
    m_prepos = m_intpos;
    m_lastlbtstate = FALSE;
    m_currdirction = 0;
  }

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
  return m_intpos;
}
float MediaCenterScrollBar::GetOffset()
{
  return m_offset;
}

void  MediaCenterScrollBar::SetScrollBarRange(float winh)
{
  m_winh = winh;
}

float MediaCenterScrollBar::GetScrollBarRange()
{
  return m_winh;
}

RECT  MediaCenterScrollBar::GetHittest()
{
  return m_hittest;
}

void MediaCenterScrollBar::SetInitializeFlag(BOOL bl)
{
  m_binitialize = bl;
}

BOOL MediaCenterScrollBar::GetInitializeFlag()
{
  return m_binitialize;
}

BOOL MediaCenterScrollBar::NeedRepaint()
{
  BOOL bl =  m_prestat == m_stat? FALSE : TRUE;
  if (!bl)
    bl = m_prepos == m_pos? FALSE : TRUE;

  return bl;
}

BOOL  MediaCenterScrollBar::BeOffseting()
{
  return m_currdirction == 0? FALSE : TRUE;
}