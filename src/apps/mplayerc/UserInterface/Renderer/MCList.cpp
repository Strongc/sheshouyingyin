#include "stdafx.h"
#include "MCList.h"
#include <logging.h>
#include "../../Controller/MediaCenterController.h"
#include "../../SUIButton.h"
#include <ResLoader.h>
#include "../../MainFrm.h"
  
SPMCList::SPMCList():
  m_dbsource(NULL),
  m_lockpaint(FALSE),
  m_selblockunit(NULL),
  m_listempty(TRUE),
  m_deltatime(0),
  m_hOldAccel(0)
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
}

SPMCList::~SPMCList()
{
  delete m_dbsource;
  delete m_sbar;
}

SPScrollBar* SPMCList::GetScrollBar()
{
  return m_sbar;
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

    SetStretchBltMode(dc, HALFTONE);
    SetBrushOrgEx(dc, 0, 0, NULL);
    StretchBlt(dc, rcclient.left, rcclient.top, rcclient.right, rcclient.bottom,
               dcmem, 0, 0, m_coversize.cx, m_coversize.cy, SRCCOPY);

    dcmem.SelectBitmap(hold);
    dcmem.DeleteDC();
    return;
  }

  if (!m_lockpaint)
  {
    if (m_sbar->GetDisplay())
    {
      m_sbar->SetClientRect(rcclient);
      m_sbar->DoPaint(dc);
    }
//     std::vector<int>::iterator itx =  m_x.begin();
//     std::vector<int>::iterator ity =  m_y.begin();
//     std::vector<int>::iterator it;
//     POINT pt;
//     for (;ity != m_y.end(); ity++)
//     {
//       pt.y = *ity;
//       for (it = itx; it != m_x.end(); it++)
//       {
//         pt.x = *it;
//         MCLoopInit(m_dbsource);
//         MCLoopCurData()->DoPaint(dc, pt);
//         MCLoopNextData();
//       }
//     }
    std::vector<int>::iterator itx =  m_x.begin();
    std::vector<int>::iterator ity =  m_y.begin();

    MCLoopList(m_dbsource)
      if (itx == m_x.end())
      {
        itx = m_x.begin();
        ++ity;
      }

      POINT pt = {*itx, *ity};
      MCLoopOne()->DoPaint(dc, pt);
      ++itx;
    MCEndLoop()
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
    MCEndLoop()
  }
  else if (m_selblockunit)
  {
    if (m_selblockunit->ActMouseMove(pt))
    {
      if (m_selblockunit->ActMouseOver(pt))
        MediaCenterController::GetInstance()->Render();
    }
    else
    {
      if (m_selblockunit->ActMouseOut(pt))
        MediaCenterController::GetInstance()->Render();
      m_selblockunit = NULL;
    }
  }
}

void SPMCList::Update(DWORD deltatime)
{
  int offset = m_sbar->GetSBarOffset();
  m_deltatime += deltatime;
  if (!offset || !deltatime)
    return;

  int status = m_dbsource->GetReaderStatus();
  if (status == MCDBSource::MCDB_UNKNOW)
    return;

  CString log;
  float v = (offset) / (float)(m_wndsize.cy/2/5) * 0.1f;
  int distance = (int)(deltatime * v);

  if (!distance)
    return;

  m_lockpaint = TRUE;
  BOOL dir = m_sbar->GetDirection();
  if (dir) // move up
  {
    if (status == MCDBSource::MCDB_TOSTART)
    {
      int heady = m_y.front();
      if (heady == m_blocktop)
      {
        m_lockpaint = FALSE;
        return;
      }
      
      if (heady + distance > m_blocktop)
        distance = m_blocktop - heady;

      std::vector<int>::iterator it = m_y.begin();
      for (; it != m_y.end(); it++)
        *it += distance;

      m_lockpaint = FALSE;
      return;
    }

    if (m_y.back() >= m_wndsize.cy)
    {
      m_dbsource->RemoveRowDatas(dir, m_x.size());
      m_y.pop_back();
    }

    if (m_y.front() >= m_blocktop)
    {
      status = m_dbsource->LoadRowDatas(dir, m_x.size());
      if (status == MCDBSource::MCDB_MORE)
      {
        m_y.insert(m_y.begin(), m_y.front()-m_blocktop-m_blockh);
      }
      else
      {
        m_lockpaint = FALSE;
        return;
      }
    }

    std::vector<int>::iterator it = m_y.begin();
    for (; it != m_y.end(); it++)
      *it += distance;
  }
  else// move down
  {
    if (status == MCDBSource::MCDB_TOEND)
    {
      m_lockpaint = FALSE;
      return;
    }

    if (m_y.front()+m_blockh <= 0)
    {
      m_dbsource->RemoveRowDatas(dir, m_x.size());
      m_y.erase(m_y.begin());
    }

    int taily = m_y.back()+m_blockh;
    if (m_wndsize.cy-taily >= m_blocktop)
    {
      status = m_dbsource->LoadRowDatas(dir, m_x.size());
      if (status == MCDBSource::MCDB_MORE)
      {
        m_y.push_back(taily + m_blocktop);
      }
      else
      {
        m_lockpaint = FALSE;
        return;
      }
    }

    std::vector<int>::iterator it = m_y.begin();
    for (; it != m_y.end(); it++)
      *it -= distance;
  }
  m_lockpaint = FALSE;
}

BOOL SPMCList::ActMouseMove(const POINT& pt)
{
  BOOL ret = FALSE;
  
  if (m_sbar->ActMouseMove(pt))
  {
    ;
  }
  else
  {
    BlocksMouseMove(pt);
  }

  return ret;
}

BOOL SPMCList::ActMouseLBDown(const POINT& pt)
{
  BOOL ret = FALSE;

  // determine whether destroy the filmname editor
  CRect rcEditor;
  if (m_pFilmNameEdit)
  {
    m_pFilmNameEdit->GetClientRect(&rcEditor);
    if (!rcEditor.PtInRect(pt))
      HideFilmNameEditor();
  }
  
  ret = m_sbar->ActMouseLBDown(pt);

  return ret;
}

BOOL SPMCList::ActMouseLBUp(const POINT& pt)
{
  BOOL ret = FALSE;

  ret = m_sbar->ActMouseLBUp(pt);

  m_deltatime = 0;
  return ret;
}

void SPMCList::InitMCList(int w, int h)
{
  SetMCRect(w, h);
  if (m_dbsource->PreLoad(m_blockcount))
  {
    m_listempty = FALSE;
    m_sbar->SetDisplay(m_dbsource->IsMoreData());
  }
  else
    SetCover();
}

void SPMCList::InitTextEdit()
{
  if (!m_pFilmNameEdit)
  {
    // create new editor
    m_pFilmNameEdit.reset(new TextEdit);
    m_pFilmNameEdit->Create(WS_CHILD | ES_AUTOHSCROLL | ES_MULTILINE | ES_CENTER, CRect(0, 0, 0, 0)
                          , CWnd::FromHandle(MediaCenterController::GetInstance()->GetFrame()), 1111);

    static CFont *pFont = CFont::FromHandle(MediaCenterController::GetInstance()->GetFilmTextFont());
    m_pFilmNameEdit->SetFont(pFont);
  }
}

void SPMCList::OnSetFilmName()  // set filmname by the edit control
{
  if (m_pFilmNameEdit && (m_pFilmNameEdit->IsWindowVisible()))
  {
    // restore the accelerate table
    CMainFrame *pFrame = (CMainFrame *)(AfxGetMyApp()->GetMainWnd());
    pFrame->m_hAccelTable = m_hOldAccel;

    // set filmname
    CString sNewFilmName;
    m_pFilmNameEdit->GetWindowText(sNewFilmName);
    (*m_itCurEdit)->m_mediadata.filmname = sNewFilmName;

    // rename file
    std::wstring sPath = (*m_itCurEdit)->m_mediadata.path;
    std::wstring sOldFilename = (*m_itCurEdit)->m_mediadata.filename;
    std::wstring sExt;
    size_t nPos = sOldFilename.find_last_of('.');
    if (nPos != std::wstring::npos)
      sExt = sOldFilename.substr(nPos);

    boost::system::error_code err;
    boost::filesystem::rename(sPath + sOldFilename, sPath + (LPCTSTR)sNewFilmName + sExt, err);

    if (err == boost::system::errc::success)
    {
      // set new filename
      (*m_itCurEdit)->m_mediadata.filename = sNewFilmName;
    }

    // store info to database
    media_tree::model &tree_model = MediaCenterController::GetInstance()->GetMediaTree();
    tree_model.addFile((*m_itCurEdit)->m_mediadata);
  }
}

void SPMCList::HideFilmNameEditor()
{
  // modify the block's filmname
  OnSetFilmName();

  // hide the editor
  m_pFilmNameEdit->SetWindowText(L"");
  m_pFilmNameEdit->ShowWindow(SW_HIDE);
}

HWND SPMCList::GetFilmNameEdit()
{
  if (m_pFilmNameEdit)
    return m_pFilmNameEdit->GetSafeHwnd();
  else
    return 0;
}

void SPMCList::SetMCRect(int w, int h)
{
  m_wndsize.cx = w;
  m_wndsize.cy = h;

  AlignColumns();
  AlignRows();

  m_blockcount = m_x.size() * m_y.size();
}

BOOL SPMCList::ActWindowChange(int w, int h)
{
  // set filmname if the child view's size is changed
  // determine whether destroy the filmname editor
  if (m_pFilmNameEdit)
    HideFilmNameEditor();

  SetMCRect(w, h);

  m_lockpaint = TRUE;

  if (!m_listempty)
    m_dbsource->AdjustRange(m_blockcount);
  
  m_dbsource->SetReadNums(m_blockcount);

  m_sbar->SetDisplay(m_dbsource->IsMoreData());

  m_lockpaint = FALSE;

  return TRUE;
}

BOOL SPMCList::ActLButtonDblClick(const POINT& pt)
{
  std::vector<int>::iterator itx =  m_x.begin();
  std::vector<int>::iterator ity =  m_y.begin();

  MCLoopList(m_dbsource)
    CRect rcText = MCLoopOne()->GetTextRect();
    if (rcText.PtInRect(pt) && m_pFilmNameEdit)
    {
      m_pFilmNameEdit->MoveWindow(rcText);
      m_pFilmNameEdit->ShowWindow(SW_SHOW);

      CMainFrame *pFrame = (CMainFrame *)(AfxGetMyApp()->GetMainWnd());
      m_hOldAccel = pFrame->m_hAccelTable;
      pFrame->m_hAccelTable = 0;        // temp destroy the accelerate table

      std::wstring sFilmName = MCLoopOne()->m_mediadata.filmname;
      if (sFilmName.empty())
        sFilmName = MCLoopOne()->m_mediadata.filename;

      int pos = sFilmName.find_last_of('.');
      if (pos != std::wstring::npos)
        sFilmName = sFilmName.substr(0, pos);

      m_pFilmNameEdit->SetTextVCenter();

      m_pFilmNameEdit->SetWindowText(sFilmName.c_str());
      m_pFilmNameEdit->SetFocus();

      m_pFilmNameEdit->SetSel(0, -1); // only focus on filename, exclude the ext

      m_itCurEdit = sp;  // sp is defined in macro MCLoopList
    }
  MCEndLoop()

  return TRUE;
}

BOOL SPMCList::ActRButtonUp(const POINT &pt)
{
  BOOL bl = FALSE;

  if (m_pFilmNameEdit)
    HideFilmNameEditor();

  return bl;
}

void SPMCList::AlignColumns()
{
  int barwidth = GetScrollBar()->GetBarWidth();

  int w1 = m_wndsize.cx - barwidth - (m_wndmargin.left + m_wndmargin.right) + m_blockspacing;
  int w2 = m_blockw + m_blockspacing;
  int count = w1 / w2;

  int remainspacing = w1 - count * w2;

  int spacing = 0;
  if (remainspacing && count > 1)
    spacing = remainspacing / (count - 1) + m_blockspacing;

  m_x.clear();
  int x = m_wndmargin.left;

  while (count--)
  {
    m_x.push_back(x);
    x += m_blockw + spacing;
  }
}

void SPMCList::AlignRows()
{

  int y = m_y.empty() ? m_blocktop : m_y.front();
  int next = m_blockh + m_blocktop;
  int len = m_wndsize.cy + next;
  
  m_y.clear();

  while(y < len)
  {
    m_y.push_back(y);
    y += next;
  }
}

void SPMCList::SetCover()
{
  if (!m_cover.IsNull())
    return;

  ResLoader resLoader;
  HBITMAP cover = resLoader.LoadBitmap(L"skin\\mccover.jpg");

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