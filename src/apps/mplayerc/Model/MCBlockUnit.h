#pragma once

#include "MediaComm.h"
#include "../UserInterface/Renderer/UILayerBlock.h"

class BlockUnit
{
public:
  BlockUnit();
  ~BlockUnit();

  void DefLayer();
  void DeleteLayer();
  void DoPaint(WTL::CDC& dc, POINT& pt);

  void SetCover();
  void CleanCover();
  
  void SetDisplay(BOOL show = TRUE);

  CRect GetTextRect();
  BOOL ActMouseMove(const POINT& pt);
  BOOL ActMouseOver(const POINT& pt);
  BOOL ActMouseOut(const POINT& pt);
  int ActMouseDown(const POINT& pt);
  BOOL ActLButtonUp(const POINT& pt);
  BOOL ActRButtonUp(const POINT& pt);

public:
  MediaData m_mediadata;

private:
  int m_coverwidth;
  int m_coverheight;

  UILayerBlock* m_layer;
  BOOL m_display;
  HBITMAP m_cover;
  CRect m_rcText;
};