#pragma once

#include <threadhelper.h>
#include "NetworkControlerImpl.h"
#include "../Model/MediaComm.h"

class CoverController
: public ThreadHelperImpl<CoverController>
, public NetworkControlerImpl
{
/* Normal part */
public:
  CoverController();
  ~CoverController();

/* Properties */
public:
  void SetBlockUnit(const MediaData &md);

/* Operations */
public:
  void _Thread();

/* Helper functions */
protected:
  // For download
  BOOL HttpGetResponse(std::string szFileHash, std::wstring szFilePath, std::wstring requesturl, std::string& responsestr);
  BOOL ParseRespondString(std::string& parsestr, std::wstring& title, std::wstring& cover);
  BOOL HttpDownloadCover(std::wstring downloadurl, std::wstring& downloadpath, std::wstring cover);
  std::wstring GetSnapshot(const MediaData &md, const std::string &szFileHash);

  // For upload
  void UploadCover(const MediaData &md);

/* Private */
private:
  std::list<MediaData> m_list;  // store each media's media data
};