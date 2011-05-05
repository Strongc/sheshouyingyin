#include "stdafx.h"
#include "UILayer.h"
#include <ResLoader.h>
#include "logging.h"
UILayer::UILayer(std::wstring respath, BOOL display /* = TRUE */)
{
  ResLoader rs;
  SetTexture(rs.LoadBitmap(respath));
  m_fixdisplay = display;
  SetDisplay(FALSE);
}

UILayer::~UILayer()
{

}

BOOL UILayer::SetTexture(HBITMAP texture)
{
  if (texture == NULL)
    return FALSE;

  m_texture.Attach(texture);

  m_texture.GetBitmap(&m_bm);

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

  return TRUE;
}

BOOL UILayer::GetTextureRect(RECT& rc)
{
//   rc.top = m_texturepos.y;
//   rc.left = m_texturepos.x;
//   rc.right = m_texturepos.x + m_bm.bmWidth;
//   rc.bottom = m_texturepos.y + m_bm.bmHeight;
  rc = m_texturerect;

  return TRUE;
}

BOOL UILayer::GetTexturePos(POINT& pt)
{
  pt = m_texturepos;
  return TRUE;
}

BOOL UILayer::SetTexturePos(const POINT& pt)
{
  m_texturerect.left = pt.x;
  m_texturerect.top = pt.y;
  m_texturerect.right = pt.x + m_bm.bmWidth;
  m_texturerect.bottom = pt.y + m_bm.bmHeight;
  return TRUE;
}

BOOL UILayer::SetTexturePos(const POINT& pt, int width, int height)
{
  //m_texturepos = pt;
  m_texturerect.left = pt.x;
  m_texturerect.top = pt.y;
  m_texturerect.right = pt.x + width;
  m_texturerect.bottom = pt.y + height;
  return TRUE;
}

BOOL UILayer::SetDisplay(BOOL display)
{
  m_display = display;
  return TRUE;
}

BOOL UILayer::DoPaint(WTL::CDC& dc)
{
  if (!m_display || !m_fixdisplay)
    return FALSE;

  WTL::CDC texturedc;
  HBITMAP hold_texture;

  texturedc.CreateCompatibleDC(dc);
  hold_texture = texturedc.SelectBitmap(m_texture);

  BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  if (m_bm.bmBitsPixel == 32)
    dc.AlphaBlend(m_texturerect.left, m_texturerect.top, m_texturerect.Width(), m_texturerect.Height(),
                  texturedc, 0, 0, m_bm.bmWidth, m_bm.bmHeight, bf);
  else
  {
    dc.SetStretchBltMode(HALFTONE);
    dc.StretchBlt(m_texturerect.left, m_texturerect.top, m_texturerect.Width(), m_texturerect.Height(),
                  texturedc, 0, 0, m_bm.bmWidth, m_bm.bmHeight, SRCCOPY);
  }
  
  texturedc.SelectBitmap(hold_texture);
  texturedc.DeleteDC();
  
  return TRUE;
}

BOOL UILayer::OnHittest(POINT pt, BOOL blbtndown, int extent)
{
  if (!m_fixdisplay)
    return FALSE;

//   RECT hittestrc = {m_texturepos.x, m_texturepos.y, m_texturepos.x + m_bm.bmWidth,
//              m_texturepos.y + m_bm.bmHeight + extent};
  BOOL bl = PtInRect(&m_texturerect, pt);
  if (bl)
    SetDisplay(TRUE);
  else
    SetDisplay(FALSE);
  return bl;
}
