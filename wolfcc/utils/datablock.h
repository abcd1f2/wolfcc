#ifndef __WOLFCC_DATABLOCK_H__
#define __WOLFCC_DATABLOCK_H__
#include <sys/types.h>
#include "utils/allocator.h"

class DataBlock
{
public:
    enum { DEFAULT_BLOCK_SIZE = 1024 };

public:
    typedef char * Iterator;
    typedef const char * ConstIterator;

public:
    DataBlock(size_t iSize = DEFAULT_BLOCK_SIZE, Allocator *pAllocator = Allocator::Instance());
    DataBlock(const DataBlock &);
    ~DataBlock();
    DataBlock & operator=(const DataBlock &);

    void Swap(DataBlock &);
    int Resize(size_t iSize);
    inline void Clear() { _pEnd = _pBegin; }
    int IncEnd(size_t iSize);
    int DecEnd(size_t iSize);

    int Copy(Iterator pDestBuffer, ConstIterator pSrcBuffer, size_t iSize);
    inline int Append(ConstIterator pSrcBuffer, size_t iSize) { return Copy(GetEnd(), pSrcBuffer, iSize); }
    int Erase(Iterator pBegin, Iterator pEnd);

    inline Iterator GetBegin() { return _pBegin; }
    inline Iterator GetEnd() { return _pEnd; }
    inline ConstIterator GetBegin() const { return _pBegin; }
    inline ConstIterator GetEnd() const { return _pEnd; }

    inline bool IsInit() const { return GetBegin() != NULL; }
    inline bool IsEmpty() const { return GetBegin() == GetEnd(); }
    inline bool IsFull() const { return GetEnd() == _pEndOfBlock; }

    inline size_t GetSize() const  { return GetEnd() - GetBegin(); }
    inline size_t GetLength() const  { return GetSize(); }
    inline size_t GetAvailable() const { return _pEndOfBlock - GetEnd(); }
    inline size_t GetCapacity() const  { return _pEndOfBlock - GetBegin(); }

public:
    Allocator * _pAllocator;
    Iterator _pBegin;
    Iterator _pEnd;
    Iterator _pEndOfBlock;
};

#endif
