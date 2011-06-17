#pragma once

class UILayer
{
public:
  UILayer(std::wstring respath, BOOL display = TRUE, UINT nums = 1);
  virtual ~UILayer();

// Interface
public:
  BOOL SetTexture(HBITMAP texture, UINT nums = 1);
  BOOL SetTexturePos(const POINT& pt);
  BOOL SetDisplay(BOOL display = TRUE);
  BOOL GetDisplay();

  BOOL GetTexturePos(POINT& pt);
  BOOL GetTextureRect(RECT& rc);

  BOOL GetTextureWH(int& w, int& h);
  virtual BOOL DoPaint(WTL::CDC& dc);

  BOOL DeleteTexture();

  void SetState(int stat);
  int  GetState();

  virtual BOOL ActMouseMove(const POINT& pt) {return FALSE;}
  virtual BOOL ActMouseOver(const POINT& pt) {return FALSE;}
  virtual BOOL ActMouseOut(const POINT& pt) {return FALSE;}
  virtual BOOL ActMouseLBDown(const POINT& pt) {return FALSE;}
  virtual BOOL ActMouseLBUp(const POINT& pt) {return FALSE;}

private:
  WTL::CBitmap   m_texture;
  POINT          m_texturepos;
  BOOL           m_display;
  BITMAP         m_bm;
  int            m_stat;
};