#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>

class BlockUnit;

namespace MCBlockModel {

// !****************************************************************************
// Abstract class for model
// *****************************************************************************
template<typename _DataType, typename _Container = std::list<_DataType* > >
class abstract_model
{
public:
  typedef _Container DataContainer;
  typedef _Container * DataPtr;
  typedef typename _Container::iterator * DataIteratorPtr;

public:
  virtual void set_data(DataPtr ptr, DataIteratorPtr iteratorPtr) = 0;
  virtual int insert(_DataType *pData) = 0;
};

// !****************************************************************************
// Sort block unit by added time
// *****************************************************************************
class added_time_sort_model : public abstract_model<BlockUnit>
{
public:
  added_time_sort_model() : m_ptrData(0), m_ptrIterator(0) {}

public:
  virtual void set_data(DataPtr ptr, DataIteratorPtr iteratorPtr);
  virtual int insert(BlockUnit *pBlockUnit);

private:
  DataPtr m_ptrData;
  DataIteratorPtr m_ptrIterator;
};

// !****************************************************************************
// Sort block unit by filename
// *****************************************************************************
class filename_sort_model : public abstract_model<BlockUnit>
{
public:
  filename_sort_model() : m_ptrData(0) {}

public:
  virtual void set_data(DataPtr ptr, DataIteratorPtr iteratorPtr);
  virtual int insert(BlockUnit *pBlockUnit);

private:
  DataPtr m_ptrData;
  DataIteratorPtr m_ptrIterator;
};
}  // namespace MCBlockModel