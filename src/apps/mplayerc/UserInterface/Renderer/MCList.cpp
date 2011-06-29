#include "stdafx.h"
#include "MCList.h"
#include <logging.h>
#include "../../Controller/MediaCenterController.h"
#include "../../SUIButton.h"
#include <ResLoader.h>
#include "../../MainFrm.h"
#include <math.h>
  
SPMCList::SPMCList():
  m_dbsource(NULL),
  m_lockpaint(FALSE),
  m_selblockunit(NULL),
  m_listempty(TRUE),
  m_maxsbar(0),
  m_maxoffset(0),
  m_rowpos(0),
  m_updaterowpos(0),
  m_mousewheel(FALSE),
  m_mousewheelcount(0),
  m_anispeed(0.f)
{
  m_dbsource = new MCDBSource;

  m_blockw = 140;
  m_blockh = 130;

  m_blockspacing = 34;
  m_blocktop = 25;

  m_wndmargin.left = 22;
  m_wndmargin.top = 26;
  m_wndmargin.right = 5;
  m_wndmargin.bottom = 22;

  AddScrollBar();

  m_ulmask = new ULMask(L"\\skin\\mask.png", TRUE);
  m_uiopenbtn = new ULMCOpenButton(L"\\skin\\open.png", TRUE, 2);
}

SPMCList::~SPMCList()
{
  delete m_dbsource;
  delete m_sbar;
}

ULMCOpenButton* SPMCList::GetOpenFileBtn()
{
  return !m_uiopenbtn ? NULL : m_uiopenbtn;
}

SPScrollBar* SPMCList::GetScrollBar()
{
  return !m_sbar ? NULL : m_sbar;
}

CSize SPMCList::GetMCSize()
{
  return m_wndsize;
}

void SPMCList::AddScrollBar()
{
  m_sbar = new SPScrollBar(L"\\skin\\rol.png", TRUE, 2);
}

void SPMCList::DoPaint(WTL::CDC& dc, RECT& rcclient)
{

  if (m_listempty && !m_cover.IsNull())
  {
    WTL::CDC dcmem;
    HBITMAP  hold;

    dcmem.CreateCompatibleDC(dc);
    hold = dcmem.SelectBitmap(m_cover);

    CPoint piCenter = CRect(rcclient).CenterPoint();

    SetBrushOrgEx(dc, 0, 0, NULL);
    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    AlphaBlend(dc, piCenter.x - m_coversize.cx / 2, piCenter.y - m_coversize.cy / 2, m_coversize.cx, m_coversize.cy,
               dcmem, 0, 0, m_coversize.cx, m_coversize.cy, bf);

    dcmem.SelectBitmap(hold);
    dcmem.DeleteDC();
  }
  else
  {
    if (!m_lockpaint)
    {
      if (m_sbar->GetDisplay())
      {
        m_sbar->SetClientRect(rcclient);
        m_sbar->DoPaint(dc);
      }

      POINT pt;
      int rowpos = m_rowpos;
      int colpos = 0;

      MCDBSource::BUPOINTER sp, ep;
      m_dbsource->GetPointer(sp, ep);

      for (int y=1; y<=m_maxrows; ++y)
      {
        pt.y = rowpos;
        colpos = m_columnpos;
        for (int x=1; x<=m_maxcolumns; ++x)
        {
          pt.x = colpos;
          (*sp)->DoPaint(dc, pt);
          colpos = m_fixcolumnwidth * x + m_columnpos;
          if (sp != ep)
            sp++;
        }
        rowpos = m_fixrowheight * y + m_rowpos;
      }

      m_ulmask->DoPaint(dc);
    }
  }
}

void SPMCList::HideData()
{
  if (m_selblockunit)
  {
    m_dbsource->HideData(m_selblockunit->m_mediadata);
    m_dbsource->Reload();
    m_selblockunit = NULL;
  }
}

void SPMCList::BlocksMouseMove(const POINT& pt)
{
  if (!m_selblockunit)
  {
    MCLoopList(m_dbsource)
      if (MCLoopOne()->ActMouseMove(pt))
      {
        m_selblockunit = MCLoopOne();
        if (m_selblockunit->ActMouseOver(pt))
          MediaCenterController::GetInstance()->Render();
        break;
      }
      else if (MCLoopOne()->GetTextRect().PtInRect(pt))
      {
        // set if point in the text area
        m_selblockunit = MCLoopOne();
      }
    MCEndLoop()
  }
  else if (m_selblockunit)
  {
    if (m_selblockunit->ActMouseMove(pt))
    {
      if (m_selblockunit->ActMouseOver(pt))
        MediaCenterController::GetInstance()->Render();
    }
    else if (!m_selblockunit->GetTextRect().PtInRect(pt))
    {
      if (m_selblockunit->ActMouseOut(pt))
        MediaCenterController::GetInstance()->Render();
      m_selblockunit = NULL;
    }
  }
}

BOOL SPMCList::ActMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
  BOOL ret = TRUE;

  if (!m_mousewheelcount)
    MediaCenterController::GetInstance()->Update();

  m_mousewheel = TRUE;
  m_mousewheelcount = 40;
  m_anispeed = 0.00005f * pow(100.f, 2.0f);

  if (zDelta < 0) // move down
    m_sbardir = FALSE;
  else // move up
    m_sbardir = TRUE;

  return ret;
}

void SPMCList::Update(DWORD deltatime)
{
  if (m_mousewheel && --m_mousewheelcount<=0)
  {
    m_mousewheel = FALSE;
    m_rowpos = m_updaterowpos;
    MediaCenterController::GetInstance()->StopUpdate();
    m_mousewheelcount = 0;
    return;
  }

  int status = m_dbsource->GetReaderStatus();

  m_lockpaint = TRUE;

  m_rowpos += (m_sbardir?1:-1) * (int)((float)deltatime * m_anispeed);

  if (m_sbardir) // move up
  {
    if (status != MCDBSource::MCDB_TOSTART)
    {
      int tail = (m_fixrowheight * (m_maxrows - 1)) + m_rowpos;
      if (tail >= m_wndsize.cy)
      {
        status = m_dbsource->LoadRowDatas(m_sbardir, m_maxcolumns);
        if (status != MCDBSource::MCDB_TOSTART)
          m_rowpos = m_rowpos - m_fixrowheight;
      }
    }
    else if (m_rowpos > m_wndsize.cy / 3)
    {
      m_rowpos = m_wndsize.cy / 3;
      m_updaterowpos = m_blocktop;
    }
  }
  else  // move down
  {
    if (status != MCDBSource::MCDB_TOEND)
    {
      int head = m_rowpos + m_fixrowheight;
      if (head <= 0)
      {
        status = m_dbsource->LoadRowDatas(m_sbardir, m_maxcolumns);
        if (status != MCDBSource::MCDB_TOEND)
          m_rowpos = head;
      }
    }
    else if (m_rowpos < m_wndsize.cy - m_fixrowheight - (m_fixrowheight*(m_maxrows-1)) - m_wndsize.cy / 3)
    {
      m_rowpos = m_wndsize.cy - m_fixrowheight - (m_fixrowheight*(m_maxrows-1)) - m_wndsize.cy / 3;
      m_updaterowpos = m_wndsize.cy - m_fixrowheight - (m_fixrowheight*(m_maxrows-1));
    }
  }

  m_lockpaint = FALSE;
}

BOOL SPMCList::ActMouseMove(const POINT& pt)
{
  BOOL ret = FALSE;
  UINT offset = 0;

  if (m_sbar->ActMouseMove(pt))
  {
    m_sbardir = m_sbar->GetDirection();
    offset = m_sbar->GetSBarOffset();
    if (offset)
    {
      if (offset > m_maxsbar)
        offset = m_maxsbar;
      m_anispeed = 0.00005f * pow((float)offset, 2.0f);
    }
  }
  else if (m_uiopenbtn->ActMouseOver(pt))
  {
    if (m_selblockunit)
    {
      POINT pt = {0, 0};
      m_selblockunit->ActMouseOut(pt);
    }
    MediaCenterController::GetInstance()->Render();
  }
  else
  {
    BlocksMouseMove(pt);
  }

  m_uiopenbtn->ActMouseOver(pt);

  return ret;
}

BOOL SPMCList::ActMouseLBDown(const POINT& pt)
{
  BOOL ret = FALSE;
  
  ret = m_sbar->ActMouseLBDown(pt);
  if (m_sbar->GetSBarOffset() && ret)
    ActMouseMove(pt);
  else if (ret)
    m_mousewheel = FALSE;

  return ret;
}

BOOL SPMCList::ActMouseLBUp(const POINT& pt)
{
  BOOL ret = FALSE;

  ret = m_sbar->ActMouseLBUp(pt);

  m_anispeed = 0.f;

  if (m_updaterowpos)
  {
    m_rowpos = m_updaterowpos;
    m_updaterowpos = 0;
  }

  if (!m_sbar->IsDragBar() && m_selblockunit)
  {
    m_selblockunit->ActLButtonUp(pt);
    ret = TRUE;
  }

  if (m_uiopenbtn->ActMouseLBUp(pt))
    ret = TRUE;

  return ret;
}

void SPMCList::InitMCList(int w, int h)
{
  m_rowpos = 0;
  SetMCRect(w, h);

  if (m_dbsource->PreLoad(m_blockcount))
  {
    m_listempty = FALSE;
    m_sbar->SetDisplay(m_dbsource->IsMoreData());
  }
  else
    SetCover();
}

void SPMCList::ReleaseList()
{
  m_dbsource->CleanData();
}

void SPMCList::SetMCRect(int w, int h)
{
  m_wndsize.cx = w;
  m_wndsize.cy = h;

  m_maxsbar = h / 2;
  m_maxoffset = m_blocktop+m_blockh;

  AlignColumns();
  AlignRows();

  m_blockcount = m_maxcolumns * m_maxrows;
}

BOOL SPMCList::ActWindowChange(int w, int h)
{
  int maskw, maskh, btnw, btnh;
  POINT maskpt, btnpos;

  m_lockpaint = TRUE;
  SetMCRect(w, h);

  m_ulmask->GetTextureWH(maskw, maskh);
  maskpt.x = 0;
  maskpt.y = h - maskh;
  m_ulmask->SetTexturePos(maskpt);
  m_ulmask->SetDisplayWH(w, 0);

  m_uiopenbtn->GetTextureWH(btnw, btnh);
  btnpos.x = w - btnw - m_sbar->GetBarWidth();
  btnpos.y = h + 15 - btnh;
  m_uiopenbtn->SetTexturePos(btnpos);

  if (!m_listempty)
    m_dbsource->AdjustRange(m_blockcount, m_maxcolumns);
  
  m_dbsource->SetReadNums(m_blockcount);

  m_sbar->SetDisplay(m_dbsource->IsMoreData());

  m_lockpaint = FALSE;

  return TRUE;
}

BOOL SPMCList::ActLButtonDblClick(const POINT& pt)
{
  BOOL bl = FALSE;

  if (m_selblockunit)
  {
    bl = TRUE;
    MediaCenterController::GetInstance()->ShowFilmNameEdit(m_selblockunit);
  }

  if (m_sbar->ActLButtonDblClick(pt))
    bl = TRUE;
  
  return bl;
}

BOOL SPMCList::ActRButtonUp(const POINT &pt)
{
  // if a blockunit deal with this message, then return TRUE to avoid popup menu
  BOOL bl = FALSE;

  if (m_selblockunit)
  {
    bl = TRUE;
    m_selblockunit->ActRButtonUp(pt);
  }

  if (m_sbar->ActRButtonUp(pt))
    bl = TRUE;

  return bl;
}

void SPMCList::AlignColumns()
{
  int barwidth = GetScrollBar()->GetBarWidth();

  int maxwidth = m_wndsize.cx - barwidth - (m_wndmargin.left + m_wndmargin.right) + m_blockspacing;
  int zone = m_blockw + m_blockspacing;

  int remains = maxwidth % zone;
  int spacing = 0;

  m_maxcolumns = maxwidth / zone;
  
  if (remains && m_maxcolumns > 1)
    spacing = remains / (m_maxcolumns - 1) + m_blockspacing;

  m_columnpos = m_wndmargin.left;
  m_fixcolumnwidth = m_blockw + spacing;
}

void SPMCList::AlignRows()
{
  m_fixrowheight = m_blockh + m_blocktop;
  int maxheight = m_wndsize.cy + m_fixrowheight;
  
  m_maxrows = (maxheight-m_blocktop) / m_fixrowheight + 1;

  m_rowpos = (!m_rowpos) ? m_blocktop : m_rowpos;
}

void SPMCList::SetCover()
{
  if (!m_cover.IsNull())
    return;

  ResLoader resLoader;
  HBITMAP cover = resLoader.LoadBitmap(L"skin\\mccover.png");

  if (cover)
  {
    CBitmap tmpcover;
    tmpcover.Attach(cover);
    if (CSUIButton::PreMultiplyBitmap(tmpcover, m_coversize, TRUE))
      m_cover = (HBITMAP)tmpcover.Detach();
    else
      tmpcover.Detach();
  }
}