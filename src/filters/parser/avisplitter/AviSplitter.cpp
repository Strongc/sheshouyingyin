/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "StdAfx.h"
#include <mmreg.h>
#include "AviFile.h"
#include "AviReportWnd.h"
#include "AviSplitter.h"
#include "../../../svplib/svplib.h"
#include <afxtempl.h>
#include "..\..\..\apps\mplayerc\mplayerc.h"

#define TRACE __noop
#undef  SVP_LogMsg5
#define SVP_LogMsg5 __noop
#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_Avi},
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CAviSplitterFilter), L"Avi Splitter", MERIT_NORMAL+1, countof(sudpPins), sudpPins},
	{&__uuidof(CAviSourceFilter), L"Avi Source", MERIT_NORMAL+1, 0, NULL},
};

CFactoryTemplate g_Templates[] =
{
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CAviSplitterFilter>, NULL, &sudFilter[0]},
	{sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CAviSourceFilter>, NULL, &sudFilter[1]},
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	CAtlList<CString> chkbytes;
	chkbytes.AddTail(_T("0,4,,52494646,8,4,,41564920")); // 'RIFF' ... 'AVI '
	chkbytes.AddTail(_T("0,4,,52494646,8,4,,41564958")); // 'RIFF' ... 'AVIX'
	chkbytes.AddTail(_T("0,4,,52494646,8,4,,414D5620")); // 'RIFF' ... 'AMV '

	RegisterSourceFilter(
		CLSID_AsyncReader, 
		MEDIASUBTYPE_Avi, 
		chkbytes, 
		_T(".avi"), _T(".divx"), _T(".vp6"), _T(".amv"), NULL);

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
//	UnRegisterSourceFilter(MEDIASUBTYPE_Avi);

	return AMovieDllRegisterServer2(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

class CAviSplitterApp : public CWinApp
{
public:
	CAviSplitterApp() {}

	BOOL InitInstance()
	{
		if(!__super::InitInstance()) return FALSE;
		DllEntryPoint(m_hInstance, DLL_PROCESS_ATTACH, 0);
		return TRUE;
	}

	BOOL ExitInstance()
	{
		DllEntryPoint(m_hInstance, DLL_PROCESS_DETACH, 0);
		return __super::ExitInstance();
	}

	void SetDefaultRegistryKey()
	{
		SetRegistryKey(_T("Gabest"));
	}

	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CAviSplitterApp, CWinApp)
END_MESSAGE_MAP()

CAviSplitterApp theApp;

#endif

//
// CAviSplitterFilter
//

CAviSplitterFilter::CAviSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseSplitterFilter(NAME("CAviSplitterFilter"), pUnk, phr, __uuidof(this))
	, m_timeformat(TIME_FORMAT_MEDIA_TIME)
{
}

STDMETHODIMP CAviSplitterFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	*ppv = NULL;

	return 
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CAviSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	CheckPointer(pAsyncReader, E_POINTER);

	HRESULT hr = E_FAIL;
	BOOL bHasVideo = FALSE;
	m_pFile.Free();
	m_tFrame.Free();

	m_pFile.Attach(new CAviFile(pAsyncReader, hr));
	if(!m_pFile) return E_OUTOFMEMORY;

	bool fShiftDown = !!(::GetKeyState(VK_SHIFT)&0x8000);
	bool fShowWarningText = !m_pFile->IsInterleaved(fShiftDown);

	if(SUCCEEDED(hr) && (fShowWarningText || fShiftDown))
	{
#ifdef REGISTER_FILTER
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
		bool fHideWarning = TRUE;//!!AfxGetMyApp()->GetProfileInt(_T("Settings"), _T("HideAviSplitterWarning"), TRUE);

		if(!fHideWarning && !dynamic_cast<CAviSourceFilter*>(this) || fShiftDown)
		{
			CAviReportWnd wnd;
			fHideWarning = wnd.DoModal(m_pFile, fHideWarning, fShowWarningText);
			//AfxGetMyApp()->WriteProfileInt(_T("Settings"), _T("HideAviSplitterWarning"), fHideWarning);
		}

		//if(fShowWarningText) hr = E_FAIL;
	}

	if(FAILED(hr)) {m_pFile.Free(); return hr;}

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = m_rtDuration = m_pFile->GetTotalTime();

	bool fHasIndex = false;

	for(DWORD i = 0; !fHasIndex && i < m_pFile->m_strms.GetCount(); i++)
		if(m_pFile->m_strms[i]->cs.GetCount() > 0) 
			fHasIndex = true;

	for(DWORD i = 0; i < m_pFile->m_strms.GetCount(); i++)
	{
		CAviFile::strm_t* s = m_pFile->m_strms[i];

		if(fHasIndex && s->cs.GetCount() == 0) continue;

		CMediaType mt;
		CAtlArray<CMediaType> mts;
		
		CStringW name, label;

		if(s->strh.fccType == FCC('vids'))
		{
			label = L"Video";

			ASSERT(s->strf.GetCount() >= sizeof(BITMAPINFOHEADER));

			BITMAPINFOHEADER* pbmi = &((BITMAPINFO*)s->strf.GetData())->bmiHeader;

			mt.majortype = MEDIATYPE_Video;
			mt.subtype = FOURCCMap(pbmi->biCompression);
			mt.formattype = FORMAT_VideoInfo;
			VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER) + s->strf.GetCount() - sizeof(BITMAPINFOHEADER));
			SVP_LogMsg5(L"avi len %d %d %d" ,  s->strf.GetCount() ,  mt.FormatLength() , sizeof(VIDEOINFOHEADER) + s->strf.GetCount() - sizeof(BITMAPINFOHEADER));
			memset(mt.Format(), 0, mt.FormatLength());
			memcpy(&pvih->bmiHeader, s->strf.GetData(), s->strf.GetCount());
			if(s->strh.dwRate > 0) pvih->AvgTimePerFrame = 10000000i64 * s->strh.dwScale / s->strh.dwRate;
			switch(pbmi->biCompression)
			{
			case BI_RGB: case BI_BITFIELDS: mt.subtype = 
						pbmi->biBitCount == 1 ? MEDIASUBTYPE_RGB1 :
						pbmi->biBitCount == 4 ? MEDIASUBTYPE_RGB4 :
						pbmi->biBitCount == 8 ? MEDIASUBTYPE_RGB8 :
						pbmi->biBitCount == 16 ? MEDIASUBTYPE_RGB565 :
						pbmi->biBitCount == 24 ? MEDIASUBTYPE_RGB24 :
						pbmi->biBitCount == 32 ? MEDIASUBTYPE_ARGB32 :
						MEDIASUBTYPE_NULL;
						break;
//			case BI_RLE8: mt.subtype = MEDIASUBTYPE_RGB8; break;
//			case BI_RLE4: mt.subtype = MEDIASUBTYPE_RGB4; break;
			}

			/* Extract palette from extradata if bpp <= 8
			* (assumes that extradata contains only palette but appears
			*  to be true for all palettized codecs we support) */
			if( pbmi->biBitCount > 0 && pbmi->biBitCount <= 8 )
			{
				/* The palette is not always included in biSize */
			/*
				fmt.i_extra = p_vids->i_chunk_size - sizeof(BITMAPINFOHEADER);
				if( fmt.i_extra > 0 )
				{
					const uint8_t *p_pal = fmt.p_extra;

					fmt.video.p_palette = calloc( 1, sizeof(video_palette_t) );
					fmt.video.p_palette->i_entries = __MIN(fmt.i_extra/4, 256);

					for( int i = 0; i < fmt.video.p_palette->i_entries; i++ )
					{
						for( int j = 0; j < 4; j++ )
							fmt.video.p_palette->palette[i][j] = p_pal[4*i+j];
					}
				}
				*/
			}
			if(s->cs.GetCount() && pvih->AvgTimePerFrame > 0)
			{
				__int64 size = 0;
				for(int i = 0; i < s->cs.GetCount(); i++)
					size += s->cs[i].orgsize;
				pvih->dwBitRate = size*8 / s->cs.GetCount() * 10000000i64 / pvih->AvgTimePerFrame;
			}

			mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0 
				? s->strh.dwSuggestedBufferSize*3/2
				: (pvih->bmiHeader.biWidth*pvih->bmiHeader.biHeight*4));
			bHasVideo = TRUE;
			mts.Add(mt);
		}
		else if(s->strh.fccType == FCC('auds') || s->strh.fccType == FCC('amva'))
		{
			label = L"Audio";

			ASSERT(s->strf.GetCount() >= sizeof(WAVEFORMATEX)
				|| s->strf.GetCount() == sizeof(PCMWAVEFORMAT));

            WAVEFORMATEX* pwfe = (WAVEFORMATEX*)s->strf.GetData();

			if(pwfe->nBlockAlign == 0) continue;

			mt.majortype = MEDIATYPE_Audio;
			if (m_pFile->m_isamv)
				mt.subtype = FOURCCMap(MAKEFOURCC('A','M','V','A'));
			else
				mt.subtype = FOURCCMap(pwfe->wFormatTag);
			mt.formattype = FORMAT_WaveFormatEx;
			mt.SetFormat(s->strf.GetData(), max(s->strf.GetCount(), sizeof(WAVEFORMATEX)));
			pwfe = (WAVEFORMATEX*)mt.Format();
			if(s->strf.GetCount() == sizeof(PCMWAVEFORMAT)) pwfe->cbSize = 0;
			if(pwfe->wFormatTag == WAVE_FORMAT_PCM) pwfe->nBlockAlign = pwfe->nChannels*pwfe->wBitsPerSample>>3;
			if(pwfe->wFormatTag == WAVE_FORMAT_EXTENSIBLE) mt.subtype = FOURCCMap(WAVE_FORMAT_PCM); // audio renderer doesn't accept fffe in the subtype
			mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0 
				? s->strh.dwSuggestedBufferSize*3/2
				: (pwfe->nChannels*pwfe->nSamplesPerSec*32>>3));
			mts.Add(mt);
		}
		else if(s->strh.fccType == FCC('mids'))
		{
			label = L"Midi";

			mt.majortype = MEDIATYPE_Midi;
			mt.subtype = MEDIASUBTYPE_NULL;
			mt.formattype = FORMAT_None;
			mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0 
				? s->strh.dwSuggestedBufferSize*3/2
				: (1024*1024));
			mts.Add(mt);
		}
		else if(s->strh.fccType == FCC('txts'))
		{
			label = L"Text";

			mt.majortype = MEDIATYPE_Text;
			mt.subtype = MEDIASUBTYPE_NULL;
			mt.formattype = FORMAT_None;
			mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0 
				? s->strh.dwSuggestedBufferSize*3/2
				: (1024*1024));
			mts.Add(mt);
		}
		else if(s->strh.fccType == FCC('iavs'))
		{
			label = L"Interleaved";

			ASSERT(s->strh.fccHandler == FCC('dvsd'));

			mt.majortype = MEDIATYPE_Interleaved;
			mt.subtype = FOURCCMap(s->strh.fccHandler);
			mt.formattype = FORMAT_DvInfo;
			mt.SetFormat(s->strf.GetData(), max(s->strf.GetCount(), sizeof(DVINFO)));
			mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0 
				? s->strh.dwSuggestedBufferSize*3/2
				: (1024*1024));
			bHasVideo = TRUE;
			mts.Add(mt);
		}else{
			SVP_LogMsg5(L"CAviSourceFilter: Unsupported fcc %x" , s->strh.fccType);
		}

		if(mts.IsEmpty())
		{
			TRACE(_T("CAviSourceFilter: Unsupported stream (%d)\n"), i);
			continue;
		}

		name.Format(L"%s %d (%s)", label, i , CStringW(s->strn) );
		SVP_LogMsg5(L"CAviSourceFilter Got %s", name);

		HRESULT hr;

		CAutoPtr<CBaseSplitterOutputPin> pPinOut(new CAviSplitterOutputPin(mts, name, this, this, &hr));
		AddOutputPin(i, pPinOut);
	}

	POSITION pos = m_pFile->m_info.GetStartPosition();
	while(pos)
	{
		DWORD fcc;
		CStringA value;
		m_pFile->m_info.GetNextAssoc(pos, fcc, value);

        switch(fcc)
		{
		case FCC('INAM'): SetProperty(L"TITL", CStringW(value)); break;
		case FCC('IART'): SetProperty(L"AUTH", CStringW(value)); break;
		case FCC('ICOP'): SetProperty(L"CPYR", CStringW(value)); break;
		case FCC('ISBJ'): SetProperty(L"DESC", CStringW(value)); break;
		}
	}

	m_tFrame.Attach(new DWORD[m_pFile->m_avih.dwStreams]);
	if(!bHasVideo){
		SVP_LogMsg5(L"Got No Video");
		return E_FAIL;
	}

	return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

bool CAviSplitterFilter::DemuxInit()
{
	if(!m_pFile) return(false);

	
	// reindex if needed

	bool fReIndex = false;

	for(int i = 0; i < (int)m_pFile->m_avih.dwStreams && !fReIndex; i++)
	{
		if(m_pFile->m_strms[i]->cs.GetCount() == 0 && GetOutputPin(i)) 
			fReIndex = true;
	}

	if(fReIndex)
	{
		m_pFile->EmptyIndex();

		m_fAbort = false;
		m_nOpenProgress = 0;

		m_rtDuration = 0;

		CAutoVectorPtr<UINT64> pSize;
		pSize.Allocate(m_pFile->m_avih.dwStreams);
		memset((UINT64*)pSize, 0, sizeof(UINT64)*m_pFile->m_avih.dwStreams);
		m_pFile->Seek(0);
		m_llLastPos = 0;
        ReIndex(m_pFile->GetLength(), pSize);

		if(m_fAbort) m_pFile->EmptyIndex();

		m_fAbort = false;
		m_nOpenProgress = 100;
	}

	return(true);
}

HRESULT CAviSplitterFilter::ReIndex(__int64 end, UINT64* pSize)
{
	HRESULT hr = S_OK;

	while(S_OK == hr && m_pFile->GetPos() < end && SUCCEEDED(hr) && !m_fAbort)
	{
		__int64 pos = m_pFile->GetPos();

		DWORD id = 0, size;
		if(S_OK != m_pFile->Read(id) || id == 0)
			return E_FAIL;

		if(id == FCC('RIFF') || id == FCC('LIST'))
		{
			if(S_OK != m_pFile->Read(size) || S_OK != m_pFile->Read(id))
				return E_FAIL;

			size += (size&1) + 8;

			if(id == FCC('AVI ') || id == FCC('AVIX') || id == FCC('movi') || id == FCC('rec '))
				hr = ReIndex(pos + size, pSize);
		}
		else
		{
			if(S_OK != m_pFile->Read(size))
				return E_FAIL;

			DWORD TrackNumber = TRACKNUM(id);

			if(TrackNumber < m_pFile->m_strms.GetCount())
			{
				CAviFile::strm_t* s = m_pFile->m_strms[TrackNumber];

				WORD type = TRACKTYPE(id);

				if(type == 'db' || type == 'dc' || /*type == 'pc' ||*/ type == 'wb'
				|| type == 'iv' || type == '__' || type == 'xx')
				{
					CAviFile::strm_t::chunk c;
					c.filepos = pos;
					c.size = pSize[TrackNumber];
					c.orgsize = size;
					c.fKeyFrame = size > 0; // TODO: find a better way...
					c.fChunkHdr = true;
					s->cs.Add(c);

					pSize[TrackNumber] += s->GetChunkSize(size);

					REFERENCE_TIME rt = s->GetRefTime(s->cs.GetCount()-1, pSize[TrackNumber]);
					m_rtDuration = max(rt, m_rtDuration);
				}
			}

			size += (size&1) + 8;
		}

		m_pFile->Seek(pos + size);

		m_nOpenProgress = m_pFile->GetPos()*100/m_pFile->GetLength();

		DWORD cmd;
		if(CheckRequest(&cmd))
		{
			if(cmd == CMD_EXIT) m_fAbort = true;
			else Reply(S_OK);
		}
	}

	return hr;
}

void CAviSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	memset((DWORD*)m_tFrame, 0, sizeof(DWORD)*m_pFile->m_avih.dwStreams);
	m_pFile->Seek(0);
	m_llLastPos = 0;

	DbgLog((LOG_TRACE, 0, _T("Seek: %I64d"), rt/10000));

	if(rt > 0)
	{
		UINT64 minfp = _I64_MAX;

		for(int j = 0; j < (int)m_pFile->m_strms.GetCount(); j++)
		{
			CAviFile::strm_t* s = m_pFile->m_strms[j];

			int f = s->GetKeyFrame(rt);
			UINT64 fp = f >= 0 ? s->cs[f].filepos : m_pFile->GetLength();

			if(!s->IsRawSubtitleStream())
				minfp = min(minfp, fp);
		}

		for(int j = 0; j < (int)m_pFile->m_strms.GetCount(); j++)
		{
			CAviFile::strm_t* s = m_pFile->m_strms[j];

			for(int i = 0; i < s->cs.GetCount(); i++)
			{
				CAviFile::strm_t::chunk& c = s->cs[i];
				if(c.filepos >= minfp)
				{
					m_tFrame[j] = i;
					break;
				}
			}
		}

		DbgLog((LOG_TRACE, 0, _T("minfp: %I64d"), minfp));
	}
}

bool CAviSplitterFilter::DemuxLoop()
{
	HRESULT hr = S_OK;

	int nTracks = (int)m_pFile->m_strms.GetCount();

	CAtlArray<BOOL> fDiscontinuity;
	fDiscontinuity.SetCount(nTracks);
	memset(fDiscontinuity.GetData(), 0, nTracks*sizeof(bool));

	while(SUCCEEDED(hr) && !CheckRequest(NULL))
	{
		int minTrack = nTracks;
		UINT64 minFilePos = _I64_MAX;

		for(int i = 0; i < nTracks; i++)
		{
			CAviFile::strm_t* s = m_pFile->m_strms[i];

			DWORD f = m_tFrame[i];
			if(f >= (DWORD)s->cs.GetCount()) continue;

			bool fUrgent = s->IsRawSubtitleStream();

			if(fUrgent || s->cs[f].filepos < minFilePos)
			{
				minTrack = i;
				minFilePos = s->cs[f].filepos;
			}

			if(fUrgent) break;
		}

		SVP_LogMsg5(L"%d %d",minTrack , nTracks);
		if(minTrack == nTracks)
			break;

		DWORD& f = m_tFrame[minTrack];

		do
		{
			CAviFile::strm_t* s = m_pFile->m_strms[minTrack];

			m_llLastPos = m_pFile->GetPos();
			m_pFile->Seek(s->cs[f].filepos);

			DWORD size = 0;

			if(s->cs[f].fChunkHdr)
			{
				//SVP_LogMsg5(L"fChunkHdr");
				UINT64 expectedsize = -1;
				expectedsize = f < (DWORD)s->cs.GetCount()-1
					? s->cs[f+1].size - s->cs[f].size
					: s->totalsize - s->cs[f].size;

				DWORD id = 0;
				if(S_OK != m_pFile->Read(id) || id == 0 || minTrack != TRACKNUM(id)
				|| S_OK != m_pFile->Read(size))
				{
					int itry = 1;
					__int64 llGuessPos = m_llLastPos;
					if(m_llLastPos < s->cs[f].filepos){
						SVP_LogMsg5(L"fChunkHdr %f %f" , (double) m_llLastPos , (double) s->cs[f].filepos );
						llGuessPos = m_llLastPos;
					}else{
							SVP_LogMsg5(L"fChunkHdr2"  );
						llGuessPos = s->cs[f].filepos;
					}
					
                    if(!fDiscontinuity[minTrack]){
					    while(1){
						    if(itry % 2){
							    m_pFile->Seek(llGuessPos+(itry>>1));
						    }else{
							    m_pFile->Seek(llGuessPos-(itry>>1));
						    }
    						
						    /*S_OK ==*/ m_pFile->Read(id);
						    if(id != 0 && minTrack == TRACKNUM(id)){
							    if(S_OK == m_pFile->Read(size) && size > 0 && s->GetChunkSize(size) == expectedsize){
								    SVP_LogMsg5(L"fChunkHdr3 tried %d"  , itry);
								    break;
							    }
						    }
						    itry++;
						    if(itry%3000 == 0){
							    Sleep(1);
						    }
						    if(itry > 40962){
							    id=0;
							    break;
						    }
					    }
                    }
					if(id == 0 || minTrack != TRACKNUM(id) ){
						fDiscontinuity[minTrack] = true;
						SVP_LogMsg5(L"fDiscontinuity %d %f %x %d", minTrack ,(double)s->cs[f].filepos , id , TRACKNUM(id) );
						break;
					}else{
						 //m_pFile->Read(size);
						 SVP_LogMsg5(L"fContinuity %f %f %d %d",(double) s->cs[f].filepos , (double) m_pFile->GetPos(), itry ,size  );

					}
				}

			
				DWORD realChunkSize = s->GetChunkSize(size);
				if(expectedsize != realChunkSize)
				{
					fDiscontinuity[minTrack] = true;
					SVP_LogMsg5(L"fDiscontinuity2 %d %d %d" , minTrack, expectedsize, realChunkSize);
					// ASSERT(0);
					//break;
				}
				 m_llLastPos = m_pFile->GetPos();
			}
			else
			{
				size = s->cs[f].orgsize;
			}

			CAutoPtr<Packet> p(new Packet());

			p->TrackNumber = minTrack;
			p->bSyncPoint = (BOOL)s->cs[f].fKeyFrame;
			p->bDiscontinuity = fDiscontinuity[minTrack];
			p->rtStart = s->GetRefTime(f, s->cs[f].size);
			p->rtStop = s->GetRefTime(f+1, f+1 < (DWORD)s->cs.GetCount() ? s->cs[f+1].size : s->totalsize);
			
			p->SetCount(size);
			if(S_OK != (hr = m_pFile->ByteRead(p->GetData(), p->GetCount()))) {
				SVP_LogMsg5(L"read %x" , hr);
				return(true); // break;
			}

			SVP_LogMsg5( _T("AVI Demux %d (%d): %I64d - %I64d, %I64d - %I64d (size = %d)"), 
				minTrack, (int)p->bSyncPoint,
				(p->rtStart)/10000, (p->rtStop)/10000, 
				(p->rtStart-m_rtStart)/10000, (p->rtStop-m_rtStart)/10000,
				size);

			hr = DeliverPacket(p);

			fDiscontinuity[minTrack] = false;
		}
		while(0);

		f++;
	}

	return(true);
}

// IMediaSeeking

STDMETHODIMP CAviSplitterFilter::GetDuration(LONGLONG* pDuration)
{
	CheckPointer(pDuration, E_POINTER);
	CheckPointer(m_pFile, VFW_E_NOT_CONNECTED);

	if(m_timeformat == TIME_FORMAT_FRAME)
	{
		for(int i = 0; i < (int)m_pFile->m_strms.GetCount(); i++)
		{
			CAviFile::strm_t* s = m_pFile->m_strms[i];
			if(s->strh.fccType == FCC('vids'))
			{
				*pDuration = s->cs.GetCount();
				return S_OK;
			}
		}

		return E_UNEXPECTED;
	}

	return __super::GetDuration(pDuration);
}

//

STDMETHODIMP CAviSplitterFilter::IsFormatSupported(const GUID* pFormat)
{
	CheckPointer(pFormat, E_POINTER);
	HRESULT hr = __super::IsFormatSupported(pFormat);
	if(S_OK == hr) return hr;
	return *pFormat == TIME_FORMAT_FRAME ? S_OK : S_FALSE;
}

STDMETHODIMP CAviSplitterFilter::GetTimeFormat(GUID* pFormat)
{
	CheckPointer(pFormat, E_POINTER);
	*pFormat = m_timeformat;
	return S_OK;
}

STDMETHODIMP CAviSplitterFilter::IsUsingTimeFormat(const GUID* pFormat)
{
	CheckPointer(pFormat, E_POINTER);
	return *pFormat == m_timeformat ? S_OK : S_FALSE;
}

STDMETHODIMP CAviSplitterFilter::SetTimeFormat(const GUID* pFormat)
{
	CheckPointer(pFormat, E_POINTER);
	if(S_OK != IsFormatSupported(pFormat)) return E_FAIL;
	m_timeformat = *pFormat;
	return S_OK;
}

STDMETHODIMP CAviSplitterFilter::GetStopPosition(LONGLONG* pStop)
{
	CheckPointer(pStop, E_POINTER);
	if(FAILED(__super::GetStopPosition(pStop))) return E_FAIL;
	if(m_timeformat == TIME_FORMAT_MEDIA_TIME) return S_OK;
	LONGLONG rt = *pStop;
	if(FAILED(ConvertTimeFormat(pStop, &TIME_FORMAT_FRAME, rt, &TIME_FORMAT_MEDIA_TIME))) return E_FAIL;
	return S_OK;
}

STDMETHODIMP CAviSplitterFilter::ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat)
{
	CheckPointer(pTarget, E_POINTER);

	const GUID& SourceFormat = pSourceFormat ? *pSourceFormat : m_timeformat;
	const GUID& TargetFormat = pTargetFormat ? *pTargetFormat : m_timeformat;
	
	if(TargetFormat == SourceFormat)
	{
		*pTarget = Source; 
		return S_OK;
	}
	else if(TargetFormat == TIME_FORMAT_FRAME && SourceFormat == TIME_FORMAT_MEDIA_TIME)
	{
		for(int i = 0; i < (int)m_pFile->m_strms.GetCount(); i++)
		{
			CAviFile::strm_t* s = m_pFile->m_strms[i];
			if(s->strh.fccType == FCC('vids'))
			{
				*pTarget = s->GetFrame(Source);
				return S_OK;
			}
		}
	}
	else if(TargetFormat == TIME_FORMAT_MEDIA_TIME && SourceFormat == TIME_FORMAT_FRAME)
	{
		for(int i = 0; i < (int)m_pFile->m_strms.GetCount(); i++)
		{
			CAviFile::strm_t* s = m_pFile->m_strms[i];
			if(s->strh.fccType == FCC('vids'))
			{
				if(Source < 0 || Source >= s->cs.GetCount()) return E_FAIL;
				CAviFile::strm_t::chunk& c = s->cs[(int)Source];
				*pTarget = s->GetRefTime((DWORD)Source, c.size);
				return S_OK;
			}
		}
	}
	
	return E_FAIL;
}

STDMETHODIMP CAviSplitterFilter::GetPositions(LONGLONG* pCurrent, LONGLONG* pStop)
{
	HRESULT hr;
	if(FAILED(hr = __super::GetPositions(pCurrent, pStop)) || m_timeformat != TIME_FORMAT_FRAME)
		return hr;

	if(pCurrent)
		if(FAILED(ConvertTimeFormat(pCurrent, &TIME_FORMAT_FRAME, *pCurrent, &TIME_FORMAT_MEDIA_TIME))) return E_FAIL;
	if(pStop)
		if(FAILED(ConvertTimeFormat(pStop, &TIME_FORMAT_FRAME, *pStop, &TIME_FORMAT_MEDIA_TIME))) return E_FAIL;

	return S_OK;
}

HRESULT CAviSplitterFilter::SetPositionsInternal(void* id, LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags)
{
	if(m_timeformat != TIME_FORMAT_FRAME)
		return __super::SetPositionsInternal(id, pCurrent, dwCurrentFlags, pStop, dwStopFlags);

	if(!pCurrent && !pStop
	|| (dwCurrentFlags&AM_SEEKING_PositioningBitsMask) == AM_SEEKING_NoPositioning 
		&& (dwStopFlags&AM_SEEKING_PositioningBitsMask) == AM_SEEKING_NoPositioning)
		return S_OK;

	REFERENCE_TIME 
		rtCurrent = m_rtCurrent,
		rtStop = m_rtStop;

	if((dwCurrentFlags&AM_SEEKING_PositioningBitsMask)
	&& FAILED(ConvertTimeFormat(&rtCurrent, &TIME_FORMAT_FRAME, rtCurrent, &TIME_FORMAT_MEDIA_TIME))) 
		return E_FAIL;
	if((dwStopFlags&AM_SEEKING_PositioningBitsMask)
	&& FAILED(ConvertTimeFormat(&rtStop, &TIME_FORMAT_FRAME, rtStop, &TIME_FORMAT_MEDIA_TIME)))
		return E_FAIL;

	if(pCurrent)
	switch(dwCurrentFlags&AM_SEEKING_PositioningBitsMask)
	{
	case AM_SEEKING_NoPositioning: break;
	case AM_SEEKING_AbsolutePositioning: rtCurrent = *pCurrent; break;
	case AM_SEEKING_RelativePositioning: rtCurrent = rtCurrent + *pCurrent; break;
	case AM_SEEKING_IncrementalPositioning: rtCurrent = rtCurrent + *pCurrent; break;
	}

	if(pStop)
	switch(dwStopFlags&AM_SEEKING_PositioningBitsMask)
	{
	case AM_SEEKING_NoPositioning: break;
	case AM_SEEKING_AbsolutePositioning: rtStop = *pStop; break;
	case AM_SEEKING_RelativePositioning: rtStop += *pStop; break;
	case AM_SEEKING_IncrementalPositioning: rtStop = rtCurrent + *pStop; break;
	}

	if((dwCurrentFlags&AM_SEEKING_PositioningBitsMask)
	&& pCurrent)
		if(FAILED(ConvertTimeFormat(pCurrent, &TIME_FORMAT_MEDIA_TIME, rtCurrent, &TIME_FORMAT_FRAME))) return E_FAIL;
	if((dwStopFlags&AM_SEEKING_PositioningBitsMask)
	&& pStop)
		if(FAILED(ConvertTimeFormat(pStop, &TIME_FORMAT_MEDIA_TIME, rtStop, &TIME_FORMAT_FRAME))) return E_FAIL;

	return __super::SetPositionsInternal(id, pCurrent, dwCurrentFlags, pStop, dwStopFlags);
}

// IKeyFrameInfo

STDMETHODIMP CAviSplitterFilter::GetKeyFrameCount(UINT& nKFs)
{
	if(!m_pFile) return E_UNEXPECTED;

	HRESULT hr = S_OK;

	nKFs = 0;

	for(int i = 0; i < (int)m_pFile->m_strms.GetCount(); i++)
	{
		CAviFile::strm_t* s = m_pFile->m_strms[i];
		if(s->strh.fccType != FCC('vids')) continue;

		for(int j = 0; j < s->cs.GetCount(); j++)
		{
			CAviFile::strm_t::chunk& c = s->cs[j];
			if(c.fKeyFrame) nKFs++;
		}

		if(nKFs == s->cs.GetCount())
			hr = S_FALSE;

		break;
	}

	return hr;
}

STDMETHODIMP CAviSplitterFilter::GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs)
{
	CheckPointer(pFormat, E_POINTER);
	CheckPointer(pKFs, E_POINTER);

	if(!m_pFile) return E_UNEXPECTED;
	if(*pFormat != TIME_FORMAT_MEDIA_TIME && *pFormat != TIME_FORMAT_FRAME) return E_INVALIDARG;

	UINT nKFsTmp = 0;

	for(int i = 0; i < (int)m_pFile->m_strms.GetCount(); i++)
	{
		CAviFile::strm_t* s = m_pFile->m_strms[i];
		if(s->strh.fccType != FCC('vids')) continue;

		bool fConvertToRefTime = !!(*pFormat == TIME_FORMAT_MEDIA_TIME);

		for(int j = 0; j < s->cs.GetCount() && nKFsTmp < nKFs; j++)
		{
			if(s->cs[j].fKeyFrame)
				pKFs[nKFsTmp++] = fConvertToRefTime ? s->GetRefTime(j, s->cs[j].size) : j;
		}

		break;
	}

	nKFs = nKFsTmp;

	return S_OK;
}

//
// CAviSourceFilter
//

CAviSourceFilter::CAviSourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CAviSplitterFilter(pUnk, phr)
{
	m_clsid = __uuidof(this);
	m_pInput.Free();
}

//
// CAviSplitterOutputPin
//

CAviSplitterOutputPin::CAviSplitterOutputPin(CAtlArray<CMediaType>& mts, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
	: CBaseSplitterOutputPin(mts, pName, pFilter, pLock, phr)
{
}

HRESULT CAviSplitterOutputPin::CheckConnect(IPin* pPin)
{
	int iPosition = 0;
	CMediaType mt;
	while(S_OK == GetMediaType(iPosition++, &mt))
	{
		if(mt.majortype == MEDIATYPE_Video 
		&& (mt.subtype == FOURCCMap(FCC('IV32'))
		|| mt.subtype == FOURCCMap(FCC('IV31'))
		|| mt.subtype == FOURCCMap(FCC('IF09'))))
		{
			CLSID clsid = GetCLSID(GetFilterFromPin(pPin));
			if(clsid == CLSID_VideoMixingRenderer || clsid == CLSID_OverlayMixer)
				return E_FAIL;
		}

		mt.InitMediaType();
	}

	return __super::CheckConnect(pPin);
}

