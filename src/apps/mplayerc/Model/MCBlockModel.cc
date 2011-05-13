#include "stdafx.h"
#include "MCBlockModel.h"
#include "../UserInterface/Renderer/BlockList.h"
#include <boost/thread.hpp>
#include <boost/lambda/bind.hpp>

using namespace boost;
using namespace boost::posix_time;
using namespace boost::lambda;
using MCBlockModel::added_time_sort_model;
using MCBlockModel::filename_sort_model;

// !****************************************************************************
// class : added_time_sort_model
// Features : sort files by its added time
// *****************************************************************************
void added_time_sort_model::set_data(DataPtr ptr, DataIteratorPtr iteratorPtr)
{
  // sort the data by added time
  m_ptrData = ptr;
  m_ptrIterator = iteratorPtr;
}

int added_time_sort_model::insert(BlockUnit *pBlockUnit)
{
  // just push back to the list
  m_ptrData->push_back(pBlockUnit);

  return -1;
}


// !****************************************************************************
// class : filename_sort_model
// Features : sort files by its filename
// *****************************************************************************
bool _helper_sort_by_filename(BlockUnit *pFirst, BlockUnit *pSecond)
{
  return pFirst->m_mediadata.filename < pSecond->m_mediadata.filename;
}

void filename_sort_model::set_data(DataPtr ptr, DataIteratorPtr iteratorPtr)
{
  // sort the data by filename
  m_ptrData = ptr;
  m_ptrData->sort(_helper_sort_by_filename);
  m_ptrIterator = iteratorPtr;
}

bool _helper_find_by_filename(BlockUnit *pCur, BlockUnit *pToCompare)
{
  if (pCur->m_mediadata.filename <= pToCompare->m_mediadata.filename)
    return true;
  else
    return false;
}

int filename_sort_model::insert(BlockUnit *pBlockUnit)
{
  // find the first element by filename
  //DataContainer::iterator itBegin = m_ptrData->begin();
  DataContainer::iterator itFind;
  itFind = std::find_if(m_ptrData->begin(), m_ptrData->end(), 
                        bind(&_helper_find_by_filename, pBlockUnit, lambda::_1));
  DataContainer::iterator itInsert = m_ptrData->insert(itFind, pBlockUnit);

  // adjust the start iterator
  int nStart = 0;
  std::list<BlockUnit*>::iterator itTemp = m_ptrData->begin();
  while (itTemp != *m_ptrIterator)
  {
    ++nStart;
    ++itTemp;
  }

  int nInsert = 0;
  itTemp = m_ptrData->begin();
  while (itTemp != itInsert)
  {
    ++nInsert;
    ++itTemp;
  }

  if (nInsert < nStart)
  {
    (*m_ptrIterator)--;
  }


  return -1;
}