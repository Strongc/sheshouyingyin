// Copyright (C) 2003-2006 Gabest.
// http://www.gabest.org
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html

#include ".\reader\CDDAReader\CDDAReader.h"
#include ".\reader\CDXAReader\CDXAReader.h"
#include ".\reader\UDPReader\UDPReader.h"
#include ".\reader\VTSReader\VTSReader.h"
//#include ".\source\RarSource\RarSource.h"
#include ".\source\D2VSource\D2VSource.h"
#include ".\source\DTSAC3Source\DTSAC3Source.h"
#include ".\source\FLICSource\FLICSource.h"
#include ".\source\FLACSource\FLACSource.h"
#include ".\source\ShoutcastSource\ShoutcastSource.h"
#include ".\source\SubtitleSource\SubtitleSource.h"
#include ".\switcher\AudioSwitcher\AudioSwitcher.h"
#include ".\transform\avi2ac3filter\AVI2AC3Filter.h"
#include ".\transform\BufferFilter\BufferFilter.h"
#include ".\transform\DeCSSFilter\DeCSSFilter.h"
#include ".\transform\Mpeg2DecFilter\Mpeg2DecFilter.h"
#include ".\transform\MpaDecFilter\MpaDecFilter.h"
#include ".\transform\mpcvideodec\MPCVideoDecFilter.h"
//#include ".\transform\webm\vp8decoder\vp8decoderfilter.hpp"
#include ".\muxer\wavdest\wavdest.h"
#include ".\muxer\DSMMuxer\DSMMuxer.h"
#include ".\muxer\MatroskaMuxer\MatroskaMuxer.h"
#include ".\parser\StreamDriveThru\StreamDriveThru.h"
#include ".\parser\MatroskaSplitter\MatroskaSplitter.h"
#include ".\parser\RealMediaSplitter\RealMediaSplitter.h"
#include ".\parser\AviSplitter\AviSplitter.h"
//#include ".\parser\RadGtSplitter\RadGtSplitter.h"
#include ".\parser\RoQSplitter\RoQSplitter.h"
#include ".\parser\OggSplitter\OggSplitter.h"
#include ".\parser\NutSplitter\NutSplitter.h"
#include ".\parser\MpegSplitter\MpegSplitter.h"
#include ".\parser\DiracSplitter\DiracSplitter.h"
#include ".\parser\MpaSplitter\MpaSplitter.h"
#include ".\parser\DSMSplitter\DSMSplitter.h"
#include ".\parser\MP4Splitter\MP4Splitter.h"
#include ".\parser\FLVSplitter\FLVSplitter.h"
#include ".\parser\WMVSpliter\WMVSpliter.h"
#include ".\parser\SSFSplitter\SSFSplitter.h"

//#include ".\transform\vsfilter\DirectVobSubFilter.h"
#include ".\renderer\MpcAudioRenderer\MpcAudioRenderer.h"


#include "..\..\Thirdparty\wavpack-ds-wavpack-directshow\wavpack\wputils.h"
#include "..\..\Thirdparty\wavpack-ds-wavpack-directshow\wavpacklib\wavpack_common.h"
#include "..\..\Thirdparty\wavpack-ds-wavpack-directshow\wavpacklib\wavpack_frame.h"
#include "..\..\Thirdparty\wavpack-ds-wavpack-directshow\wavpacklib\wavpack_parser.h"
#include "..\..\Thirdparty\wavpack-ds-wavpack-directshow\wavpacklib\wavpack_buffer_decoder.h"
#include "..\..\Thirdparty\wavpack-ds-wavpack-directshow\WavPackDS_GUID.h"
#include "..\..\Thirdparty\wavpack-ds-wavpack-directshow\WavPackDSSplitter\WavPackDSSplitter.h"
#include "..\..\Thirdparty\wavpack-ds-wavpack-directshow\WavPackDSDecoder\IWavPackDSDecoder.h"
#include "..\..\Thirdparty\wavpack-ds-wavpack-directshow\WavPackDSDecoder\WavPackDSDecoder.h"
//Unfinished
//#include ".\parser\EASplitter\EASpliter.h"