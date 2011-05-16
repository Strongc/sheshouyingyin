#pragma once

class MediaCenterScrollBar
{
public:
  MediaCenterScrollBar(void);
  ~MediaCenterScrollBar(void);

  void  CreatScrollBar(std::wstring respath);
  void  SetPosition(POINT pt);
  void  SetDisPlay(BOOL dp);
  void  SetScrollBarRange(float winh);
  BOOL  DoPaint(WTL::CDC& dc);
  int  OnHittest(POINT pt, int bLbtdown, int& offsetspeed, HWND hwnd);
  void  UpdataHittest(POINT pt);
  RECT  GetRect();
  POINT GetPosition();
  float GetOffset();
  RECT  GetHittest();

private:
  WTL::CBitmap m_hbitmap;
  POINT   m_pos;
  POINT   m_prepos;
  POINT   m_prelbtpos;
  BOOL    m_display;
  BITMAP  m_bm;
  RECT    m_hittest;
  BOOL    m_lastlbtstate;
  float   m_offset;
  float   m_winh;
  int     m_preoffset;
  BOOL    m_binitialize;
};
