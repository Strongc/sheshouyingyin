﻿// PlayerToolTopBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "PlayerToolTopBar.h"


// PlayerToolTopBar.cpp : implementation file
#include "MainFrm.h"
#include "../../svplib/svplib.h"
#include "ButtonManage.h"
#include "GUIConfigManage.h"
#include "ResLoader.h"

#define CONFIGBUTTON(btnname,bmp,fixalign,fixcrect,notbutton,id,hide,hidewidth,relativealign,pbuttonname,relativecrect) \
  m_btnList.AddTail(new CSUIButton(L#bmp,fixalign,fixcrect,notbutton,id,hide,relativealign,m_btnList.GetButton(L#pbuttonname), \
  relativecrect,hidewidth,L#btnname)); \


#define CONFIGADDALIGN(pbtnname,relativealign,pbuttonname,relativecrect) \
  m_btnList.GetButton(L#pbtnname)->addAlignRelButton(relativealign,m_btnList.GetButton(L#pbuttonname),relativecrect);

// CPlayerToolTopBar

IMPLEMENT_DYNAMIC(CPlayerToolTopBar, CWnd)

CPlayerToolTopBar::CPlayerToolTopBar():
m_hovering(0),
m_pbtnList(&m_btnList),
m_nHeight(20)
{
}

CPlayerToolTopBar::~CPlayerToolTopBar()
{
}


BEGIN_MESSAGE_MAP(CPlayerToolTopBar, CWnd)
	ON_WM_CREATE()
	ON_WM_MOVE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_NCPAINT()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnTtnNeedText)
	ON_WM_ENABLE()
	ON_WM_ACTIVATE()
	ON_WM_NCHITTEST()
	ON_WM_NCCALCSIZE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_MOUSELEAVE()
	ON_WM_SHOWWINDOW()
	ON_WM_NCACTIVATE()
	ON_WM_SETFOCUS()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_ACTIVATEAPP()
END_MESSAGE_MAP()



// CPlayerToolTopBar message handlers

BOOL CPlayerToolTopBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message){

	//CPoint pt;
	//::GetCursorPos (&pt);
	//ScreenToClient (&pt);

	if(m_nItemToTrack){	
		SetCursor(cursorHand );
		return TRUE;
	}
	return CWnd::OnSetCursor(pWnd, 0, 0);
}

BOOL CPlayerToolTopBar::OnTtnNeedText(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	//AfxMessageBox(_T("x")); //where is my tooltip!?!
	UNREFERENCED_PARAMETER(id);

	TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
	UINT_PTR nID = pNMHDR->idFrom;
	BOOL bRet = FALSE;

	if(m_nItemToTrack){
        if(iLeftBorderPos != -9990){
            if(m_nItemToTrack != ID_FILE_EXIT){
                *pResult = 0;
                return bRet;
            }
        }
		// idFrom is actually the HWND of the tool
		CString toolTip;
		m_lastTipItem = m_nItemToTrack;
		switch(m_nItemToTrack){
					case ID_VIEW_VF_FROMINSIDE:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_STANDARD_VIDEO_FRAME);
						break;
					case ID_VIEW_VF_FROMOUTSIDE:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_VIDEO_FRAME_REMOVE_BLACK_BAR);
						break;
					case ID_VIEW_FULLSCREEN:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_FULLSCREEN);
						break;
					case ID_VIEW_ZOOM_100:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_ZOOM_100);
						break;
					case ID_VIEW_ZOOM_200:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_ZOOM_200);
						break;
					case ID_MENU_AUDIO:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_AUDIO);
						break;
					case ID_MENU_VIDEO:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_VIDEO);
						break;
					case ID_ONTOP_ALWAYS:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_PIN_ONTOP);
						break;
					case ID_ONTOP_NEVER:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_RESTORE_WINDOW);
						break;
					case ID_ROTATE_90:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_ROTATE_90);
						break;
					case ID_ROTATE_V:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_FLIP_V);
						break;
					case ID_FILE_SAVE_IMAGE:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_IMAGE_CAPTURE);
						break;
					case ID_SHOWTRANSPRANTBAR:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_TRANSPARENT_CONTROL_BAR);
						break;
					case ID_SHOWCOLORCONTROLBAR:
						toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_BRIGHT_CONTROL);
						break;
          case ID_PLAYBACK_LOOP_PLAYLIST:
            toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_NOCYCLE_CONTROL);
            break;
          case ID_PLAYBACK_LOOP_RANDOM:
            toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_SINGLECYCLE_CONTROL);
            break;
          case ID_PLAYBACK_LOOP_CURRENT:
            toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_ALLCYCLE_CONTROL);
            break;
          case ID_PLAYBACK_LOOP_NORMAL:
            toolTip = ResStr(IDS_TOOLTIP_TOPTOOLBAR_BUTTON_RANDOM_CONTROL);
					default:
						toolTip = ResStr(m_nItemToTrack);
						break;
		}
		//AfxMessageBox(toolTip);
		//if(toolTip.IsEmpty())
		//	toolTip = _T("Unkown");
		
			if(AfxGetMyApp()->IsVista() ){
				if(!toolTip.IsEmpty()){
					pTTT->lpszText = toolTip.GetBuffer();
					pTTT->hinst = AfxGetResourceHandle();
					bRet = TRUE;
				}
			}else{
				
				CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
				pFrame->m_tip.SetTips(toolTip, TRUE);
			}
		
	}else{
		CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
		pFrame->m_tip.SetTips(ResStr(nID), TRUE);
	}


	*pResult = 0;

	return bRet;
}
int CPlayerToolTopBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	cursorHand = ::LoadCursor(NULL, IDC_HAND);
	cursorArrow = ::LoadCursor(NULL, IDC_ARROW);

	GetSystemFontWithScale(&m_statft, 14.0);

	BOOL bExtenedBtn = FALSE;

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	m_nLogDPIY = pFrame->m_nLogDPIY;

  AppSettings& s = AfxGetAppSettings();
  GUIConfigManage cfgfile;
  ButtonManage  cfgbtn;
  std::wstring cfgfilepath(L"skins\\");
  cfgfilepath += s.skinname;
  cfgfilepath += L"\\TopToolBarButton.dat";

  //cfgfile.SetCfgFilePath(L"skins\\TopToolBarButton.dat");
  cfgfile.SetCfgFilePath(cfgfilepath);
  cfgfile.ReadFromFile();
  if (cfgfile.IsFileExist())
  {
    cfgbtn.SetParse(cfgfile.GetCfgString(), &m_btnList);
    cfgbtn.ParseConfig(FALSE);
  }
  else
    DefaultButtonManage();
  
  PointCloseBtn();

  m_toolTip.Create(this);
	m_ti.cbSize = sizeof(m_ti);
	m_ti.uFlags = 0  ;//TTF_TRACK|TTF_ABSOLUTE
	m_ti.hwnd = m_hWnd;
	m_ti.hinst = NULL;
	m_ti.uId = (UINT)0;
	m_ti.lpszText = LPSTR_TEXTCALLBACK;
	m_ti.rect.left = 0;    
	m_ti.rect.top = 0;
	m_ti.rect.right = 0;
	m_ti.rect.bottom = 0;

	m_toolTip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&m_ti);

  m_nHeight = max(20, m_btnList.GetMaxHeight());
  if(m_nHeight > 20){
        m_nHeight += 4;
  }

  return 0;
}
void CPlayerToolTopBar::ReCalcBtnPos(){
	UpdateButtonStat();
	CRect rc;
	GetWindowRect(&rc);
	m_btnList.OnSize( rc);
	POSITION pos = m_btnList.GetHeadPosition();
	while(pos){
		break;
	}
	
}
void CPlayerToolTopBar::UpdateButtonStat(){
	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	AppSettings& s = AfxGetAppSettings();
	BOOL ontop = !!s.iOnTop ;
	BOOL fullscreen = pFrame->m_fFullScreen;
	BOOL bCaptionHidden = pFrame->IsCaptionMenuHidden();
	
  CRect rc;
  GetWindowRect(&rc);
  long iWidth = rc.Width();
  CMainFrame* pfrm = ((CMainFrame*)AfxGetMainWnd());
  double skinsRate = (double)pfrm->m_lMinFrameWidth / 310;
  m_btnList.SetCurrentHideState(rc.Width(),skinsRate,m_nLogDPIY);

  m_btnList.SetHideStat( ID_ONTOP_NEVER , !ontop );
	m_btnList.SetHideStat( ID_ONTOP_ALWAYS , ontop );

	m_btnList.SetHideStat( ID_FILE_EXIT , !fullscreen && !bCaptionHidden );
	//
	m_btnList.SetHideStat( L"TOP_RESTORE.BMP" , !fullscreen );

	BOOL bViewFROMOUTSIDE = (s.iDefaultVideoSize == 5);


	m_btnList.SetHideStat( L"TOP_LETTERBOX_WIDER.BMP" , pFrame->m_fScreenHigherThanVideo || bViewFROMOUTSIDE);
	m_btnList.SetHideStat( L"TOP_LETTERBOX.BMP" , !pFrame->m_fScreenHigherThanVideo || bViewFROMOUTSIDE);
	m_btnList.SetHideStat( L"TOP_NORMAL_WIDER.BMP" , pFrame->m_fScreenHigherThanVideo || !bViewFROMOUTSIDE);
	m_btnList.SetHideStat( L"TOP_NORMAL.BMP" , !pFrame->m_fScreenHigherThanVideo || !bViewFROMOUTSIDE);


	m_btnList.SetHideStat( L"TOP_NORMAL_WIDER.BMP" , pFrame->m_fScreenHigherThanVideo || bViewFROMOUTSIDE);
	m_btnList.SetHideStat( L"TOP_NORMAL.BMP" , !pFrame->m_fScreenHigherThanVideo || bViewFROMOUTSIDE);
	m_btnList.SetHideStat( L"TOP_LETTERBOX_WIDER.BMP" , pFrame->m_fScreenHigherThanVideo || !bViewFROMOUTSIDE);
	m_btnList.SetHideStat( L"TOP_NORMAL.BMP" , !pFrame->m_fScreenHigherThanVideo || !bViewFROMOUTSIDE);

	
	if(s.htpcmode){
		m_btnList.SetHideStat(L"TOP_RESTORE.BMP", 1);
		//m_btnList.SetHideStat( L"PINAIL.BMP" ,1 );
		//m_btnList.SetHideStat( L"PINAIL2.BMP" , 1);
		m_btnList.SetHideStat( L"TOP_FULLSCREEN.BMP" , 1);
	}else{
		m_btnList.SetHideStat( L"TOP_FULLSCREEN.BMP" , fullscreen  );
  }

  if(AfxGetMyApp()->IsVista() && s.bUserAeroUI())
    m_btnList.SetHideStat(L"TOP_TRANS.BMP", TRUE);


  if (pFrame->m_wndPlaylistBar.GetCount() == 0)
  {
    m_btnList.SetHideStat(L"TOP_ALLCYCLE.BMP", TRUE);
    m_btnList.SetHideStat(L"TOP_RANDOM.BMP", TRUE);
    m_btnList.SetHideStat(L"TOP_SINGLECYCLE.BMP", TRUE);
    m_btnList.SetHideStat(L"TOP_NOCYCLE.BMP", TRUE);
  }
  else if (pFrame->m_wndPlaylistBar.GetCount() == 1)
  {
    
    switch (pFrame->m_nLoopSetting)
    {
    case ID_PLAYBACK_LOOP_CURRENT:
      m_btnList.SetHideStat(L"TOP_ALLCYCLE.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_RANDOM.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_NOCYCLE.BMP", TRUE);
      break;
    case ID_PLAYBACK_LOOP_PLAYLIST:
      m_btnList.SetHideStat(L"TOP_RANDOM.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_SINGLECYCLE.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_NOCYCLE.BMP", TRUE);
      break;
    default:
      m_btnList.SetHideStat(L"TOP_SINGLECYCLE.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_RANDOM.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_ALLCYCLE.BMP", TRUE);
      ::SendMessage(pFrame->m_hWnd, WM_COMMAND, MAKEWPARAM(ID_PLAYBACK_LOOP_NORMAL, 0), 0);
      break;
    }
  }
  else if (pFrame->m_wndPlaylistBar.GetCount() > 1)
  {
    switch (pFrame->m_nLoopSetting)
    {
    case ID_PLAYBACK_LOOP_PLAYLIST:
      m_btnList.SetHideStat(L"TOP_RANDOM.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_SINGLECYCLE.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_NOCYCLE.BMP", TRUE);
      break;
    case ID_PLAYBACK_LOOP_RANDOM:
      m_btnList.SetHideStat(L"TOP_ALLCYCLE.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_SINGLECYCLE.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_NOCYCLE.BMP", TRUE);
      break;
    case ID_PLAYBACK_LOOP_CURRENT:
      m_btnList.SetHideStat(L"TOP_ALLCYCLE.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_RANDOM.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_NOCYCLE.BMP", TRUE);
      break;
    case ID_PLAYBACK_LOOP_NORMAL:
      m_btnList.SetHideStat(L"TOP_RANDOM.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_SINGLECYCLE.BMP", TRUE);
      m_btnList.SetHideStat(L"TOP_ALLCYCLE.BMP", TRUE);
      break;
    }
  }

}
void CPlayerToolTopBar::OnMove(int x, int y)
{
	__super::OnMove(x, y);

	ReCalcBtnPos();
	Invalidate();
}

void CPlayerToolTopBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	
	AppSettings& s = AfxGetAppSettings();
	
	CRect rcClient;
	GetClientRect(&rcClient);
	CMemoryDC hdc(&dc, rcClient);

	CRect rc;
	GetWindowRect(&rc);
	CRect rcLeft(rcClient);
	CRect rcBottomSqu = rcClient;
	rcBottomSqu.top = rcBottomSqu.bottom - 1;
	//hdc.FillSolidRect(rcBottomSqu, NEWUI_COLOR_BG);

	CRect rcUpperSqu = rcClient;
	rcUpperSqu.bottom--;
	//rcUpperSqu.right--;
  if (s.skinid == ID_SKIN_FIRST)
  {
	  hdc.FillSolidRect(rcUpperSqu, s.GetColorFromTheme(_T("TopToolBarBG"), RGB(61,65,69) ));
    hdc.FillSolidRect(rcBottomSqu,s.GetColorFromTheme(_T("TopToolBarBorder"), RGB(89,89,89)));
  }
  else
  {

    CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
    CBitmap* cbm = CBitmap::FromHandle(pFrame->m_ttoolbarbg);
    CDC bmpDc;
    bmpDc.CreateCompatibleDC(&hdc);
    HBITMAP oldhbm = (HBITMAP)bmpDc.SelectObject(cbm);
    BITMAP btmp;
    cbm->GetBitmap(&btmp);
    hdc.StretchBlt(0, 0, rcUpperSqu.Width(), rcUpperSqu.Height(), &bmpDc, 0, 0, btmp.bmWidth, btmp.bmHeight, SRCCOPY);
    bmpDc.SelectObject(oldhbm);
  }

	if(iLeftBorderPos >= 0)
	{
		//m_rgn.
		//SVP_LogMsg5(L"hah2a %d %d", rcClient.left , iLeftBorderPos);
		rcLeft.left = iLeftBorderPos;
		rcLeft.right = rcLeft.left+1;
		hdc.FillSolidRect(rcLeft,s.GetColorFromTheme(_T("TopToolBarBorder"), RGB(89,89,89)));
		/*CBrush brush;
		brush.CreateSolidBrush(s.GetColorFromTheme(_T("TopToolBarBorder"), RGB(89,89,89)));
		HBRUSH holdbrush = (HBRUSH)hdc.SelectObject(brush);
		hdc.FrameRgn(&m_rgn ,&brush , -1,-1);
		hdc.SelectObject(holdbrush);*/
	}
	//rcBottomSqu = rcClient;
	//rcBottomSqu.left = rcBottomSqu.right - 1;
	//hdc.FillSolidRect(rcBottomSqu, RGB(89,89,89));
	UpdateButtonStat();
	m_btnList.PaintAll(&hdc, rc);
	
}
void CPlayerToolTopBar::OnResizeRgn(){
	CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());


	 if(pFrame && ((pFrame->IsSomethingLoaded() && !pFrame->m_fAudioOnly) || !pFrame->IsSomethingLoaded() || !pFrame->IsCaptionMenuHidden()))
	 {
		 iLeftBorderPos = -9990;
		SetWindowRgn(NULL,TRUE);  
	 }
	else{
		CRect rc;
		WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
		GetWindowPlacement(&wp);
		GetWindowRect(&rc);
		CRect rc2(rc);
		rc-=rc.TopLeft();
		if((HRGN)m_rgn)
		{
			m_rgn.DeleteObject();
		}
		// create rounded rect region based on new window size
		{
			//rc.InflateRect(GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CYBORDER));
			//int l_size_of_corner = s.GetColorFromTheme(_T("WinFrameSizeOfCorner"), 3);
			//m_rgn.CreateRoundRectRgn(0,0,rc.Width()-1,rc.Height()-1, 3,3);                 // rounded rect w/50 pixel corners

			//btnClose->m_rcHitest.left
			
			//
			//SVP_LogMsg5(L"haha %d %d", rc.Width()-20 , );
			iLeftBorderPos =  btnClose->m_rcHitest.left-rc2.left-2 ;
			//CRgn rgn_round, rgn_rect;
			//m_rgn.CreateRectRgn(iLeftBorderPos,0, rc.Width(), rc.Height() );
			m_rgn.CreateRoundRectRgn(iLeftBorderPos,0, rc.Width()+3, rc.Height()+1 ,3,3);
			//rgn_round.CreateRoundRectRgn(iLeftBorderPos,0,rc.Width(),rc.Height()-1, 3,3); 

			//m_rgn.CombineRgn( &rgn_rect, &rgn_round, RGN_AND);
			// set window region to make rounded window
		}
		SetWindowRgn(m_rgn,TRUE); 
	}
}
void CPlayerToolTopBar::OnSize(UINT nType, int cx, int cy)
{
	
	__super::OnSize(nType, cx, cy);

	ReCalcBtnPos();
	OnResizeRgn();
	Invalidate();
}


BOOL CPlayerToolTopBar::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return __super::OnEraseBkgnd(pDC);
}

void CPlayerToolTopBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
	
	ReleaseCapture();
  
	CRect rc;
	GetWindowRect(&rc);

	CPoint xpoint = point + rc.TopLeft() ;
	UINT ret = m_btnList.OnHitTest(xpoint,rc,false);
	if( m_btnList.HTRedrawRequired ){
		if(ret)
			pFrame->PostMessage( WM_COMMAND, ret);
    m_toolTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
		
		Invalidate();
	}
	m_nItemToTrack = ret;

	KillTimer(IDT_CLOSE);

  
	//SetTimer(IDT_CLOSE, 8000 , NULL);


	//	__super::OnLButtonUp(nFlags, point);
	
	return;
	//__super::OnLButtonUp(nFlags, point);
}

void CPlayerToolTopBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetCapture();
	CRect rc;
	GetWindowRect(&rc);
  point += rc.TopLeft() ;
	UINT ret = m_btnList.OnHitTest(point,rc,true);
	if ( m_btnList.HTRedrawRequired ){
// 		if (ret)
// 		SetCapture();
		Invalidate();
	}
	m_nItemToTrack = ret;

	KillTimer(IDT_CLOSE);

  
	//SetTimer(IDT_CLOSE, 8000 , NULL);

	//__super::OnLButtonDown(nFlags, point);
}

void CPlayerToolTopBar::OnNcPaint()
{
	// TODO: Add your message handler code here
	// Do not call CToolBar::OnNcPaint() for painting messages
}

void CPlayerToolTopBar::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(IDT_TIPS == nIDEvent){
		KillTimer(IDT_TIPS);
		if(m_nItemToTrack){
			CPoint pt;
			if(m_lastTipItem != m_nItemToTrack) {
				
				GetCursorPos(&pt);
				//ClientToScreen(&pt);
				pt.y+=10;
				//m_ti.uId = ret;

				m_toolTip.SendMessage(TTM_TRACKPOSITION, 0, (LPARAM)MAKELPARAM(pt.x, pt.y));
				m_toolTip.SendMessage(TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ti);
			}
		}
	}else if(IDT_CLOSE == nIDEvent){
		KillTimer(IDT_CLOSE);
		ShowWindow(SW_HIDE);
	}
	__super::OnTimer(nIDEvent);
}
static CPoint m_lastMouseMove;
void CPlayerToolTopBar::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CSize diff = m_lastMouseMove - point;
	CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
	
	BOOL bMouseMoved =  diff.cx || diff.cy ;
	if(bMouseMoved){
		KillTimer(IDT_CLOSE);
		pFrame->KillTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER);
       // SVP_LogMsg5(L" IsSomethingLoaded %d %d ", pFrame->IsSomethingLoaded(), __LINE__);
		if(pFrame->IsSomethingLoaded()){
			pFrame->SetTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER, 5000, NULL); 
		}
		
		//SetTimer(IDT_CLOSE, 8000 , NULL);
		m_lastMouseMove = point;
	}

	CRect rc;
	//CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	GetWindowRect(&rc);
	point += rc.TopLeft() ;
	

	if( bMouseMoved){

		UINT ret = m_btnList.OnHitTest(point,rc, -1);
		m_nItemToTrack = ret;
		if(ret){
			if( GetCursor() == NULL )
				SetCursor(cursorHand);

			SetTimer(IDT_TIPS, 100 , NULL);
			
		}else{
			m_toolTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
			pFrame->m_tip.SetTips(_T(""));
			if( GetCursor() == NULL ) 
				SetCursor(cursorArrow);
		}
		if( m_btnList.HTRedrawRequired ){
			Invalidate();
		}
	}
 
  __super::OnMouseMove(nFlags, point);
}

INT_PTR CPlayerToolTopBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	// TODO: Add your specialized code here and/or call the base class
		
	if(!pTI){
		return -1;
	}

	
	UINT ret = m_nItemToTrack;
	
	if(ret){

		
		pTI->hwnd = AfxGetMainWnd()->m_hWnd;
		pTI->uId = (UINT) (ret);
		//pTI->uFlags = TTF_SUBCLASS TTF_IDISHWND;
		pTI->lpszText = LPSTR_TEXTCALLBACK;
		RECT rcClient;
		GetClientRect(&rcClient);
		//SVP_LogMsg3("Tooltip %d" , ret);
		pTI->rect = rcClient;

		
		return pTI->uId;

	}
	return -1;
	
}

void CPlayerToolTopBar::OnEnable(BOOL bEnable)
{
	//CWnd::OnEnable(TRUE);

	return;
	// TODO: Add your message handler code here
}

BOOL CPlayerToolTopBar::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: Add your specialized code here and/or call the base class
	
	return __super::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

}

BOOL CPlayerToolTopBar::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
    if( iLeftBorderPos != -9990 && pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MYMOUSELAST && pMsg->hwnd == this->m_hWnd)
    {
//         CWnd* pParent = GetParent();
         CPoint p(pMsg->lParam);
//         CRect rc2;
//         GetWindowRect(&rc2);
//         SVP_LogMsg5(L"p %d %d %d",p.x , rc2.left, iLeftBorderPos);
        if(p.x < iLeftBorderPos){
//            ::MapWindowPoints(pMsg->hwnd, pParent->m_hWnd, &p, 1);
            //pParent->PostMessage(pMsg->message, pMsg->wParam, MAKELPARAM(p.x, p.y));
            return TRUE;
        }
    }
	
	return CWnd::PreTranslateMessage(pMsg);
}

LRESULT CPlayerToolTopBar::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	return CWnd::OnNcHitTest(point);
}

void CPlayerToolTopBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{

	__super::OnNcCalcSize(bCalcValidRects, lpncsp);
	
}

void CPlayerToolTopBar::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	//CWnd::OnClose();
}

void CPlayerToolTopBar::OnDestroy()
{
	CWnd::OnDestroy();

	// TODO: Add your message handler code here
}
void CPlayerToolTopBar::OnRealClose(){
	__super::OnClose();
}
void CPlayerToolTopBar::OnMouseLeave()
{
	// TODO: Add your message handler code here and/or call default
	m_toolTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
	m_nItemToTrack = 0 ;

  CWnd::OnMouseLeave();
}

void CPlayerToolTopBar::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if(bShow != SW_SHOW){
		m_toolTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
		m_nItemToTrack = 0 ;
	}
	
	CWnd::OnShowWindow(bShow, nStatus);

}

void CPlayerToolTopBar::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{

//
	CWnd::OnActivate(WA_INACTIVE, pWndOther, bMinimized); //


}

BOOL CPlayerToolTopBar::OnNcActivate(BOOL bActive)
{
	// TODO: Add your message handler code here and/or call default
	//CWnd::
	bActive = false;
	return __super::OnNcActivate(bActive);;
}

void CPlayerToolTopBar::OnSetFocus(CWnd* pOldWnd)
{
	
	__super::OnSetFocus(pOldWnd);

	//AfxGetMainWnd()->SendMessage(WM_SETFOCUS, (WPARAM )m_hWnd, NULL);

	//ModifyStyleEx(0, WS_EX_NOACTIVATE);
	//::SetForegroundWindow( AfxGetMainWnd()->m_hWnd );

	// TODO: Add your message handler code here
}

void CPlayerToolTopBar::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	//lpwndpos->flags |= SWP_NOACTIVATE;
	CWnd::OnWindowPosChanging(lpwndpos);

	// TODO: Add your message handler code here
}

void CPlayerToolTopBar::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	//bActive = false;
	CWnd::OnActivateApp(bActive, dwThreadID);

	// TODO: Add your message handler code here
}

void CPlayerToolTopBar::DefaultButtonManage()
{
  CONFIGBUTTON(CLOSE,TOP_CLOSE.BMP,ALIGN_TOPRIGHT,CRect(1,1,1,1),0,ID_FILE_EXIT,FALSE,0,0,0,CRect(0,0,0,0))

  CONFIGBUTTON(RESTORE,TOP_RESTORE.BMP,ALIGN_TOPRIGHT,CRect(1,1,1,1),0,ID_VIEW_FULLSCREEN,FALSE,0,ALIGN_RIGHT,CLOSE,CRect(1,1,1,1))

  CONFIGBUTTON(PINAIL,PINAIL.BMP,ALIGN_TOPRIGHT,CRect(1,1,1,1),0,ID_ONTOP_ALWAYS,FALSE,0,ALIGN_RIGHT,CLOSE,CRect(1,1,1,1))
  CONFIGADDALIGN(PINAIL,ALIGN_RIGHT,RESTORE,CRect(1,1,1,1))

  CONFIGBUTTON(PINAIL2,PINAIL2.BMP,ALIGN_TOPRIGHT,CRect(1,1,1,1),0,ID_ONTOP_NEVER,FALSE,0,ALIGN_RIGHT,CLOSE,CRect(1,1,1,1))
  CONFIGADDALIGN(PINAIL2,ALIGN_RIGHT,RESTORE,CRect(1,1,1,1))

  CONFIGBUTTON(FULLSCREEN,TOP_FULLSCREEN.BMP,ALIGN_TOPRIGHT,CRect(1,1,1,1),0,ID_VIEW_FULLSCREEN,FALSE,0,ALIGN_RIGHT,PINAIL,CRect(1,1,1,1))
  CONFIGADDALIGN(FULLSCREEN,ALIGN_RIGHT,PINAIL2,CRect(1,1,1,1))

  AppSettings& s = AfxGetAppSettings();
/*	CSUIButton* btnFlip = NULL;
	if(bExtenedBtn){
		CSUIButton* btnRotate  = new CSUIButton(L"TOP_ROTATE.BMP" , ALIGN_TOPRIGHT, CRect(1 , 1, 1,1)  , 0, ID_ROTATE_90, FALSE, ALIGN_RIGHT, btnFull  , CRect(1,1,1,1),0,L"ROTATE") ;
		btnRotate->addAlignRelButton( ALIGN_RIGHT, btnRestore  , CRect(1,1,1,1)  );
		btnRotate->addAlignRelButton( ALIGN_RIGHT, btnPin1  , CRect(1,1,1,1)  );
		btnRotate->addAlignRelButton( ALIGN_RIGHT, btnPin2  , CRect(1,1,1,1)  );
		m_btnList.AddTail(btnRotate);
	}
	if(0){//s.iSVPRenderType == 0
		
		btnFlip = new CSUIButton(L"TOP_FLIP.BMP" , ALIGN_TOPRIGHT, CRect(1 , 1, 1,1)  , 0, ID_ROTATE_V, FALSE, ALIGN_RIGHT,btnFull , CRect(1,1,1,1),0,L"FLIP");
		btnFlip->addAlignRelButton( ALIGN_RIGHT, btnRestore  , CRect(1,1,1,1)  );
		btnFlip->addAlignRelButton( ALIGN_RIGHT, btnPin1  , CRect(1,1,1,1)  );
		btnFlip->addAlignRelButton( ALIGN_RIGHT, btnPin2  , CRect(1,1,1,1)  );
		m_btnList.AddTail( btnFlip  );

	}*/

  CONFIGBUTTON(CAPTURE,TOP_CAPTURE.BMP,ALIGN_TOPRIGHT,CRect(1,1,1,1),0,ID_FILE_SAVE_IMAGE,FALSE,0,ALIGN_RIGHT,FULLSCREEN,CRect(1,1,1,1))
  CONFIGADDALIGN(CAPTURE,ALIGN_RIGHT,RESTORE,CRect(1,1,1,1))
  CONFIGADDALIGN(CAPTURE,ALIGN_RIGHT,PINAIL,CRect(1,1,1,1))
  CONFIGADDALIGN(CAPTURE,ALIGN_RIGHT,PINAIL2,CRect(1,1,1,1))

  if(AfxGetMyApp()->IsVista() && s.bUserAeroUI())
    CONFIGBUTTON(TRANS,TOP_TRANS.BMP,ALIGN_TOPRIGHT,CRect(1,1,1,1),0,ID_SHOWTRANSPRANTBAR,FALSE,0,ALIGN_RIGHT,CAPTURE,CRect(1,1,1,1))

  CONFIGBUTTON(GAMMA,TOP_GAMMA.BMP,ALIGN_TOPRIGHT,CRect(1,1,1,1),0,ID_SHOWCOLORCONTROLBAR,FALSE,0,ALIGN_RIGHT,CAPTURE,CRect(1,1,1,1))
  CONFIGADDALIGN(GAMMA,ALIGN_RIGHT,TRANS,CRect(1,1,1,1))

  CONFIGBUTTON(ALLCYCLE, TOP_ALLCYCLE.BMP, ALIGN_TOPRIGHT, CRect(1,1,1,1), 0, ID_PLAYBACK_LOOP_CURRENT, FALSE, 0, ALIGN_RIGHT, GAMMA, CRect(1,1,1,1))
  CONFIGBUTTON(RANDOM, TOP_RANDOM.BMP, ALIGN_TOPRIGHT, CRect(1,1,1,1), 0, ID_PLAYBACK_LOOP_NORMAL, FALSE, 0, ALIGN_RIGHT, GAMMA, CRect(1,1,1,1))
  CONFIGBUTTON(SINGLECYCLE, TOP_SINGLECYCLE.BMP, ALIGN_TOPRIGHT, CRect(1,1,1,1), 0, ID_PLAYBACK_LOOP_RANDOM, FALSE, 0, ALIGN_RIGHT, GAMMA, CRect(1,1,1,1))
  CONFIGBUTTON(NOCYCLE, TOP_NOCYCLE.BMP, ALIGN_TOPRIGHT, CRect(1,1,1,1), 0, ID_PLAYBACK_LOOP_PLAYLIST, FALSE, 0, ALIGN_RIGHT, GAMMA, CRect(1,1,1,1))

	
/*
#define ID_VIEW_VF_STRETCH              838
#define ID_VIEW_VF_FROMINSIDE           839
#define ID_VIEW_VF_FROMOUTSIDE          840
#define ID_VIEW_VF_KEEPASPECTRATIO      841*/

  CONFIGBUTTON(TOP1X,TOP_1X.BMP,ALIGN_TOPLEFT,CRect(1,1,1,1),0,ID_VIEW_ZOOM_100,FALSE,0,0,0,CRect(0,0,0,0))
  
  CONFIGBUTTON(TOP2X,TOP_2X.BMP,ALIGN_TOPLEFT,CRect(1,1,1,1),0,ID_VIEW_ZOOM_200,FALSE,0,ALIGN_LEFT,TOP1X,CRect(1,1,1,1))

  CONFIGBUTTON(NORMAL,TOP_NORMAL.BMP,ALIGN_TOPLEFT,CRect(1,1,1,1),0,ID_VIEW_VF_FROMINSIDE,FALSE,0,ALIGN_LEFT,TOP2X,CRect(1,1,1,1))

  CONFIGBUTTON(NORMALWIDER,TOP_NORMAL_WIDER.BMP,ALIGN_TOPLEFT,CRect(1,1,1,1),0,ID_VIEW_VF_FROMINSIDE,TRUE,0,ALIGN_LEFT,TOP2X,CRect(1,1,1,1))

  CONFIGBUTTON(LETTERBOX,TOP_LETTERBOX.BMP,ALIGN_TOPLEFT,CRect(1,1,1,1),0,ID_VIEW_VF_FROMOUTSIDE,TRUE,0,ALIGN_LEFT,TOP2X,CRect(1,1,1,1))

  CONFIGBUTTON(LETTERBOXWIDER,TOP_LETTERBOX_WIDER.BMP,ALIGN_TOPLEFT,CRect(1,1,1,1),0,ID_VIEW_VF_FROMOUTSIDE,TRUE,0,ALIGN_LEFT,TOP2X,CRect(1,1,1,1))

  CONFIGBUTTON(AUDIO,TOP_AUDIO.BMP,ALIGN_TOPLEFT,CRect(1,1,1,1),0,ID_MENU_AUDIO,FALSE,0,ALIGN_LEFT,LETTERBOX,CRect(5,1,1,1))
  CONFIGADDALIGN(AUDIO,ALIGN_LEFT,LETTERBOXWIDER,CRect(5,1,1,1))
	CONFIGADDALIGN(AUDIO,ALIGN_LEFT,NORMALWIDER,CRect(5,1,1,1))
  CONFIGADDALIGN(AUDIO,ALIGN_LEFT,NORMAL,CRect(5,1,1,1))

  CONFIGBUTTON(VIDEO,TOP_VIDEO.BMP,ALIGN_TOPLEFT,CRect(1,1,1,1),0,ID_MENU_VIDEO,FALSE,0,ALIGN_LEFT,AUDIO,CRect(1,1,1,1))
}

void CPlayerToolTopBar::PointCloseBtn()
{
  btnClose = m_btnList.GetButton(L"CLOSE");
}

void CPlayerToolTopBar::ResizeToolbarHeight()
{
  m_nHeight = max(20, m_btnList.GetMaxHeight());
  if (m_nHeight > 20)
    m_nHeight += 2;
}