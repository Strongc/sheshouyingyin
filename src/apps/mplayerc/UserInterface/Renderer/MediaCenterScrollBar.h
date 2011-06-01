#pragma once

class MediaCenterScrollBar
{
public:
  MediaCenterScrollBar(void);
  ~MediaCenterScrollBar(void);

  void  CreatScrollBar(std::wstring respath);
  void  SetPosition(POINT pt);
  void  SetDisPlay(BOOL dp);
  BOOL  GetDisPlay();
  void  SetScrollBarRange(float winh);
  float GetScrollBarRange();
  BOOL  DoPaint(WTL::CDC& dc);
  BOOL  OnHittest(POINT pt, BOOL bLbtdown, int& offsetdirection, float& offsetspeed, HWND hwnd);
  void  UpdataHittest(POINT pt);
  RECT  GetRect();
  POINT GetPosition();
  float GetOffset();
  RECT  GetHittest();
  void  SetInitializeFlag(BOOL bl);
  BOOL  GetInitializeFlag();
  BOOL  NeedRepaint();
  BOOL  BeOffseting();

private:
  WTL::CBitmap m_hbitmap;
  // scrollbar current position
  WTL::CPoint   m_pos;
  // scrollbar initialize position
  WTL::CPoint   m_intpos;
  // scrollbar prvious position
  WTL::CPoint   m_prepos;
  POINT   m_prelbtpos;
  BOOL    m_display;
  BITMAP  m_bm;
  RECT    m_hittest;
  BOOL    m_lastlbtstate;
  float   m_offset;
  float   m_winh;
  BOOL    m_binitialize;
  int     m_stat;
  int     m_prestat;
  int     m_predirction;
};
