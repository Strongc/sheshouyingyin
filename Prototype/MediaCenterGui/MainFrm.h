// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#define TIMER_OFFSET 11
#define  TIMER_SLOWOFFSET 12

class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CUpdateUI<CMainFrame>,
  public CMessageFilter, public CIdleHandler
{
public:
  DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

  CMediaCenterView m_view;
  BlockListView m_blocklist;
  int m_offsetspeed;
  int m_preoffsetspeed;
  
  virtual BOOL PreTranslateMessage(MSG* pMsg)
  {
    if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
      return TRUE;
   
    return m_view.PreTranslateMessage(pMsg);
  }

  virtual BOOL OnIdle()
  {
    return FALSE;
  }

  BEGIN_UPDATE_UI_MAP(CMainFrame)
  END_UPDATE_UI_MAP()

  BEGIN_MSG_MAP(CMainFrame)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
    COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
    COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
    
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBg)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
    MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    
    CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
    CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
  END_MSG_MAP()

  // Handler prototypes (uncomment arguments if needed):
  //	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  //	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  //	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

  LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  {
    
    m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
    
    // register object for message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    m_blocklist.SetFrameHwnd(m_hWnd);
    m_blocklist.SetScrollSpeed(&m_offsetspeed);
    
    return 0;
  }

  LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
  {
    KillTimer(123);
    // unregister message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->RemoveMessageFilter(this);
    pLoop->RemoveIdleHandler(this);

    bHandled = FALSE;
    return 1;
  }

  LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  {
    PostMessage(WM_CLOSE);
    return 0;
  }

  LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  {
    // TODO: add code to initialize document
    BlockUnit* one = new BlockUnit;
    m_blocklist.AddBlock(one);
    RECT rc;
    GetClientRect(&rc);
    m_blocklist.Update(rc.right - rc.left, rc.bottom - rc.top);
    Invalidate(FALSE);
    return 0;
  }

  LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  {
    CAboutDlg dlg;
    dlg.DoModal();
    return 0;
  }

  LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  {
    CPaintDC dc(m_hWnd);

    //TODO: Add your drawing code here
    RECT client;
    GetClientRect(&client);
    WTL::CMemoryDC memdc(dc, client);
    HBRUSH hbrush = ::CreateSolidBrush(RGB(231, 231, 231));
    memdc.FillRect(&client, hbrush);
    
    m_blocklist.DoPaint(memdc);

    return 0;
  }

  LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
  {
    int w = LOWORD(lParam);
    int h = HIWORD(lParam);
    m_blocklist.Update(w, h);
    Invalidate(FALSE);
    return 0;
  }

  LRESULT OnEraseBg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
  {
    bHandled = TRUE;
    return 0;
  }

  LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
  {
    POINT pt;
    pt.x = LOWORD(lParam);
    pt.y = HIWORD(lParam);
    RECT rc;
    GetClientRect(&rc);
    m_blocklist.HandleLButtonDown(pt, rc);
    return 0;
  }

  LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
  {
    
    POINT pt;
    pt.x = LOWORD(lParam);
    pt.y = HIWORD(lParam);
    RECT rc;
    GetClientRect(&rc);
    m_blocklist.HandleLButtonUp(pt, rc);

    return 0;
  }

  LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
  {
    POINT pt;
    pt.x = LOWORD(lParam);
    pt.y = HIWORD(lParam);
    RECT rc;
    GetClientRect(&rc);
    m_blocklist.HandleMouseMove(pt, rc);
    return 0;
  }

  LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
  {
    switch (wParam)
    {
    case TIMER_OFFSET:
      m_preoffsetspeed = m_offsetspeed;
      m_blocklist.SetOffset(m_offsetspeed);
      break;
    case TIMER_SLOWOFFSET:
      m_blocklist.SetOffset(m_offsetspeed);
      if (m_preoffsetspeed == m_offsetspeed)
        KillTimer(TIMER_SLOWOFFSET);
      break;
    }
    
    RECT rc;
    GetClientRect(&rc);
    m_blocklist.Update(rc.right - rc.left, rc.bottom - rc.top);
    InvalidateRect(&rc);
    return 0;
  }
};

