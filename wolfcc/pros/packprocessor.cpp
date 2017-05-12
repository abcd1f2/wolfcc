#include <cassert>
#include <string.h>
#include <new>
#include "pros/package.h"
#include "utils/datablock.h"

Package::Package()
    : allocator(0), handlerid(0), processorid(0), protocol(0), retcode(0), data(0)
{
    
}

Package::~Package()
{
    if (data) {
        data->~DataBlock();
        allocator->Deallocate(data);
        data = 0;
    }
}


const char* Package::GetData() const {
    return data->GetBegin();
}

size_t Package::GetDataLen() const {
    return data->GetSize();
}


void Package::SetAllocator(Allocator* allocator)
{
    assert(allocator);
    this->allocator = allocator;
}

void Package::SetPeerAddr(const SockAddr& addr)
{
    peerAddr = addr;
}

const SockAddr& Package::GetPeerAddr() const
{
    return peerAddr;
}

void Package::SetData(const char* ptr, size_t len)
{
    if (!data)
    {
        void* p = allocator->Allocate(sizeof(DataBlock));
        assert(p);
        data = new (p)DataBlock(DataBlock::DEFAULT_BLOCK_SIZE, allocator);
    }
    data->Clear();

    data->Resize(len);

    data->Copy(data->GetBegin(), ptr, len);
}

void Package::Serialize(char* buf, size_t bufsize) const
{
    assert(bufsize >= GetSize());
    char* p = buf;
    memcpy(p, &handlerid, sizeof(handlerid));
    p += sizeof(handlerid);
    memcpy(p, &protocol, sizeof(protocol));
    p += sizeof(protocol);
    size_t datalen = GetDataLen();
    memcpy(p, &datalen, sizeof(datalen));
    p += sizeof(datalen);
    memcpy(p, data->GetBegin(), datalen);
}

void Package::Unserialize(const char* buf, size_t bufsize)
{
    assert(data);
    const char* p = buf;
    size_t remain = bufsize;

    assert(remain >= sizeof(handlerid));
    memcpy(&handlerid, p, sizeof(handlerid));
    p += sizeof(handlerid);
    remain -= sizeof(handlerid);

    assert(remain >= sizeof(protocol));
    memcpy(&handlerid, p, sizeof(protocol));
    p += sizeof(protocol);
    remain -= sizeof(protocol);

    size_t datalen;
    assert(remain >= sizeof(datalen));
    memcpy(&datalen, p, sizeof(datalen));
    p += sizeof(datalen);
    remain -= sizeof(datalen);
    assert(remain >= datalen);

    data->Resize(datalen);
    data->Copy(data->GetBegin(), p, datalen);
}

size_t Package::GetSize() const
{
    size_t datalen = GetDataLen();
    return sizeof(handlerid) + sizeof(protocol) + sizeof(datalen) + datalen;
}