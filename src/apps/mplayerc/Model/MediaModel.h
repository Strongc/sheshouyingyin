#pragma once

#include "MediaComm.h"
#include "SourceModel.h"
#include <sqlitepp/sqlitepp.hpp>

// An implement class to add, find and delete media data in the sqlite database
class MediaModel :
    public SourceModel<MediaData, MediaFindCondition>
{
public:
    MediaModel();
    ~MediaModel();
    
public:
    int GetCount();

    void Add(MediaData& mdData);
    void Add(MediaDatas& data);

    void Add(MediaPath& mdData);
    void Add(MediaPaths& data);

    void FindAll(MediaPaths& data);
    void FindAll(MediaDatas& data);

    void FindOne(MediaData& data, const MediaFindCondition& condition);
    void Find(MediaDatas& data, const MediaFindCondition& condition,
              int limit_start, int limit_end);

    void Find(int limit_start, int limit_end, MediaDatas& data);

    void Delete(const MediaFindCondition& condition);
    void DeleteAll();

    static std::wstring EscapeSQL(const std::wstring &sSQL);
};