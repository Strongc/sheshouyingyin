
#pragma once

#include "stdafx.h"

#include <atlbase.h>
#define NEWUI_BTN_WIDTH 21
#define NEWUI_BTN_HEIGTH 17
#define NEWUI_BTN_MARGIN_TOP 7
#define NEWUI_BTN_MARGIN_RIGHT 15
#define NEWUI_COLOR_BG RGB(214,214,214)
#define NEWUI_COLOR_PEN  RGB(0x7f,0x7f,0x7f)
#define NEWUI_COLOR_PEN_BRIGHT RGB(0xe9,0xe9,0xe9)
#define NEWUI_COLOR_PEN_DARK RGB(154,154,154)
#define NEWUI_COLOR_TOOLBAR_UPPERBG  RGB(0x17,0x17,0x17)
#define NEWUI_COLOR_SEEKBAR_PLAYED RGB(0x54,0x54,0x54)

class CMemoryDC : public CDC
{
public:
	// Data members
	CDC* m_hDCOriginal;
	RECT m_rcPaint;
	CBitmap m_bmp;
	HBITMAP m_hBmpOld;

	// Constructor/destructor
	CMemoryDC(CDC* hDC, RECT& rcPaint) : m_hDCOriginal(hDC), m_hBmpOld(NULL)
	{
		m_rcPaint = rcPaint;
		CreateCompatibleDC(m_hDCOriginal);
		ATLASSERT(m_hDC != NULL);
		m_bmp.CreateCompatibleBitmap( m_hDCOriginal, m_rcPaint.right - m_rcPaint.left, m_rcPaint.bottom - m_rcPaint.top);
		ATLASSERT( (HBITMAP)m_bmp != NULL);
		m_hBmpOld = (HBITMAP)SelectObject(m_bmp);
		SetViewportOrg(-m_rcPaint.left, -m_rcPaint.top);
	}

	~CMemoryDC()
	{
		m_hDCOriginal->BitBlt( m_rcPaint.left, m_rcPaint.top, m_rcPaint.right - m_rcPaint.left, m_rcPaint.bottom - m_rcPaint.top, CDC::FromHandle(m_hDC), m_rcPaint.left, m_rcPaint.top, SRCCOPY);
		SelectObject(m_hBmpOld);
		DeleteDC();
	}
};

class CBtnAlign{
public:
	CBtnAlign(UINT ia, INT_PTR pb, CRect rmToBtn){
		iAlign = ia;
		bBtn = pb;
		marginToBtn = rmToBtn;
	}
	UINT iAlign;
	INT_PTR bBtn;
	CRect marginToBtn; //�������һ����ť��λ��
};

#define ALIGN_TOPLEFT 3
#define ALIGN_TOPRIGHT 5
#define ALIGN_BOTTOMLEFT 6
#define ALIGN_BOTTOMRIGHT 7

#define ALIGN_TOP 1
#define ALIGN_LEFT 2
#define ALIGN_RIGHT 4
#define ALIGN_BOTTOM 8

#define DEFAULT_MARGIN 3

#define DEFAULT_MARGIN_TOBUTTON CRect(3,3,3,3)

#include "libpng.h"

static int nLogDPIX = 0,  nLogDPIY = 0;



class CSUIButton {

public:
	CSize m_btnSize; //��ť�Ĵ�С�ߴ�
	CSize m_orgbtnSize; //��ť�Ĵ�С�ߴ�
	int m_iAlign; //�봰�ڶ��뷽��
	CRect m_marginTownd; //��ť����ڴ��ڵ�λ�ã� 0Ϊ��ǿ�� ����Ϊ�ٷֱ�
	CRect m_rcHitest; //��ť����Ч��Χ������hittest
	CBitmap m_bitmap; //ͼƬ�洢λ��
	int m_stat; //0 normal ; 1 hove ; 2 clicked ; 3 disabled
	UINT m_htMsgID;
	BOOL m_hide;
  BOOL m_currenthide;
  int  m_hidewidth;
	BOOL m_NotButton;
  CString m_buttonname;

	static HBITMAP SUILoadImage(LPCTSTR szBmpName);
	static void PreMultiplyBitmap( CBitmap& bmp , CSize& sizeBmp, BOOL NotButton);

	CString m_szBmpName;

	CList<CBtnAlign*> btnAlignList;

	CSUIButton(LPCTSTR szBmpName,  int iAlign, CRect marginTownd 
		, BOOL bNotButton = false, UINT htMsgID = NULL , BOOL bHide = FALSE
		,UINT alignToButton = 0 , CSUIButton * relativeToButton = 0, CRect marginToBtn = DEFAULT_MARGIN_TOBUTTON
    ,int  hidewidth = 0, CString buttonname = L"");
	CSUIButton(UINT Imgid,  int iAlign, CRect marginTownd 
		, BOOL bNotButton = false, UINT htMsgID = NULL , BOOL bHide = FALSE 
		,UINT alignToButton = 0 , CSUIButton * relativeToButton = 0 , CRect marginToBtn = DEFAULT_MARGIN_TOBUTTON
    ,int  hidewidth = 0, CString buttonname = L"");

	CPngImage m_png;

	
	void LoadImage(LPCTSTR szBmpName);
	void Attach(HBITMAP bmp);

	void addAlignRelButton(UINT alignToButton  , CSUIButton * relativeToButton , CRect rmToBtn = DEFAULT_MARGIN_TOBUTTON );

	void OnSize(CRect WndRect);

	void OnPaint(CMemoryDC *hDC, CRect rc);
	
	int OnHitTest(CPoint pt , int bLBtnDown);

	void CountDPI();	

  void SetCurrentHideState(long iWidth,double skinsRate,int m_nLogDPIY);

  void SetString(CString str);

  void SetStrSize(CSize sz);

private:
	LONG CalcRealMargin(LONG Mlen, LONG bW, LONG wW);
	
	int m_lastBtnDownStat;

  CString m_playtimestr;

};



class CSUIBtnList : public CList<CSUIButton*>
{

public:
	CSUIBtnList();
	virtual ~CSUIBtnList();
	BOOL HTRedrawRequired ;

	void PaintAll(CMemoryDC *hDC, CRect rc);
	
	void OnSize(CRect WndRect);

    int GetMaxHeight();
	UINT OnHitTest(CPoint pt , CRect rc, int bLBtnDown = -1);

	void SetDisableStat(UINT iMsgID, BOOL bDisable);
	void SetClickedStat(UINT iMsgID, BOOL bClicked);
	CRect GetHTRect(UINT iMsgID);
	void SetHideStat(POSITION pos, BOOL bHide); //By Position
	void SetHideStat(UINT iMsgID, BOOL bHide); //By HT MSG ID
	void SetHideStat(LPCTSTR szBmpName, BOOL bHide); //By BitMapName

	void ClearStat();

  void SetCurrentHideState(long iWidth,double skinsRate,int m_nLogDPIY);

  CSUIButton* GetButton(CString s);

  int GetRelativeMinLength(CRect WndRect, CSUIButton* btn);
};

