#include "stdafx.h"
#include "MCBlockUnit.h"
#include "../UserInterface/Renderer/MCUILayer.h"
#include "ResLoader.h"
#include "..\Controller\MediaCenterController.h"

BlockUnit::BlockUnit() :
m_layer(new UILayerBlock),
m_cover(NULL),
m_display(FALSE),
m_coverwidth(0),
m_coverheight(0)
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
  //UILayer* favourite = NULL;
  //UILayer* cover = NULL;

  BOOL ret = FALSE;
  BOOL bbg = m_layer->GetUILayer(L"background", &bg);
  BOOL bplay = m_layer->GetUILayer(L"play", &play);
  BOOL bhide = m_layer->GetUILayer(L"hide", &hide);
  //BOOL bfavourite = m_layer->GetUILayer(L"favourite", &favourite);
  //BOOL bcover = m_layer->GetUILayer(L"insertcover", &cover);

  if (!bbg || !bplay || !bhide /* || !bfavourite || !bcover */)
    return FALSE;

  play->SetDisplay(TRUE);

  RECT rcHide = {0};
  hide->GetTextureRect(rcHide);
  rcHide.left -= 20;  // Expand the size of the rect
  rcHide.top -= 20;
  rcHide.right += 20;
  rcHide.bottom += 20;

  if (PtInRect(&rcHide, pt))
  {
    hide->SetDisplay(TRUE);
    MediaCenterController::GetInstance()->Render();
  }
  else
  {
    hide->SetDisplay(FALSE);
    MediaCenterController::GetInstance()->Render();
  }

  //favourite->SetDisplay(TRUE);
  //cover->SetDisplay(TRUE);

  if (bg->ActMouseOver(pt))
    ret = TRUE;
  if (play->ActMouseOver(pt))
    ret = TRUE;
  if (hide->ActMouseOver(pt))
    ret = TRUE;
  //if (favourite->ActMouseOver(pt))
  //  ret = TRUE;
  //if (cover->ActMouseOver(pt))
  //  ret = TRUE;

  // set hand cursor
  MediaCenterController::GetInstance()->SetCursor();

  return ret;
}

BOOL BlockUnit::ActMouseOut(const POINT& pt)
{
  UILayer* bg = NULL;
  UILayer* play = NULL;
  UILayer* hide = NULL;
  //UILayer* favourite = NULL;
  //UILayer* cover = NULL;

  BOOL ret = FALSE;

  BOOL bbg = m_layer->GetUILayer(L"background", &bg);
  BOOL bplay = m_layer->GetUILayer(L"play", &play);
  BOOL bhide = m_layer->GetUILayer(L"hide", &hide);
  //BOOL bfavourite = m_layer->GetUILayer(L"favourite", &favourite);
  //BOOL bcover = m_layer->GetUILayer(L"insertcover", &cover);
  if (!bbg || !bplay || !bhide /* || !bfavourite || !bcover */)
    return ret;

  
  if (bg->ActMouseOut(pt))
    ret = TRUE;
  if (play->ActMouseOut(pt))
    ret = TRUE;
  if (hide->ActMouseOut(pt))
    ret = TRUE;
  //if (favourite->ActMouseOut(pt))
  //  ret = TRUE;
  //if (cover->ActMouseOut(pt))
  //  ret = TRUE;

  play->SetDisplay(FALSE);
  hide->SetDisplay(FALSE);
  //favourite->SetDisplay(FALSE);
  //cover->SetDisplay(FALSE);

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
  //BOOL bcover = m_layer->GetUILayer(L"insertcover", &cover);
  if (!bplay || !bhide /* || !bcover */)
    return FALSE;

  if (play->ActMouseLBDown(pt))
    return 1;

  if (hide->ActMouseLBDown(pt))
    return 2;

  //if (cover->ActMouseLBDown(pt))
  //  return 3;

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

  // hide the unit if needed
  Hide(pt);

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
  //m_layer->AddUILayer(L"favourite", new ULFavourite(L"\\skin\\favourite.png", FALSE, 2));
  //m_layer->AddUILayer(L"insertcover", new ULCover(L"\\skin\\cover.png", FALSE, 2));
}

void BlockUnit::DoPaint(WTL::CDC& dc, POINT& pt)
{
  if (!m_display)
    return;

  UILayer* layer = NULL;
  UILayer* def = NULL;
  UILayer* play = NULL;
  UILayer* hide = NULL;
  //UILayer* favourite = NULL;
  //UILayer* cover = NULL;

  BOOL bmark = m_layer->GetUILayer(L"background", &layer);
  BOOL bdef = m_layer->GetUILayer(L"def", &def);
  BOOL bplay = m_layer->GetUILayer(L"play", &play);
  BOOL bhide = m_layer->GetUILayer(L"hide", &hide);
  //BOOL bfavourite = m_layer->GetUILayer(L"favourite", &favourite);
  //BOOL bcover = m_layer->GetUILayer(L"insertcover", &cover);

  if (!bmark || !bdef || !bplay || !bhide /* || !bfavourite || !bcover */)
    return;

  layer->SetDisplay(TRUE);
  def->SetDisplay(TRUE);

  POINT layer_fixpt = {pt.x + 10, pt.y};
  POINT play_fixpt = {40, 16};
  POINT def_fixpt = {15, 4};
  POINT hide_fixpt = {92, 78};
  //POINT favourite_fixpt = {28, 74};
  //POINT cover_fixpt = {56, 74};

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

  //POINT favouritept = {favourite_fixpt.x + layer_fixpt.x, favourite_fixpt.y + layer_fixpt.y};
  //favourite->SetTexturePos(favouritept);

  //POINT coverpt = {cover_fixpt.x + layer_fixpt.x, cover_fixpt.y + layer_fixpt.y};
  //cover->SetTexturePos(coverpt);

  SetCover();
  if (m_cover)
  {
    def->TiledTexture(m_coverwidth, m_coverheight);
    def->SetDisplayWH(m_coverwidth, m_coverheight);
  }

  m_layer->DoPaint(dc);

  dc.SetBkMode(TRANSPARENT);

  RECT rc;
  layer->GetTextureRect(rc);
  rc.left = pt.x;
  rc.right += 10;
  rc.top = pt.y+117;
  rc.bottom = rc.top+20;

  m_rcText = rc;

  std::wstring fmnm = m_mediadata.filename;  // just use filename now

  HFONT hOldFont = dc.SelectFont(MediaCenterController::GetInstance()->GetFilmTextFont());
  dc.DrawText(fmnm.c_str(), fmnm.size(), &rc, DT_END_ELLIPSIS|DT_CENTER|DT_VCENTER|DT_SINGLELINE);
  dc.SelectFont(hOldFont);
}

void BlockUnit::DeleteLayer()
{
  m_layer->DeleteAllLayer();
  delete m_layer;
}

void BlockUnit::Hide(const POINT &pt)
{
  UILayer *hide = 0;
  BOOL bhide = m_layer->GetUILayer(L"hide", &hide);
  if (bhide)
  {
    RECT rc;
    hide->GetTextureRect(rc);
    if (PtInRect(&rc, pt))
      // hide the media
      MediaCenterController::GetInstance()->HandleDelBlock(m_mediadata);
  }
}

void BlockUnit::SetCover()
{
  using namespace boost::filesystem;

  ResLoader resloader;
  UILayer *def = 0;
  m_layer->GetUILayer(L"def", &def);
  boost::system::error_code err;
  if (!m_cover && exists(m_mediadata.thumbnailpath, err))
  {
    // If the thumbnail exist then load it
    HBITMAP hCover = resloader.LoadBitmapFromDisk(m_mediadata.thumbnailpath, false);
    if (hCover && def)
    {
      m_cover = hCover;  // assign value
      if (!m_coverwidth && !m_coverheight)
        def->GetTextureWH(m_coverwidth, m_coverheight);
      def->SetTexture(m_cover);
    }
  }

  if (!m_cover && !m_mediadata.filename.empty())
  {
    // If the thumbnail not exist then load the default picture
    // the default picture maybe deleted in CleanCover in somewhere
    HBITMAP hCover = resloader.LoadBitmap(L"\\skin\\def.png");
    if (hCover && def)
    {
      m_cover = hCover;
      def->SetTexture(hCover, 2);
    }
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

void BlockUnit::SetDisplay(BOOL show)
{
  m_display = show;
}