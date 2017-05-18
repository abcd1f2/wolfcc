#ifndef __WOLFCC_PROCPACK_H__
#define __WOLFCC_PROCPACK_H__
#include "nets/sockaddr.h"
#include "utils/queue.h"
#include "pros/package.h"

class ErrorCallback;
class Package;
class Allocator;

enum 
{
    TEXT_PACKLEN_LEN = 4,
    TEXT_PACKAGE_MAXLEN = 0xffff,
    BINARY_PACKLEN_LEN = 2,
    BINARY_PACKAGE_MAXLEN = 0xffff,
};

enum 
{
	BINARY_PACKAGE_STREAM_PROTOCOL_1_0 = 1,
	TEXT_PACKAGE_STREAM_PROTOCOL_1_0
};

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

class TextStreamProcpack : public Procpack
{
public:
    virtual ~TextStreamProcpack(){}

public:
    virtual int Extract(size_t sockid, size_t processorid, const char* ptr, size_t len, const SockAddr& addr);
};

class BinaryStreamProcpack : public Procpack
{
public:
    virtual ~BinaryStreamProcpack(){}

public:
    virtual int Extract(size_t sockid, size_t processorid, const char* ptr, size_t len, const SockAddr& addr);
};


typedef Queue<Package*> PackageQueue;
class AppTextStreamProcpack : public TextStreamProcpack
{
public:
    AppTextStreamProcpack(PackageQueue* queue, Allocator* allocator, PackageManager *pPackageManager, int protocol);
    virtual ~AppTextStreamProcpack(){}

public:
    void SetQueueLimit(size_t queuelimit) {
        this->queuelimit_ = queuelimit;
    }

public:
    virtual int OnPackage(size_t sockid, size_t processorid, const char* ptr, size_t len, const SockAddr& addr);

private:
    PackageQueue*  queuepackage_p_;
    Allocator* allocator_p_;
    PackageManager *  packageManager_p_;
    size_t       queuelimit_;
    int			protocol_;
};


class AppBinaryStreamProcpack : public BinaryStreamProcpack
{
public:
    AppBinaryStreamProcpack(PackageQueue* queue, Allocator* allocator, PackageManager *pPackageManager, int protocol);
    virtual ~AppBinaryStreamProcpack(){}

public:
    void SetQueueLimit(size_t queuelimit) {
        this->queuelimit_ = queuelimit;
    }

public:
    virtual int OnPackage(size_t sockid, size_t processorid, const char* ptr, size_t len, const SockAddr& addr);

private:
    PackageQueue*  queuepackage_p_;
    Allocator* allocator_p_;
    PackageManager *  packageManager_p_;
    size_t       queuelimit_;
    int			protocol_;
};

#endif
