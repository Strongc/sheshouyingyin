#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

#include "..\..\filters\switcher\AudioSwitcher\AudioSwitcher.h"
#include "..\..\filters\transform\mpadecfilter\MpaDecFilter.h"
// CUESettingPanel dialog

class CUESettingPanel : public CDHtmlDialog
{
	DECLARE_DYNAMIC(CUESettingPanel)

	CComQIPtr<IMpaDecFilter> m_pMDF;
	int m_outputformat;
	int m_ac3spkcfg;
	bool m_ac3drc;
	int m_dtsspkcfg;
	bool m_dtsdrc;
	bool m_aacdownmix;
	DolbyDigitalMode m_ddmode;

	virtual HRESULT STDMETHODCALLTYPE  GetHostInfo(DOCHOSTUIINFO *pInfo);
	virtual HRESULT STDMETHODCALLTYPE  ShowContextMenu(DWORD /*dwID*/, POINT *ppt, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/);
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	void setBackgdColorByID(CString szId, COLORREF color);
	void DisplayNodeByID(CString szId, BOOL bBlock);
	COLORREF getBackgdColorByID(CString szId);
	void ApplyAllSetting();
	void ApplyGeneralSetting();
	void ApplyVideoSetting();
	void ApplyAudioSetting();
	void ApplySubSetting();
	void ApplyFileAsscSetting();
	void ApplyHotkeySetting();
	int m_sgs_chkremhistory;
	int m_sgi_chkremwinpos;
	int m_sgi_chkcdromenu;
	int m_sgi_chkuseini;
	int m_sgi_chkfileass;
	int m_sgi_chkplayrepeat;
	int m_sgi_chkexitfullscreen;
	int m_sgi_chkabnormal;
	int m_sgi_noaudioboost;
	int m_sgi_chkautoplay;
	int m_sgi_chkautozoom;
	int m_sgi_chkautodownloadsvpsub;
	int m_sgi_chkautoresumeplay;

	int m_sgi_chkuseSmartDrag;
	int m_sgi_chkinternaltspliteronly;
	int m_sgi_UseWaveOutDeviceByDefault;

	CString m_sgs_speaker;

	CString m_sgs_initblock;
	CString m_sgi_startupcheckexts;
	CString m_sgs_videorender;
	CString m_sgs_decoder;
	
	CString  m_sgs_subfont1;
	CString  m_sgs_subfont2;
	CString m_sgs_subalign1;
	int m_sgi_suboveride1;
	CString m_sgs_subhpos1;
	CString m_sgs_subvpos1;
	CString m_sgs_engsubradio1;
	
	CString m_sgs_stepsmall;
	CString m_sgs_stepmed;
	CString m_sgs_stepbig;

	CString m_sgs_subalign2;
	int m_sgi_suboveride2;
	CString m_sgs_subhpos2;
	CString m_sgs_subvpos2;
	CString m_sgs_engsubradio2;

	long m_sgi_videorender;
	int m_sgi_lockbackbuff;
	int m_sgi_gpuacel;
	int m_sgi_uservmrmixer;
	int m_sgi_usespdif;

	int m_sgi_normalize;
	int m_sgi_downsample44k;
	long m_sgi_channelsetting;

	CString m_sgs_updateversion;
	int m_sgi_autoupdate;


	STSStyle m_stss;
	STSStyle m_stss2;
private:
	CComQIPtr<IAudioSwitcherFilter> m_pASF;
	CString GetIDfromElement(IHTMLElement *pElement);
public:
	CUESettingPanel(IFilterGraph* pFG, CWnd* pParentWnd, UINT idPagein);   // standard constructor
	virtual ~CUESettingPanel();
	
	HRESULT OnFileAss(IHTMLElement *pElement);
	HRESULT OnHotKey(IHTMLElement *pElement);
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);
	HRESULT OnButtonApply(IHTMLElement *pElement);
	HRESULT OnButtonAdvanceSetting(IHTMLElement *pElement);
	HRESULT OnColorSub(IHTMLElement *pElement);
	HRESULT OnFontSetting(IHTMLElement *pElement);
	bool bOpenAdvancePanel;
	int idPage;
// Dialog Data
	enum { IDD = IDD_DHTML_SETTING, IDH = IDR_HTML_UESETTINGPANEL };
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};