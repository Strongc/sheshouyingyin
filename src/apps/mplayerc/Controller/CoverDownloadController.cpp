#include "stdafx.h"
#include "CoverDownloadController.h"
#include "HashController.h"
#include "SVPToolBox.h"
#include "Strings.h"
#include "json\json.h"
#include "logging.h"
#include "MediaCenterController.h"

CoverDownloadController::CoverDownloadController()
{
  // create folder if MC folder is not existed
  MediaCenterController::CreateMCFolder();
}

CoverDownloadController::~CoverDownloadController()
{

}

void CoverDownloadController::SetBlockUnit(BlockUnit* unit)
{
  m_list.push_back(unit);
}

void CoverDownloadController::SetFrame(HWND hwnd)
{
  m_hwnd = hwnd;
}

void CoverDownloadController::_Thread()
{
   // sinet
   std::wstring requesturl = L"http://m.shooter.cn/api/medias/getinfoBysphash/sphash:";
   std::wstring downloadurl = L"http://img.shooter.cn/";
   refptr<pool> pool = pool::create_instance();
   refptr<task> task = task::create_instance();
   refptr<config> cfg = config::create_instance();
   task->use_config(cfg);
   std::list<BlockUnit*>::iterator it = m_list.begin();
   while (it != m_list.end())
   {
     std::wstring thumbnailpath = (*it)->m_itFile->file_data.thumbnailpath;
     if (!thumbnailpath.empty() &&
        (GetFileAttributes(thumbnailpath.c_str()) != INVALID_FILE_ATTRIBUTES || 
         GetLastError() != ERROR_FILE_NOT_FOUND))
       continue;
     
     //url = L"http://jay.webpj.com:8888/api/medias/getinfoBysphash/sphash: \
     25026521a390357bd1fcf52899268c97;c0c3ddd9b5a1c1d292131a91c9200648;cb72fdc1ff58dfc5cda9943e098c304b;e7752a7553168a73f29b0e36f09a86a8";
     
     // Get http response
     std::wstring szFilePath = (*it)->m_itFile->file_data.path + (*it)->m_itFile->file_data.filename; 
     std::string results("");
     BOOL bGet = HttpGetResponse(requesturl, szFilePath, results);
     if (!bGet)
       continue;
     if (results.empty())
       continue;
     
     // Parse the respond string
     std::wstring cover;
     BOOL bParse = ParseRespondString(results, (*it)->m_itFile->file_data.filmname, cover);
     if (!bParse)
       continue;
     if (cover.empty())
       continue;
    
     // Download cover
     std::wstring coverdownloadpath;
     BOOL bDownload = HttpDownloadCover(downloadurl, coverdownloadpath, cover);
     if (!bDownload)
       continue;
    
     // Change cover
     cover = coverdownloadpath.substr(coverdownloadpath.find(L"mc"));
     (*it)->m_itFile->file_data.thumbnailpath = cover;
     (*it)->ResetCover();
     InvalidateRect(m_hwnd, &((*it)->GetHittest()), FALSE);
     
     m_list.pop_front();
     it = m_list.begin();
   }
}

BOOL CoverDownloadController::HttpGetResponse(std::wstring szFilePath, std::wstring requesturl, 
                                              std::string& responsestr)
{
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  task->use_config(cfg);
  refptr<request> req = request::create_instance();
 
  std::wstring szFileHash = HashController::GetInstance()->GetSPHash(szFilePath.c_str());
  requesturl += szFileHash;

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

BOOL CoverDownloadController::ParseRespondString(std::string& parsestr, std::wstring& title, std::wstring& cover)
{
  Json::Reader reader;
  Json::Value json_object;
  if (!reader.parse(parsestr, json_object))
    return FALSE;
  std::string coverstr = json_object["mediainfo"]["cover"].asString();

  std::string titlestr;
  const Json::Value titlevalue = json_object["mediainfo"]["title"];
  for ( int index = 0; index < titlevalue.size(); ++index )
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

BOOL CoverDownloadController::HttpDownloadCover(std::wstring downloadurl, std::wstring& downloadpath, 
                                                std::wstring cover)
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

  CSVPToolBox csvptb;
  csvptb.GetAppDataPath(downloadpath);
  downloadpath += L"\\mc\\cover\\";
  downloadpath += GetSystemTimeString() + L".jpg";

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

std::wstring CoverDownloadController::GetSystemTimeString()
{
  SYSTEMTIME time;
  GetSystemTime(&time);

  wchar_t* buff = new wchar_t[1024];
  wsprintf(buff, L"%d%d%d%d%d%d%d", time.wYear, time.wMonth, time.wDay, time.wHour,
    time.wMinute, time.wSecond, time.wMilliseconds);

  std::wstring timestr = buff;
  delete[] buff;
  return timestr;
}