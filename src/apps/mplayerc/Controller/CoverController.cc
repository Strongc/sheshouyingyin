#include "stdafx.h"
#include "CoverController.h"
#include "HashController.h"
#include "SVPToolBox.h"
#include "Strings.h"
#include "json\json.h"
#include "logging.h"
#include "MediaCenterController.h"
#include "..\Model\MediaDB.h"

////////////////////////////////////////////////////////////////////////////////
// Normal part
CoverController::CoverController()
{
  // create folder if MC folder is not existed
  MediaCenterController::CreateMCFolder();
}

CoverController::~CoverController()
{

}

////////////////////////////////////////////////////////////////////////////////
// Properties
void CoverController::SetBlockUnit(const MediaData &md)
{
  m_list.push_back(md);
}

////////////////////////////////////////////////////////////////////////////////
// Operations
void CoverController::_Thread()
{
  using namespace boost::filesystem;

  std::wstring requesturl = L"http://m.shooter.cn/api/medias/getinfoBysphash/sphash:";
  std::wstring downloadurl = L"http://img.shooter.cn/";
  std::list<MediaData>::iterator it;

  while (true)
  {
    // see if need to be stop
    if (_Exit_state(0))
      return;

    // download cover or make a snapshot for media
    it = m_list.begin();
    if (it != m_list.end())
    {
      // common
      std::wstring szFilePath = it->path + it->filename; 
      std::string szFileHash = Strings::WStringToUtf8String(HashController::GetInstance()->GetSPHash(szFilePath.c_str()));

      // if already has thumbnail then continue
      std::wstring thumbnailpath = it->thumbnailpath;
      if (exists(thumbnailpath) && !is_directory(thumbnailpath))
      {
        //UploadCover(*it);
        m_list.pop_front();
        continue;
      }

      thumbnailpath = MediaCenterController::GetCoverPath(it->path + it->filename);
      if (exists(thumbnailpath) && !is_directory(thumbnailpath))
      {
        //UploadCover(*it);
        m_list.pop_front();
        continue;
      }

      //url = L"http://jay.webpj.com:8888/api/medias/getinfoBysphash/sphash: \
      25026521a390357bd1fcf52899268c97;c0c3ddd9b5a1c1d292131a91c9200648;cb72fdc1ff58dfc5cda9943e098c304b;e7752a7553168a73f29b0e36f09a86a8";

      // Get http response
      std::string results("");
      BOOL bGet = HttpGetResponse(szFileHash, szFilePath, requesturl, results);
      std::wstring cover;
      std::wstring coverdownloadpath;
      if (!bGet || results.empty())
      {
        // if no cover on the server, then get the snapshot by ourself
        coverdownloadpath = GetSnapshot(*it, szFileHash);
      }
      else
      {
        // Parse the respond string
        BOOL bParse = ParseRespondString(results, it->filmname, cover);
        if (!bParse || cover.empty())
        {
          // if no cover on the server, then get the snapshot by ourself
          coverdownloadpath = GetSnapshot(*it, szFileHash);
        }
        else
        {
          // Download cover
          coverdownloadpath = MediaCenterController::GetCoverPath(it->path + it->filename);
          BOOL bDownload = HttpDownloadCover(downloadurl, coverdownloadpath, cover);
          if (!bDownload)
          {
            // if no cover on the server, then get the snapshot by ourself
            coverdownloadpath = GetSnapshot(*it, szFileHash);
          }
        }
      }

      // Upload cover
      UploadCover(*it);

      // pop the front media data even if fail to get the cover
      m_list.pop_front();
    }

    // sleep for a while
    ::Sleep(300);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Helper functions
// For download
BOOL CoverController::HttpGetResponse(std::string szFileHash, std::wstring szFilePath, std::wstring requesturl, std::string& responsestr)
{
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  task->use_config(cfg);
  refptr<request> req = request::create_instance();

  requesturl += Strings::Utf8StringToWString(szFileHash);

  req->set_request_url(requesturl.c_str());
  req->set_request_method(REQ_GET);
  task->append_request(req);
  pool->execute(task);

  while (pool->is_running_or_queued(task))
  {
    if (_Exit_state(500))
      return FALSE;
  }

  if (req->get_response_errcode() != 0 )
    return FALSE;

  si_buffer buff = req->get_response_buffer();
  buff.push_back(0);
  responsestr = (char*)&buff[0];
  return TRUE;
}

BOOL CoverController::ParseRespondString(std::string& parsestr, std::wstring& title, std::wstring& cover)
{
  Json::Reader reader;
  Json::Value json_object;
  if (!reader.parse(parsestr, json_object))
    return FALSE;
  std::string coverstr = json_object["mediainfo"]["cover"].asString();

  std::string titlestr;
  const Json::Value titlevalue = json_object["mediainfo"]["title"];
  for (size_t index = 0; index < titlevalue.size(); ++index )
    titlestr = titlevalue[index]["name"].asString();

  if (!titlestr.empty())
    title = Strings::Utf8StringToWString(titlestr);
  else
    title.clear();

  if (!coverstr.empty())
    cover = Strings::Utf8StringToWString(coverstr);
  else
    cover.clear();

  return TRUE;
}

BOOL CoverController::HttpDownloadCover(std::wstring downloadurl, std::wstring& downloadpath, std::wstring cover)
{
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  task->use_config(cfg);
  refptr<request> req = request::create_instance();

  std::wstring url(downloadurl);
  url += cover.substr(cover.size() - 4, 2) + L"/";
  url += cover.substr(cover.size() -2) + L"/";
  url += cover + L"_100x0.jpg";

  req->set_request_url(url.c_str());
  req->set_request_method(REQ_GET);
  req->set_request_outmode(REQ_OUTFILE);
  req->set_outfile(downloadpath.c_str());
  task->append_request(req);
  pool->execute(task);

  while (pool->is_running_or_queued(task))
  {
    if (_Exit_state(500))
      return FALSE;
  }

  if (req->get_response_errcode() != 0 )
    return FALSE;

  if (req->get_response_size() != req->get_retrieved_size())
    return FALSE;

  return TRUE;
}

std::wstring CoverController::GetSnapshot(const MediaData &md, const std::string &szFileHash)
{
  CSVPToolBox toolbox;
  std::wstring sPlayerPath = toolbox.GetPlayerPath_STL();
  std::wstring sParameters = L" /snapshot \"" + md.path + md.filename + L"\"";

  SHELLEXECUTEINFO shExecInfo = {0};
  shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
  shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
  shExecInfo.hwnd = 0;
  shExecInfo.lpVerb = 0;
  shExecInfo.lpFile = sPlayerPath.c_str();
  shExecInfo.lpParameters = sParameters.c_str();
  shExecInfo.lpDirectory = 0;
  shExecInfo.nShow = SW_HIDE;
  shExecInfo.hInstApp = 0;
  BOOL bRet = ::ShellExecuteEx(&shExecInfo);
  if (bRet)
  {
    ::WaitForSingleObject(shExecInfo.hProcess, INFINITE);
    ::CloseHandle(shExecInfo.hProcess);
  }

  std::wstring sRet = MediaCenterController::GetCoverPath(md.path + md.filename);

  return sRet;
}

// For upload
void CoverController::UploadCover(const MediaData &md)
{
  std::wstring url = L"http://zz.webpj.com:8888/api/medias/add_sfScreenshot";

  std::wstring szFilePath = md.path + md.filename; 
  std::wstring szFileHash = HashController::GetInstance()->GetSPHash(szFilePath.c_str());

  std::wstring coverPath;
  CSVPToolBox csvptb;
  csvptb.GetAppDataPath(coverPath);
  coverPath += L"\\" + md.thumbnailpath;

  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<request> req = request::create_instance();
  refptr<postdata> pd = postdata::create_instance();

  req->set_request_url(url.c_str());
  req->set_request_method(REQ_POST);

  refptr<postdataelem> pelem1 = postdataelem::create_instance();
  pelem1->set_name(L"sphash");
  pelem1->setto_text(szFileHash.c_str());
  pd->add_elem(pelem1);

  refptr<postdataelem> pelem2 = postdataelem::create_instance();
  pelem2->set_name(L"img");
  pelem2->setto_file(coverPath.c_str());
  pd->add_elem(pelem2);

  req->set_postdata(pd);
  task->append_request(req);
  pool->execute(task);

  while (pool->is_running_or_queued(task))
  {
    if (_Exit_state(500))
      return;
  }

  if (req->get_response_errcode() != 0 )
    return;
}