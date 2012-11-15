#include "StdAfx.h"
#include "AviFile.h"

#include "../../../svplib/svplib.h"
//
// CAviFile
//
#define  TRACE __noop
#undef  SVP_LogMsg5
#define  SVP_LogMsg5  __noop

CAviFile::CAviFile(IAsyncReader* pAsyncReader, HRESULT& hr)
	: CBaseSplitterFile(pAsyncReader, hr)
{
	if(FAILED(hr)) return;
	m_isamv = false;
	hr = Init();
}

template<typename T> 
HRESULT CAviFile::Read(T& var, int offset)
{
	memset(&var, 0, sizeof(var));
	HRESULT hr = ByteRead((BYTE*)&var + offset, sizeof(var) - offset);
	return hr;
}

HRESULT CAviFile::Init()
{
	Seek(0);
	DWORD dw[3];
	if(S_OK != Read(dw) || dw[0] != FCC('RIFF') || (dw[2] != FCC('AVI ') && dw[2] != FCC('AVIX') && dw[2] != FCC('AMV ')))
		return E_FAIL;

	m_isamv = (dw[2] == FCC('AMV '));
	Seek(0);
	HRESULT hr = Parse(0, GetLength());
	if(m_movis.GetCount() == 0) // FAILED(hr) is allowed as long as there was a movi chunk found
		return E_FAIL;

	if(m_avih.dwStreams == 0 && m_strms.GetCount() > 0)
		m_avih.dwStreams = m_strms.GetCount();

	if(m_avih.dwStreams != m_strms.GetCount())
		return E_FAIL;

	for(int i = 0; i < (int)m_avih.dwStreams; i++)
	{
		strm_t* s = m_strms[i];
		if(s->strh.fccType != FCC('auds')) continue;
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)s->strf.GetData();
		if(wfe->wFormatTag == 0x55 && wfe->nBlockAlign == 1152 
		&& s->strh.dwScale == 1 && s->strh.dwRate != wfe->nSamplesPerSec)
		{
			// correcting encoder bugs...
			s->strh.dwScale = 1152;
			s->strh.dwRate = wfe->nSamplesPerSec;
		}
	}

	if (!m_isamv){
		HRESULT hrb = BuildIndex();
		if( (FAILED(hrb)))
			EmptyIndex();
		if(hrb == E_ABORT)
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CAviFile::BuildAMVIndex()
{
	strm_t::chunk	NewChunk;
	ULONG	ulType;
	ULONG	ulSize;

	memset (&NewChunk, 0, sizeof(strm_t::chunk));
	while((Read(ulType) == S_OK) && (Read(ulSize) == S_OK))
	{
		switch (ulType)
		{
		case FCC('00dc'):	// 01bw : JPeg
			NewChunk.size = ulSize;
			NewChunk.filepos = GetPos();
			NewChunk.orgsize = ulSize;
			NewChunk.fKeyFrame = true;
			m_strms[0]->cs.Add (NewChunk);
			break;
		case FCC('01wb') :	// 00dc : Audio
			NewChunk.size    = ulSize;
			NewChunk.orgsize = ulSize;
			NewChunk.fKeyFrame = true;
			NewChunk.filepos = GetPos();
			m_strms[1]->cs.Add (NewChunk);
			break;
		}
		Seek(GetPos() + ulSize);			
	}

	TRACE (L"Video packet : %d   Audio packet :%d\n", m_strms[0]->cs.GetCount(), m_strms[1]->cs.GetCount());
	return S_OK;
}


HRESULT CAviFile::Parse(DWORD parentid, __int64 end)
{
	HRESULT hr = S_OK;

	CAutoPtr<strm_t> strm;

	while(S_OK == hr && GetPos() < end)
	{
		UINT64 pos = GetPos();

		DWORD id = 0, size;
		if(S_OK != Read(id) || id == 0)
			return E_FAIL;

		if(id == FCC('RIFF') || id == FCC('LIST'))
		{
			if(S_OK != Read(size) || S_OK != Read(id))
				return E_FAIL;

			if (m_isamv) size = end - GetPos() - 8;		// No size set in AVM : guess end of file...
			size += (size&1) + 8;

			TRACE(_T("CAviFile::Parse(..): LIST '%c%c%c%c'\n"), 
				TCHAR((id>>0)&0xff), 
				TCHAR((id>>8)&0xff), 
				TCHAR((id>>16)&0xff),
				TCHAR((id>>24)&0xff));

			if(id == FCC('movi'))
			{
				m_movis.AddTail(pos);
				if (m_isamv) BuildAMVIndex();
			}
			else
			{
				hr = Parse(id, pos + size);
			}
		}
		else
		{
			if(S_OK != Read(size))
				return E_FAIL;

			TRACE(_T("CAviFile::Parse(..): '%c%c%c%c'\n"), 
				TCHAR((id>>0)&0xff), 
				TCHAR((id>>8)&0xff), 
				TCHAR((id>>16)&0xff),
				TCHAR((id>>24)&0xff));

			if(parentid == FCC('INFO') && size > 0)
			{
				switch(id)
				{
				case FCC('IARL'): // Archival Location. Indicates where the subject of the file is archived.
				case FCC('IART'): // Artist. Lists the artist of the original subject of the file; for example, �Michaelangelo.?
				case FCC('ICMS'): // Commissioned. Lists the name of the person or organization that commissioned the subject of the file; for example, �Pope Julian II.?
				case FCC('ICMT'): // Comments. Provides general comments about the file or the subject of the file. If the comment is several sentences long, end each sentence with a period. Do not include new-line characters.
				case FCC('ICOP'): // Copyright. Records the copyright information for the file; for example, �Copyright Encyclopedia International 1991.?If there are multiple copyrights, separate them by a semicolon followed by a space.
				case FCC('ICRD'): // Creation date. Specifies the date the subject of the file was created. List dates in year-month-day format, padding one-digit months and days with a zero on the left; for example, ?553-05-03?for May 3, 1553.
				case FCC('ICRP'): // Cropped. Describes whether an image has been cropped and, if so, how it was cropped; for example, �lower-right corner.?
				case FCC('IDIM'): // Dimensions. Specifies the size of the original subject of the file; for example, ?.5 in h, 11 in w.?
				case FCC('IDPI'): // Dots Per Inch. Stores dots per inch setting of the digitizer used to produce the file, such as ?00.?
				case FCC('IENG'): // Engineer. Stores the name of the engineer who worked on the file. If there are multiple engineers, separate the names by a semicolon and a blank; for example, �Smith, John; Adams, Joe.?
				case FCC('IGNR'): // Genre. Describes the original work, such as �landscape,?�portrait,?�still life,?etc.
				case FCC('IKEY'): // Keywords. Provides a list of keywords that refer to the file or subject of the file. Separate multiple keywords with a semicolon and a blank; for example, �Seattle; aerial view; scenery.?
				case FCC('ILGT'): // Lightness. Describes the changes in lightness settings on the digitizer required to produce the file. Note that the format of this information depends on hardware used.
				case FCC('IMED'): // Medium. Describes the original subject of the file, such as �computer image,?�drawing,?�lithograph,?and so on.
				case FCC('INAM'): // Name. Stores the title of the subject of the file, such as �Seattle From Above.?
				case FCC('IPLT'): // Palette Setting. Specifies the number of colors requested when digitizing an image, such as ?56.?
				case FCC('IPRD'): // Product. Specifies the name of the title the file was originally intended for, such as �Encyclopedia of Pacific Northwest Geography.?
				case FCC('ISBJ'): // Subject. Describes the contents of the file, such as �Aerial view of Seattle.?
				case FCC('ISFT'): // Software. Identifies the name of the software package used to create the file, such as �Microsoft WaveEdit.?
				case FCC('ISHP'): // Sharpness. Identifies the changes in sharpness for the digitizer required to produce the file (the format depends on the hardware used).
				case FCC('ISRC'): // Source. Identifies the name of the person or organization who supplied the original subject of the file; for example, �Trey Research.?
				case FCC('ISRF'): // Source Form. Identifies the original form of the material that was digitized, such as �slide,?�paper,?�map,?and so on. This is not necessarily the same as IMED.
				case FCC('ITCH'): // Technician. Identifies the technician who digitized the subject file; for example, �Smith, John.?
					{
						CStringA str;
						if(S_OK != ByteRead((BYTE*)str.GetBufferSetLength(size), size)) return E_FAIL;
						m_info[id] = str;
						SVP_LogMsg5(L"avi got info %x size %d %d", id , size, str[0]);
						break;
					}
				}
			}

			switch(id)
			{
			case FCC('amvh'):
			case FCC('avih'):
				m_avih.fcc = id;
				m_avih.cb = size;
				if(S_OK != Read(m_avih, 8)) return E_FAIL;
				break;
			case FCC('strh'):
				if(!strm) strm.Attach(new strm_t());
				strm->strh.fcc = FCC('strh');
				strm->strh.cb = size;
				if(S_OK != Read(strm->strh, 8)) return E_FAIL;
				if (m_isamv)
				{
					// First alway video, second always audio
					strm->strh.fccType = m_strms.GetCount() == 0 ? FCC('vids') : FCC('amva');
					strm->strh.dwRate  = m_avih.dwReserved[0]*1000;	// dwReserved[0] = fps!
					strm->strh.dwScale = 1000;
				}
				break;
			case FCC('strn'):
				if(S_OK != ByteRead((BYTE*)strm->strn.GetBufferSetLength(size), size)) return E_FAIL;
				break;
			case FCC('strf'):
				if(!strm) strm.Attach(new strm_t());
				strm->strf.SetCount(size);
				if(S_OK != ByteRead(strm->strf.GetData(), size)) return E_FAIL;
				if (m_isamv)
				{
					if (strm->strh.fccType == FCC('vids'))
					{
						strm->strf.SetCount(sizeof(BITMAPINFOHEADER));
						BITMAPINFOHEADER* pbmi = &((BITMAPINFO*)strm->strf.GetData())->bmiHeader;					
						pbmi->biSize		= sizeof(BITMAPINFOHEADER);
						pbmi->biHeight		= m_avih.dwHeight;
						pbmi->biWidth		= m_avih.dwWidth;
						pbmi->biCompression = FCC('AMVV');
						pbmi->biPlanes		= 1;
						pbmi->biBitCount	= 24;
						pbmi->biSizeImage	= pbmi->biHeight * pbmi->biWidth * (pbmi->biBitCount/8);
					}
					m_strms.Add(strm);
				}

				break;
			case FCC('indx'):
				if(!strm) strm.Attach(new strm_t());
				ASSERT(strm->indx == NULL);
				AVISUPERINDEX* pSuperIndex;
				if(size < MAXDWORD-8) {
					// Fixes vulnerabilities
					TRY
					{
						pSuperIndex = (AVISUPERINDEX*)new unsigned char [(size_t)(size + 8)];
					}
					CATCH (CMemoryException, e)
					{
						pSuperIndex = NULL;
					}
					END_CATCH
					if (pSuperIndex)
					{
						strm->indx.Attach(pSuperIndex);
						strm->indx->fcc = FCC('indx');
						strm->indx->cb = size;
						if(S_OK != ByteRead((BYTE*)(AVISUPERINDEX*)strm->indx + 8, size)) {
							return E_FAIL;
						}
						ASSERT(strm->indx->wLongsPerEntry == 4 && strm->indx->bIndexType == AVI_INDEX_OF_INDEXES);
					}
				}
				break;
			case FCC('dmlh'):
				if(S_OK != Read(m_dmlh)) return E_FAIL;
				break;
			case FCC('vprp'):
//				if(S_OK != Read(m_vprp)) return E_FAIL;
				break;
			case FCC('idx1'):
				
				ASSERT(m_idx1 == NULL);
				m_idx1.Attach((AVIOLDINDEX*)new BYTE[size + 8]);
				m_idx1->fcc = FCC('idx1');
				m_idx1->cb = size;
				SVP_LogMsg5(L"Got idx1 %d", m_idx1->cb );;
				if(S_OK != ByteRead((BYTE*)(AVIOLDINDEX*)m_idx1 + 8, size)) return E_FAIL;
				break;
			default :
				TRACE(_T("CAviFile::Parse(..): unknown tag '%c%c%c%c'\n"), 
					TCHAR((id>>0)&0xff), 
					TCHAR((id>>8)&0xff), 
					TCHAR((id>>16)&0xff),
					TCHAR((id>>24)&0xff));
				break;
			}

			size += (size&1) + 8;
		}

		Seek(pos + size);
	}

	if(strm) m_strms.Add(strm);

	return hr;
}

REFERENCE_TIME CAviFile::GetTotalTime()
{
	REFERENCE_TIME t = 0/*10i64*m_avih.dwMicroSecPerFrame*m_avih.dwTotalFrames*/;

	for(int i = 0; i < (int)m_avih.dwStreams; i++)
	{
		strm_t* s = m_strms[i];
		REFERENCE_TIME t2 = s->GetRefTime(s->cs.GetCount(), s->totalsize);
		t = max(t, t2);
	}

	if(t == 0) t = 10i64*m_avih.dwMicroSecPerFrame*m_avih.dwTotalFrames;

	return(t);
}

HRESULT CAviFile::BuildIndex()
{
	EmptyIndex();

	int nSuperIndexes = 0;

	for(int i = 0; i < (int)m_avih.dwStreams; i++)
	{
		strm_t* s = m_strms[i];
		if(s->indx && s->indx->nEntriesInUse > 0) nSuperIndexes++;
	}

	if(nSuperIndexes == m_avih.dwStreams)
	{
		for(int i = 0; i < (int)m_avih.dwStreams; i++)
		{
			strm_t* s = m_strms[i];

			AVISUPERINDEX* idx = (AVISUPERINDEX*)s->indx;

			DWORD nEntriesInUse = 0;

			for(int j = 0; j < (int)idx->nEntriesInUse; j++)
			{
				Seek(idx->aIndex[j].qwOffset);

				AVISTDINDEX stdidx;
				if(S_OK != ByteRead((BYTE*)&stdidx, FIELD_OFFSET(AVISTDINDEX, aIndex)))
				{
					EmptyIndex();
					return E_FAIL;
				}

				nEntriesInUse += stdidx.nEntriesInUse;
			} 

			s->cs.SetCount(nEntriesInUse);

			DWORD frame = 0;
			UINT64 size = 0;

			for(int j = 0; j < (int)idx->nEntriesInUse; j++)
			{
				Seek(idx->aIndex[j].qwOffset);

				CAutoPtr<AVISTDINDEX> p((AVISTDINDEX*)new BYTE[idx->aIndex[j].dwSize]);
				if(!p || S_OK != ByteRead((BYTE*)(AVISTDINDEX*)p, idx->aIndex[j].dwSize)) 
				{
					EmptyIndex();
					return E_FAIL;
				}

				for(int k = 0, l = 0; k < (int)p->nEntriesInUse; k++)
				{
					s->cs[frame].size = size;
					s->cs[frame].filepos = p->qwBaseOffset + p->aIndex[k].dwOffset;
					s->cs[frame].fKeyFrame = !(p->aIndex[k].dwSize&AVISTDINDEX_DELTAFRAME) 
						|| s->strh.fccType == FCC('auds');
					s->cs[frame].fChunkHdr = false;
					s->cs[frame].orgsize = p->aIndex[k].dwSize&AVISTDINDEX_SIZEMASK;

					if(m_idx1)
					{
						s->cs[frame].filepos -= 8;
						s->cs[frame].fChunkHdr = true;
					}

					frame++;
					size += s->GetChunkSize(p->aIndex[k].dwSize&AVISTDINDEX_SIZEMASK);
				}
			}

			s->totalsize = size;
		}
	}
	else if(AVIOLDINDEX* idx = m_idx1)
	{
		int len = idx->cb/sizeof(idx->aIndex[0]);

		UINT64 offset = m_movis.GetHead() + 8;

		for(int i = 0; i < (int)m_avih.dwStreams; i++)
		{
			strm_t* s = m_strms[i];

			int nFrames = 0;

			for(int j = 0; j < len; j++)
			{
				if(TRACKNUM(idx->aIndex[j].dwChunkId) == i)
					nFrames++;
			}

			s->cs.SetCount(nFrames);

			DWORD frame = 0;
			UINT64 size = 0;

			for(int j = 0, k = 0; j < len; j++)
			{
				DWORD TrackNumber = TRACKNUM(idx->aIndex[j].dwChunkId);

				if(TrackNumber == i)
				{
					if(j == 0 && idx->aIndex[j].dwOffset > offset)
					{
						DWORD id;
						Seek(offset + idx->aIndex[j].dwOffset);
						SVP_LogMsg5(L"m_idx1 %x %x %d %d %d %x" ,id ,  idx->aIndex[j].dwChunkId , offset + idx->aIndex[j].dwOffset , j , len ,TrackNumber);
						Read(id);
						if(id != idx->aIndex[j].dwChunkId)
						{
							SVP_LogMsg5(_T("WARNING: CAviFile::Init() detected absolute chunk addressing in \'idx1\' %x %x %d %d %d ")
								,id ,  idx->aIndex[j].dwChunkId , offset + idx->aIndex[j].dwOffset , j , len);
							offset = 0;
							/*
							offset = -8;
							
							Seek( idx->aIndex[j].dwOffset);
							do{
								offset+=8;
								if(offset > 10240){
										SVP_LogMsg5(L"m_idx1 nofound %x", id);
									offset = 0;
								}
								Read(id);
							}while(id != idx->aIndex[j].dwChunkId)	;
							SVP_LogMsg5(L"m_idx1 %d"  , offset);
							*/
							//EmptyIndex();
							//return E_ABORT;
							//continue;
							
						}
					}

					s->cs[frame].size = size;
					s->cs[frame].filepos = offset + idx->aIndex[j].dwOffset;
					s->cs[frame].fKeyFrame = !!(idx->aIndex[j].dwFlags&AVIIF_KEYFRAME) 
						|| s->strh.fccType == FCC('auds') // FIXME: some audio index is without any kf flag
						|| frame == 0; // grrr
					s->cs[frame].fChunkHdr = j == len-1 || idx->aIndex[j].dwOffset != idx->aIndex[j+1].dwOffset;
					s->cs[frame].orgsize = idx->aIndex[j].dwSize;

					//SVP_LogMsg5(L"m_idx1 TrackNumber %d %d %d %d %d %d " , TrackNumber ,s->cs[frame].fKeyFrame, s->cs[frame].size , s->cs[frame].filepos, s->cs[frame].fChunkHdr,	s->cs[frame].orgsize);
					frame++;
					size += s->GetChunkSize(idx->aIndex[j].dwSize);
				}
			}

			s->totalsize = size;
		}
	}

	m_idx1.Free();
	for(int i = 0; i < (int)m_avih.dwStreams; i++)
		m_strms[i]->indx.Free();

	return S_OK;
}

void CAviFile::EmptyIndex()
{
	for(int i = 0; i < (int)m_avih.dwStreams; i++)
	{
		strm_t* s = m_strms[i];
		s->cs.RemoveAll();
		s->totalsize = 0;
	}
}

bool CAviFile::IsInterleaved(bool fKeepInfo)
{
	if(m_strms.GetCount() < 2)
		return(true);
/*
	if(m_avih.dwFlags&AVIF_ISINTERLEAVED) // not reliable, nandub can write f*cked up files and still sets it
		return(true);
*/
	for(int i = 0; i < (int)m_avih.dwStreams; i++)
		m_strms[i]->cs2.SetCount(m_strms[i]->cs.GetCount());

	DWORD* curchunks = new DWORD[m_avih.dwStreams];
	UINT64* cursizes = new UINT64[m_avih.dwStreams];

	memset(curchunks, 0, sizeof(DWORD)*m_avih.dwStreams);
	memset(cursizes, 0, sizeof(UINT64)*m_avih.dwStreams);

	int end = 0;

	while(1)
	{
		UINT64 fpmin = _I64_MAX;

		DWORD n = -1;
		for(int i = 0; i < (int)m_avih.dwStreams; i++)
		{
			int curchunk = curchunks[i];
			CAtlArray<strm_t::chunk>& cs = m_strms[i]->cs;
			if(curchunk >= cs.GetCount()) continue;
            UINT64 fp = cs[curchunk].filepos;
			if(fp < fpmin) {fpmin = fp; n = i;}
		}
		if(n == -1) break;

		strm_t* s = m_strms[n];
		DWORD& curchunk = curchunks[n];
		UINT64& cursize = cursizes[n];

		if(!s->IsRawSubtitleStream())
		{
			strm_t::chunk2& cs2 = s->cs2[curchunk];
			cs2.t = (DWORD)(s->GetRefTime(curchunk, cursize)>>13); // for comparing later it is just as good as /10000 to get a near [ms] accuracy
//			cs2.t = (DWORD)(s->GetRefTime(curchunk, cursize)/10000);
			cs2.n = end++;
		}

		cursize = s->cs[curchunk].size;
		curchunk++;
	}

	memset(curchunks, 0, sizeof(DWORD)*m_avih.dwStreams);

	strm_t::chunk2 cs2last = {-1, 0};

	bool fInterleaved = true;

	while(fInterleaved)
	{
		strm_t::chunk2 cs2min = {LONG_MAX, LONG_MAX};

		int n = -1;
		for(int i = 0; i < (int)m_avih.dwStreams; i++)
		{
			int curchunk = curchunks[i];
			if(curchunk >= m_strms[i]->cs2.GetCount()) continue;
			strm_t::chunk2& cs2 = m_strms[i]->cs2[curchunk];
			if(cs2.t < cs2min.t) {cs2min = cs2; n = i;}
		}
		if(n == -1) break;

		curchunks[n]++;

		if(cs2last.t >= 0 && abs((int)(cs2min.n - cs2last.n)) >= 1000)
			fInterleaved = false;

		cs2last = cs2min;
	}

	delete [] curchunks;
	delete [] cursizes;

	if(fInterleaved && !fKeepInfo)
	{
		// this is not needed anymore, let's save a little memory then
		for(int i = 0; i < (int)m_avih.dwStreams; i++)
			m_strms[i]->cs2.RemoveAll();
	}

	return(fInterleaved);
}

REFERENCE_TIME CAviFile::strm_t::GetRefTime(DWORD frame, UINT64 size)
{
	float dframe = frame;

	if(strh.fccType == FCC('auds'))
	{
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)strf.GetData();

		dframe = wfe->nBlockAlign ? 1.0f * size / wfe->nBlockAlign : 0;
	}

	float scale_per_rate = strh.dwRate ? 1.0f * strh.dwScale / strh.dwRate : 0;

	return (REFERENCE_TIME)(scale_per_rate * dframe * 10000000 + 0.5f);
}

int CAviFile::strm_t::GetFrame(REFERENCE_TIME rt)
{
	int frame = -1;

	float rate_per_scale = strh.dwScale ? 1.0f * strh.dwRate / strh.dwScale : 0;

	if(strh.fccType == FCC('auds'))
	{
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)strf.GetData();

		__int64 size = (__int64)(rate_per_scale * wfe->nBlockAlign * rt / 10000000 + 0.5f);

		for(frame = 0; frame < cs.GetCount(); frame++)
		{
			if(cs[frame].size > size)
			{
				frame--;
				break;
			}
		}
	}
	else
	{
		frame = (int)(rate_per_scale * rt / 10000000 + 0.5f);
	}

	if(frame >= cs.GetCount()) frame = cs.GetCount()-1;

	return frame;
}

int CAviFile::strm_t::GetKeyFrame(REFERENCE_TIME rt)
{
	int i = GetFrame(rt);
	for(; i > 0; i--) {if(cs[i].fKeyFrame) break;}
	return(i);
}

DWORD CAviFile::strm_t::GetChunkSize(DWORD size)
{
	if(strh.fccType == FCC('auds'))
	{
		WORD nBlockAlign = ((WAVEFORMATEX*)strf.GetData())->nBlockAlign;
		size = nBlockAlign ? (size + (nBlockAlign-1)) / nBlockAlign * nBlockAlign : 0; // round up for nando's vbr hack
	}

	return size;
}

bool CAviFile::strm_t::IsRawSubtitleStream()
{
	return strn.Find("Subtitle") == 0 || (strh.fccType == FCC('txts') && cs.GetCount() == 1);
}
