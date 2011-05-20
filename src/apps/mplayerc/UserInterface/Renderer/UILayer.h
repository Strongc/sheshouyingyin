#pragma once

class UILayer
{
public:
  UILayer(std::wstring respath, BOOL display = TRUE);
  virtual ~UILayer();

// Interface
public:
  BOOL SetTexture(HBITMAP texture);
  BOOL SetTexturePos(const POINT& pt);
  BOOL SetTexturePos(const POINT& pt, int width, int height);
  BOOL SetDisplay(BOOL display = TRUE);

  BOOL GetTexturePos(POINT& pt);
  BOOL GetTextureRect(RECT& rc);

  virtual BOOL DoPaint(WTL::CDC& dc);

  BOOL DeleteTexture();

  void SetState(int stat);
  int  GetState();

  virtual BOOL ActMouseOver(const POINT& pt) {return FALSE;}
  virtual BOOL ActMouseOut(const POINT& pt) {return FALSE;}
  virtual BOOL ActMouseDown(const POINT& pt) {return FALSE;}
  virtual BOOL ActMouseUp(const POINT& pt) {return FALSE;}

private:
  WTL::CBitmap   m_texture;
  POINT          m_texturepos;
  BOOL           m_fixdisplay;
  BOOL           m_display;
  BITMAP         m_bm;
  WTL::CRect     m_texturerect;
  int            m_stat;
};