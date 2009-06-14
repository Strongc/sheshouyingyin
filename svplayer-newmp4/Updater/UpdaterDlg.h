// UpdaterDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CUpdaterDlg dialog
class CUpdaterDlg : public CDialog
{
// Construction
public:
	CUpdaterDlg(CWnd* pParent = NULL);	// standard constructor
	UINT verbose;
	BOOL bHide;

// Dialog Data
	enum { IDD = IDD_UPDATER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	cupdatenetlib cup;
	BOOL m_bGoodToGo;
	NOTIFYICONDATA tnid;

	LRESULT On_WM_NOTIFYICON(WPARAM wParam, LPARAM lParam);
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButton1();
	CStatic csCurFile;
	CStatic csTotalProgress;
	CProgressCtrl prg_curfile;
	CProgressCtrl prg_total;
	CEdit cs_stat;
	CLinkCtrl cslink;
	afx_msg void OnNMClickSyslink1(NMHDR *pNMHDR, LRESULT *pResult);
	CButton cb_stop;
	CButton cb_backgd;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};