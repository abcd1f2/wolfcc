#ifndef __WOLFCC_PACKAGE_H__
#define __WOLFCC_PACKAGE_H__
#include <sys/types.h>
#include <sys/time.h>
#include "nets/event.h"
#include "nets/sockaddr.h"
#include "utils/objectmanager.h"

class Allocator;
class DataBlock;

class Package
{
public:
    Package();
    virtual ~Package();

    void SetAllocator(Allocator* allocator);
    Allocator* GetAllocator() const { return allocator; }

    size_t GetHandlerId() const { return handlerid; }
    void SetHandlerId(size_t handlerid) { this->handlerid = handlerid; }

    size_t GetProcessorId() const { return processorid; }
    void SetProcessorId(size_t processorid) { this->processorid = processorid; }

    size_t GetProtocol() const { return protocol; }
    void SetProtocol(size_t protocol) { this->protocol = protocol; }

    const char* GetData() const;
    void SetData(const char* data, size_t len);

    void SetPeerAddr(const SockAddr& addr);
    const SockAddr& GetPeerAddr() const;

    size_t GetDataLen() const;

    int GetRetCode() const { return retcode; }
    void SetRetCode(int retcode) { this->retcode = retcode; }


    virtual void Serialize(char* buf, size_t bufsize) const;

    virtual void Unserialize(const char* buf, size_t bufsize);

    virtual size_t GetSize() const;

private:

    Allocator* allocator;
    size_t      handlerid;
    size_t      processorid;
    size_t      protocol;
    int         retcode;
    DataBlock*  data;
    SockAddr peerAddr;

public:
    timeval      starttime;
    timeval      puttime;
    timeval      gettime;
    timeval      processtime;
    timeval      puttime2;
    timeval      gettime2;
};

typedef ObjectManager<Package> PackageManager;

#endif
