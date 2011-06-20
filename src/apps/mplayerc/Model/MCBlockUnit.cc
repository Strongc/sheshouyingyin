#include "stdafx.h"
#include "MCBlockUnit.h"
#include "../UserInterface/Renderer/MCUILayer.h"
#include "ResLoader.h"
#include "..\Controller\MediaCenterController.h"

BlockUnit::BlockUnit() :
m_layer(new UILayerBlock),
m_cover(NULL)
{
}

BlockUnit::~BlockUnit()
{
  DeleteLayer();
}

BOOL BlockUnit::ActMouseMove(const POINT& pt)
{
  BOOL ret = FALSE;

  if (!m_layer)
    return ret;

  UILayer* layer = NULL;
  BOOL bmark = m_layer->GetUILayer(L"background", &layer);

  RECT rc;
  layer->GetTextureRect(rc);
  if (PtInRect(&rc, pt))
  {
    ret = TRUE;

    // set status message
    std::wstring message = m_mediadata.path + m_mediadata.filename;
    MediaCenterController::GetInstance()->SetStatusText(message);
  }

  return ret;
}

BOOL BlockUnit::ActMouseOver(const POINT& pt)
{
  UILayer* bg = NULL;
  UILayer* play = NULL;
  UILayer* hide = NULL;
  UILayer* favourite = NULL;
  UILayer* cover = NULL;

  BOOL ret = FALSE;
  BOOL bbg = m_layer->GetUILayer(L"background", &bg);
  BOOL bplay = m_layer->GetUILayer(L"play", &play);
  BOOL bhide = m_layer->GetUILayer(L"hide", &hide);
  BOOL bfavourite = m_layer->GetUILayer(L"favourite", &favourite);
  BOOL bcover = m_layer->GetUILayer(L"insertcover", &cover);

  if (!bbg || !bplay || !bhide || !bfavourite || !bcover)
    return FALSE;

  play->SetDisplay(TRUE);
  hide->SetDisplay(TRUE);
  favourite->SetDisplay(TRUE);
  cover->SetDisplay(TRUE);

  if (bg->ActMouseOver(pt))
    ret = TRUE;
  if (play->ActMouseOver(pt))
    ret = TRUE;
  if (hide->ActMouseOver(pt))
    ret = TRUE;
  if (favourite->ActMouseOver(pt))
    ret = TRUE;
  if (cover->ActMouseOver(pt))
    ret = TRUE;

  // set hand cursor
  MediaCenterController::GetInstance()->SetCursor();

  return ret;
}

BOOL BlockUnit::ActMouseOut(const POINT& pt)
{
  UILayer* bg = NULL;
  UILayer* play = NULL;
  UILayer* hide = NULL;
  UILayer* favourite = NULL;
  UILayer* cover = NULL;

  BOOL ret = FALSE;

  BOOL bbg = m_layer->GetUILayer(L"background", &bg);
  BOOL bplay = m_layer->GetUILayer(L"play", &play);
  BOOL bhide = m_layer->GetUILayer(L"hide", &hide);
  BOOL bfavourite = m_layer->GetUILayer(L"favourite", &favourite);
  BOOL bcover = m_layer->GetUILayer(L"insertcover", &cover);
  if (!bbg || !bplay || !bhide || !bfavourite || !bcover)
    return ret;

  
  if (bg->ActMouseOut(pt))
    ret = TRUE;
  if (play->ActMouseOut(pt))
    ret = TRUE;
  if (hide->ActMouseOut(pt))
    ret = TRUE;
  if (favourite->ActMouseOut(pt))
    ret = TRUE;
  if (cover->ActMouseOut(pt))
    ret = TRUE;

  play->SetDisplay(FALSE);
  hide->SetDisplay(FALSE);
  favourite->SetDisplay(FALSE);
  cover->SetDisplay(FALSE);
  CString str;str.Format(L"%d\n", ret);
  OutputDebugString(str);

  // set status message
  MediaCenterController::GetInstance()->SetStatusText(L"");

  // set arrow cursor
  MediaCenterController::GetInstance()->SetCursor(IDC_ARROW);

  return ret;
}

int BlockUnit::ActMouseDown(const POINT& pt)
{
  UILayer* play = NULL;
  UILayer* hide = NULL;
  UILayer* cover = NULL;

  BOOL bplay = m_layer->GetUILayer(L"play", &play);
  BOOL bhide = m_layer->GetUILayer(L"hide", &hide);
  BOOL bcover = m_layer->GetUILayer(L"insertcover", &cover);
  if (!bplay || !bhide || !bcover)
    return FALSE;

  if (play->ActMouseLBDown(pt))
    return 1;

  if (hide->ActMouseLBDown(pt))
    return 2;

  if (cover->ActMouseLBDown(pt))
    return 3;

  return 0;
}

BOOL BlockUnit::ActLButtonUp(const POINT& pt)
{
  BOOL bl = FALSE;

  UILayer* play = NULL;
  BOOL bplay = m_layer->GetUILayer(L"play", &play);
  if (bplay)
  {
    RECT rc;
    play->GetTextureRect(rc);
    if (PtInRect(&rc, pt))
    {
      // notify the mc to playback a file
      MediaCenterController::GetInstance()->HandlePlayback(m_mediadata);
    }
  }

  return bl;
}

BOOL BlockUnit::ActRButtonUp(const POINT& pt)
{
  BOOL ret = FALSE;

  if (!m_layer) return ret;

  UILayer* layer = 0;
  BOOL bmark = m_layer->GetUILayer(L"background", &layer);
  if (!bmark) return ret;

  RECT rc;
  layer->GetTextureRect(rc);
  if (PtInRect(&rc, pt))
    ret = TRUE;

  return ret;
}

void BlockUnit::DefLayer()
{
  m_layer->AddUILayer(L"background", new ULBackground(L"\\skin\\mark.png", FALSE, 2));
  m_layer->AddUILayer(L"def", new UILayer(L"\\skin\\def.png", FALSE, 2));
  m_layer->AddUILayer(L"play", new ULPlayback(L"\\skin\\play.png", FALSE, 2));
  m_layer->AddUILayer(L"hide", new ULDel(L"\\skin\\hide.png", FALSE, 2));
  m_layer->AddUILayer(L"favourite", new ULFavourite(L"\\skin\\favourite.png", FALSE, 2));
  m_layer->AddUILayer(L"insertcover", new ULCover(L"\\skin\\cover.png", FALSE, 2));
}

void BlockUnit::DoPaint(WTL::CDC& dc, POINT& pt)
{
  UILayer* layer = NULL;
  UILayer* def = NULL;
  UILayer* play = NULL;
  UILayer* hide = NULL;
  UILayer* favourite = NULL;
  UILayer* cover = NULL;

  BOOL bmark = m_layer->GetUILayer(L"background", &layer);
  BOOL bdef = m_layer->GetUILayer(L"def", &def);
  BOOL bplay = m_layer->GetUILayer(L"play", &play);
  BOOL bhide = m_layer->GetUILayer(L"hide", &hide);
  BOOL bfavourite = m_layer->GetUILayer(L"favourite", &favourite);
  BOOL bcover = m_layer->GetUILayer(L"insertcover", &cover);

  if (!bmark || !bdef || !bplay || !bhide || !bfavourite || !bcover)
    return;

  layer->SetDisplay(TRUE);
  def->SetDisplay(TRUE);

  POINT layer_fixpt = {pt.x + 10, pt.y};
  POINT play_fixpt = {40, 16};
  POINT def_fixpt = {15, 4};
  POINT hide_fixpt = {83, 74};
  POINT favourite_fixpt = {28, 74};
  POINT cover_fixpt = {56, 74};

  //layer->SetTexturePos(pt);
  layer->SetTexturePos(layer_fixpt);

  POINT defpt = {def_fixpt.x+layer_fixpt.x, def_fixpt.y+layer_fixpt.y};
  def->SetTexturePos(defpt);

  //POINT playpt = {play_fixpt.x+pt.x, play_fixpt.y+pt.y};
  POINT playpt = {play_fixpt.x + layer_fixpt.x, play_fixpt.y + layer_fixpt.y};
  play->SetTexturePos(playpt);

  //POINT hidept = {hide_fixpt.x+pt.x, hide_fixpt.y+pt.y};
  POINT hidept = {hide_fixpt.x + layer_fixpt.x, hide_fixpt.y + layer_fixpt.y};
  hide->SetTexturePos(hidept);

  POINT favouritept = {favourite_fixpt.x + layer_fixpt.x, favourite_fixpt.y + layer_fixpt.y};
  favourite->SetTexturePos(favouritept);

  POINT coverpt = {cover_fixpt.x + layer_fixpt.x, cover_fixpt.y + layer_fixpt.y};
  cover->SetTexturePos(coverpt);

  SetCover();
  m_layer->DoPaint(dc);

  dc.SetBkMode(TRANSPARENT);

  RECT rc;
  layer->GetTextureRect(rc);
  rc.left = pt.x;
  rc.right += 10;
  rc.top = pt.y+117;
  rc.bottom = rc.top+20;

  m_rcText = rc;

  std::wstring fmnm = m_mediadata.filmname.empty() ? m_mediadata.filename : m_mediadata.filmname;

  HFONT hOldFont = dc.SelectFont(MediaCenterController::GetInstance()->GetFilmTextFont());
  dc.DrawText(fmnm.c_str(), fmnm.size(), &rc, DT_END_ELLIPSIS|DT_CENTER|DT_VCENTER|DT_SINGLELINE);
  dc.SelectFont(hOldFont);
}

void BlockUnit::DeleteLayer()
{
  m_layer->DeleteAllLayer();
  delete m_layer;
}

void BlockUnit::SetCover()
{
  if (!m_cover && !m_mediadata.thumbnailpath.empty())
  {
    ResLoader resLoad;
    UILayer* def = NULL;
    m_cover = resLoad.LoadBitmapFromDisk(m_mediadata.thumbnailpath, false);
    m_layer->GetUILayer(L"def", &def);
    if (m_cover)
      def->SetTexture(m_cover);
  }
}

void BlockUnit::CleanCover()
{
  ::DeleteObject(m_cover);
  m_cover = NULL;
}

CRect BlockUnit::GetTextRect()
{
  return m_rcText;
}
