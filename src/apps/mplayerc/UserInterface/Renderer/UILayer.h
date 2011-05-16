#pragma once

class UILayer
{
public:
  UILayer(std::wstring respath, BOOL display = TRUE);
  ~UILayer();

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

private:
  WTL::CBitmap   m_texture;
  POINT          m_texturepos;
  BOOL           m_fixdisplay;
  BOOL           m_display;
  BITMAP         m_bm;
  WTL::CRect     m_texturerect;
};