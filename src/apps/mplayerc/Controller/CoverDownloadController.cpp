#include "stdafx.h"
#include "CoverDownloadController.h"
#include "HashController.h"
#include "SVPToolBox.h"
#include "Strings.h"
#include "bson.h"
#include "logging.h"

void CoverDownloadController::SetBlockUnit(BlockUnit* unit)
{
  m_list.push_back(unit);
}

void CoverDownloadController::_Thread()
{
   std::list<BlockUnit*>::iterator it = m_list.begin();
   while (it != m_list.end())
   {
    refptr<pool> pool = pool::create_instance();
    refptr<task> task = task::create_instance();
    refptr<config> cfg = config::create_instance();
    refptr<request> req = request::create_instance();
    task->use_config(cfg);

    std::wstring szFilePath = (*it)->m_data.path + (*it)->m_data.filename; 
    std::wstring szFileHash = HashController::GetInstance()->GetSPHash(szFilePath.c_str());
    std::wstring url = L"http://m.shooter.cn/api/medias/getinfoBysphash/sphash:";
    url += szFileHash;
//     std::wstring url = L"http://jay.webpj.com:8888/api/medias/getinfoBysphash/sphash: \
//                        25026521a390357bd1fcf52899268c97;c0c3ddd9b5a1c1d292131a91c9200648;cb72fdc1ff58dfc5cda9943e098c304b;e7752a7553168a73f29b0e36f09a86a8";

    req->set_request_url(url.c_str());
    req->set_request_method(REQ_GET);
    task->append_request(req);
    pool->execute(task);

    while (pool->is_running_or_queued(task))
    {
      if (_Exit_state(500))
        return;
    }

    if (req->get_response_errcode() != 0 )
      return;

    si_buffer buff = req->get_response_buffer();
    buff.push_back(0);
    std::string results = (char*)&buff[0];
    
    
    // parse results
    //std::wstring cover = L"4cfeb4ab4370be6a0f62b117";
    //std::wstring url = L"http://img.shooter.cn/b1//17//4cfeb4ab4370be6a0f62b117_100x0.jpg";

    bson::Reader reader;
    Json::Value json_object;
    if (!reader.parse(results, json_object))
      return;
    std::string coverstr = json_object["mediainfo"]["cover"].asString();
    std::wstring cover;
    if (!coverstr.empty())
    {
      std::string str1 = bobject["mediainfo"]["title"];
      (*it)->m_data.filmname = Strings::Utf8StringToWString(str1);
      cover = Strings::Utf8StringToWString(coverstr);
    }  
    else
      return;
    
    
    url.clear();
    std::wstring url = L"http://img.shooter.cn/";
    url += cover.substr(cover.size() - 4, 2) + L"/";
    url += cover.substr(cover.size() -2) + L"/";
    url += cover + L"_100x0.jpg";
    
    std::wstring downloadpath;//(L"C:\\Users\\Staff\\AppData\Roaming\\SPlayer\\MC\\4cfeb4ab4370be6a0f62b117.jpg");
    CSVPToolBox csvptb;
    csvptb.GetAppDataPath(downloadpath);
    downloadpath += L"\\MC\\";
    downloadpath += cover + L".jpg";

    refptr<request> req0 = request::create_instance();
    req0->set_request_url(url.c_str());
    req0->set_request_method(REQ_GET);
    req0->set_request_outmode(REQ_OUTFILE);
    req0->set_outfile(downloadpath.c_str());
    task->append_request(req0);
    pool->execute(task);

    while (pool->is_running_or_queued(task))
    {
      if (_Exit_state(500))
        return;
    }

    if (req0->get_response_errcode() != 0 )
      return;

    Logging("-----------------ok");

    
    (*it)->ChangeLayer(cover + L".jpg");
    m_list.pop_front();
    it = m_list.begin();
   }
}

void CoverDownloadController::_Thread_old()
{
  std::list<BlockUnit*>::iterator it = m_list.begin();
  while (it != m_list.end())
  {
    refptr<pool> pool = pool::create_instance();
    refptr<task> task = task::create_instance();
    refptr<config> cfg = config::create_instance();
    refptr<request> req = request::create_instance();
    task->use_config(cfg);
    
    std::wstring szFilePath = (*it)->m_data.path + (*it)->m_data.filename; 
    //std::wstring szFileHash = HashController::GetInstance()->GetSPHash(szFilePath.c_str());
    std::wstring szFileHash = L"25026521a390357bd1fcf52899268c97;c0c3ddd9b5a1c1d292131a91c9200648;cb72fdc1ff58dfc5cda9943e098c304b;e7752a7553168a73f29b0e36f09a86a8";
    std::wstring url = L"http://m.shooter.cn/api/medias/getinfoBysphash/sphash:";
    url += szFileHash;
    url = L"http://jay.webpj.com:8888/api/medias/getinfoBysphash/sphash:25026521a390357bd1fcf52899268c97;c0c3ddd9b5a1c1d292131a91c9200648;cb72fdc1ff58dfc5cda9943e098c304b;e7752a7553168a73f29b0e36f09a86a8";
    Logging(L"--------------coverdownloadfile:%s", url.c_str());
  
    req->set_request_url(url.c_str());
    req->set_request_method(REQ_GET);
    task->append_request(req);
    pool->execute(task);

    while (pool->is_running_or_queued(task))
    {
      if (_Exit_state(500))
        return;
    }

    si_buffer buff = req->get_response_buffer();
    buff.push_back(0);
    std::string results = (char*)&buff[0];
    //std::string results = Utf8StringToWString(buffstr);
    Logging("000 ---- 0000 %s", results.c_str());
    //Logging("000 ---- 0000 %s, %d", results.c_str(), buff.size());
    //std::string results = "{\"mediainfo\":{\"cover\":\"4cfeb4ab4370be6a0f62b117\",\"title\":\"\\u8001\\u4eba\\u4e0e\\u6d77-\\u5965\\u65af\\u5361\\u83b7\\u5956\\u52a8\\u753b\"}}";
//     bson::Reader reader;
//     Json::Value json_object;
//     if (!reader.parse(results, json_object))
//     {
//       Logging("-----------------------3");
//       return;
//     }
    results = "{\"mediainfo\":{\"cover\":\"4cfeb4ab4370be6a0f62b117\",\"title\":\"\\u8001\\u4eba\\u4e0e\\u6d77-\\u5965\\u65af\\u5361\\u83b7\\u5956\\u52a8\\u753b\"}}";
    mongo::BSONObj  bobject(results.c_str());
    return;
  
//     std::string coverstr = json_object["mediainfo"]["cover"].asString();
//     std::wstring cover;
//     Logging("--------------%s", coverstr.c_str());
//     if (!coverstr.empty())
//     {
//       std::string str1 = json_object["mediainfo"]["title"].asString();
//       (*it)->m_data.filmname = Strings::Utf8StringToWString(str1);
//       cover = Strings::Utf8StringToWString(coverstr);
//     }
//     else
//     {
//       Logging("----------------------4");
//       return;
//     }

    std::string coverstr = bobject["mediainfo"]["cover"];
    Logging("-------------coverstring:%s", coverstr.c_str());
    return;
    std::wstring cover;
    if (!coverstr.empty())
    {
      std::string str1 = bobject["mediainfo"]["title"];
      (*it)->m_data.filmname = Strings::Utf8StringToWString(str1);
      cover = Strings::Utf8StringToWString(coverstr);
    }  
    else
    {
      Logging("----------------------4");
      return;
    }
    Logging(L"------------coverstring:%s, %s", (*it)->m_data.filmname.c_str(), cover.c_str());
    return;
    url.clear();
    url = L"http://img.shooter.cn/";
    url += cover.substr(cover.size() - 4, 2) + L"//";
    url += cover.substr(cover.size() -2) + L"//";
    url += cover + L"_100x0.jpg";
    Logging(L"--------------coverdownloadparse:%s", url.c_str());
    std::wstring downloadpath;
    CSVPToolBox csvptb;
    csvptb.GetAppDataPath(downloadpath);
    downloadpath += L"\\MC\\";
    downloadpath += cover + L".jpg";
    refptr<request> req0 = request::create_instance();
    req0->set_request_url(url.c_str());
    req0->set_request_method(REQ_GET);
    req0->set_request_outmode(REQ_OUTFILE);
    req->set_outfile(downloadpath.c_str());
    task->append_request(req);
    pool->execute(task);
    Logging(L"--------------coverdownloaddown:%s", downloadpath.c_str());
    while (pool->is_running_or_queued(task))
    {
      if (_Exit_state(5000))
        if (req->get_response_errcode() == 0)
          break;
        else
          return;
    }
    if (req->get_response_errcode() != 0)
      return;
    Logging("----------------------------ok");
  
    (*it)->ChangeLayer(downloadpath);

    m_list.pop_front();
    it = m_list.begin();
  }
}