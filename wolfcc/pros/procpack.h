#ifndef __WOLFCC_PROCPACK_H__
#define __WOLFCC_PROCPACK_H__
#include "nets/sockaddr.h"
#include "utils/queue.h"

class ErrorCallback;
class Package;
class Allocator;
class PackageManager;

class Procpack
{
public:
    virtual ~Procpack(){}

public:
    void SetErrorCallback(ErrorCallback* ecb) {
        errorcb_p_ = ecb;
    }

public:
    virtual int Extract(size_t sockid, size_t processorid, const char* ptr, size_t len, const SockAddr& addr) = 0;
    virtual int OnPackage(size_t sockid, size_t processorid, const char* ptr, size_t len, const SockAddr& addr) = 0;

protected:
    ErrorCallback*	errorcb_p_;
};

typedef Queue<Package*> PackageQueue;
class TextStreamProcpack : public Procpack
{
public:
    TextStreamProcpack(PackageQueue* queue, Allocator* allocator, PackageManager *  pPackageManager, int protocol);

public:
    void SetQueueLimit(size_t queuelimit) {
        this->queuelimit = queuelimit;
    }

public:
    virtual int OnPackage(size_t sockid, size_t proessorid, const char* ptr, size_t len, const SockAddr& addr);
    virtual int Extract(size_t sockid, size_t processorid, const char* ptr, size_t len, const SockAddr& addr);

private:
    PackageQueue*  queuepackage_p_;
    Allocator* allocator_p_;
    PackageManager *  packageManager_p_;
    size_t       queuelimit;
    int			protocol;
};

#endif
