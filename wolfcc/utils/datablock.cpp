#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <sys/types.h>
#include "utils/datablock.h"

DataBlock::DataBlock(size_t iSize, Allocator *pAllocator)
    : _pAllocator(pAllocator), _pBegin(0), _pEnd(0), _pEndOfBlock(0)
{
    size_t iRealSize = 0;
    _pBegin = (Iterator)_pAllocator->Allocate(iSize, &iRealSize);
    if (_pBegin == NULL) {
        return;
    }
    _pEnd = _pBegin;
    _pEndOfBlock = _pBegin + iRealSize;
}

DataBlock::DataBlock(const DataBlock & oDataBlock)
{
    _pAllocator = oDataBlock._pAllocator;
    size_t iCapacity = oDataBlock.GetCapacity();

    _pBegin = (Iterator)_pAllocator->Allocate(iCapacity);
    if (_pBegin == NULL) {
        return;
    }

    _pEnd = _pBegin;
    _pEndOfBlock = _pBegin + iCapacity;

    Append(oDataBlock.GetBegin(), oDataBlock.GetSize());
}

DataBlock::~DataBlock()
{
    if (_pBegin) {
        _pAllocator->Deallocate(_pBegin);
    }
}

DataBlock & DataBlock::operator=(const DataBlock & oDataBlock)
{
    DataBlock oDataBlockTmp(oDataBlock);
    this->Swap(oDataBlockTmp);
    return *this;
}

void DataBlock::Swap(DataBlock & oDataBlock)
{
    ::std::swap(_pAllocator, oDataBlock._pAllocator);
    ::std::swap(_pBegin, oDataBlock._pBegin);
    ::std::swap(_pEnd, oDataBlock._pEnd);
    ::std::swap(_pEndOfBlock, oDataBlock._pEndOfBlock);
}

int DataBlock::Resize(size_t iSize)
{
    // maybe it doesn't need to realloc
    if (iSize <= GetCapacity()) {
        if (GetBegin() + iSize < GetEnd()) {
            _pEnd = GetBegin() + iSize;
        }
        return 0;
    }

    DataBlock oDataBlockTmp(iSize, _pAllocator);
    if (!oDataBlockTmp.IsInit()) {
        return -1;
    }

    if (oDataBlockTmp.Append(GetBegin(), ::std::min(GetSize(), iSize)) != 0) {
        return -1;
    }

    this->Swap(oDataBlockTmp);
    return 0;
}


int DataBlock::Copy(Iterator pDestBuffer, ConstIterator pSrcBuffer, size_t iSize)
{
    // pDestBuffer must be in [_pBegin, _pEnd)
    ssize_t iDestOffset = pDestBuffer - GetBegin();

    if (pDestBuffer < GetBegin() || pDestBuffer > GetEnd()) {
        return -1;
    }

    ssize_t iSrcOffset = pSrcBuffer - GetBegin();
    size_t iDestSize = iDestOffset + iSize;
    ssize_t iIncSize = iDestSize - GetSize();
    if (iIncSize < 0) {
        iIncSize = 0;
    }

    if (iDestSize > GetCapacity()) {
        if (Resize(iDestSize) != 0) {
            return -1;
        }
    }

    if (iSrcOffset >= 0 && (size_t)iSrcOffset < GetSize()) {
        memmove(GetBegin() + iDestOffset, GetBegin() + iSrcOffset, iSize);
    }
    else {
        memcpy(GetBegin() + iDestOffset, pSrcBuffer, iSize);
    }

    return IncEnd(iIncSize);
}

int DataBlock::Erase(Iterator pBegin, Iterator pEnd)
{
    if (pBegin == pEnd) {
        return 0;
    }

    if (pBegin > pEnd) {
        return -1;
    }

    if (pBegin < GetBegin() || pEnd > GetEnd()) {
        return -1;
    }

    if (pBegin == GetBegin() && pEnd == GetEnd()) {
        Clear();
        return 0;
    }

    if (Copy(pBegin, pEnd, GetEnd() - pEnd) != 0) {
        return -1;
    }

    return DecEnd(pEnd - pBegin);
}

int DataBlock::IncEnd(size_t iSize)
{
    if (iSize > GetAvailable()) {
        return -1;
    }
    _pEnd += iSize;
    return 0;
}

int DataBlock::DecEnd(size_t iSize)
{
    if (iSize > GetSize()) {
        return -1;
    }

    _pEnd -= iSize;
    return 0;
}
