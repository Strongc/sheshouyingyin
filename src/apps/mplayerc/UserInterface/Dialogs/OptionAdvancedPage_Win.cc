#include "stdafx.h"
#include "OptionAdvancedPage_Win.h"
#include <Strings.h>
#include "../../mplayerc.h"
#include "../../MainFrm.h"
#include "../../Controller/PlayerPreference.h"
#include "../../Controller/SPlayerDefs.h"

BOOL OptionAdvancedPage::OnInitDialog(HWND hwnd, LPARAM lParam)
{
  m_gpuaccelcheckbox.Attach(GetDlgItem(IDC_CHECK_ENABLEGPUACCEL));

  m_speakers_combo.Attach(GetDlgItem(IDC_COMBO_SPEAKERS));

  WTL::CString text;
  text.LoadString(IDS_NUMBEROFSPEAKERSETTING);
  std::vector<std::wstring> text_ar;
  Strings::Split(text, L"|", text_ar);
  for (std::vector<std::wstring>::iterator it = text_ar.begin();
    it != text_ar.end(); it++)
  {
    m_speakers_combo.AddString(it->c_str());
    if ( ++it != text_ar.end())
      m_speakers_combo.SetItemData(m_speakers_combo.GetCount()-1, _wtoi(it->c_str()));
    else
      break;
  }

  AppSettings& s = AfxGetAppSettings();
  PlayerPreference* pref = PlayerPreference::GetInstance();

  m_mapcenterch2lr = !pref->GetIntVar(INTVAR_MAP_CENTERCH2LR)?false:true;

  m_videoqualitymode = s.iSVPRenderType?0:1;
  // TODO: ���ϵͳ�Ƿ�֧��Ӳ������
  m_gpuaccelcheckbox.EnableWindow(s.iSVPRenderType);
  m_enablegpuaccel = s.useGPUAcel;

  m_usecustomspeakersetting = s.fCustomSpeakers;
  m_speakers_combo.EnableWindow(m_usecustomspeakersetting);
  m_usespdifprority = s.fbUseSPDIF;
  m_usenormalize = s.fAudioNormalize;

  // set speaker settings as s.iDecSpeakers
  for (int i = 0; i < m_speakers_combo.GetCount(); i++)
  {
    if (m_speakers_combo.GetItemData(i) == s.iDecSpeakers)
    {
      m_speakers_combo.SetCurSel(i);
      break;
    }
  }

  DoDataExchange();
  return TRUE;
}

void OptionAdvancedPage::OnVideoModeUpdated(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  m_gpuaccelcheckbox.EnableWindow(nID == IDC_RADIO_PICTUREQUALITY);
}

void OptionAdvancedPage::OnCustomSpeakerChanged(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  m_speakers_combo.EnableWindow(IsDlgButtonChecked(IDC_CHECK_CUSTOMSPEAKER));
}

void OptionAdvancedPage::OnDestroy()
{
  m_gpuaccelcheckbox.Detach();
}

int OptionAdvancedPage::OnSetActive()
{
  return 0;
}

int OptionAdvancedPage::OnApply()
{
  // retrieve variables from screen
  DoDataExchange(TRUE);
  AppSettings& s = AfxGetAppSettings();
  PlayerPreference* pref = PlayerPreference::GetInstance();

  pref->SetIntVar(INTVAR_MAP_CENTERCH2LR, m_mapcenterch2lr?true:false);

  // feed variables into preference
  if (m_videoqualitymode == 0)
  {
    s.iSVPRenderType = 1;
    s.iDSVideoRendererType = 6;
    s.iRMVideoRendererType = 2;
    s.iQTVideoRendererType = 2;
    s.iAPSurfaceUsage = VIDRNDT_AP_TEXTURE3D;
  }
  else
  {
    s.iSVPRenderType = 0; 
    s.iDSVideoRendererType = 5;
    s.iRMVideoRendererType = 1;
    s.iQTVideoRendererType = 1;
  }
  s.useGPUAcel = m_enablegpuaccel;

  s.fbUseSPDIF = m_usespdifprority?true:false;
  s.fAudioNormalize = m_usenormalize?true:false;
  s.fAudioNormalizeRecover = true;


  s.fCustomSpeakers = m_usecustomspeakersetting?true:false;
  s.iSS = s.iDecSpeakers = m_speakers_combo.GetItemData(m_speakers_combo.GetCurSel());
  if (s.fCustomSpeakers)
  {
    s.bNotAutoCheckSpeaker = (int)(s.iSS /100)%10 + (int)(s.iSS/10) %10 + s.iSS%10; 
    s.SetNumberOfSpeakers(s.iSS, s.bNotAutoCheckSpeaker);
  }
  //TODO: if audio setting is changed and audio clip is running,
  //we should apply it by using CComQIPtr<IAudioSwitcherFilter> m_pASF etc
  CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
  pFrame->SendMessage(WM_COMMAND, ID_UPDATE_AUDIOSETIING);

  return PSNRET_NOERROR;
}