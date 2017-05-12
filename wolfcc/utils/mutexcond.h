#ifndef __WOLFCC_MUTEXTCOND_H__
#define __WOLFCC_MUTEXTCOND_H__
#include <unistd.h>
#include <pthread.h>
#include "utils/mutex.h"

class MutexCond
{
public:
    typedef Mutex MutexType;

public:
    virtual ~MutexCond() {}
    virtual int Wait(const struct timespec * pTimeout = NULL) const = 0;
    virtual int Signal() const = 0;
    virtual int Broadcast() const = 0;
};

class NullMutexCond : public MutexCond
{
public:
    typedef NullMutex MutexType;

public:
    NullMutexCond(const NullMutex & cMutex = NullMutex());
    virtual int Wait(const struct timespec * = NULL) const { usleep(20); return 0; }
    virtual int Signal() const { return 0; }
    virtual int Broadcast() const { return 0; }
public:
    static NullMutexCond * Instance();
};

class ThreadMutexCond : public MutexCond
{
public:
    typedef ThreadMutex MutexType;

public:
    ThreadMutexCond(const ThreadMutex & cThreadMutex);
    ~ThreadMutexCond();

    virtual int Wait(const struct timespec * pTimeout = NULL) const;
    virtual int Signal() const;
    virtual int Broadcast() const;

private:
    const ThreadMutex & cThreadMutex_;
    mutable pthread_cond_t cond_;
};

#endif