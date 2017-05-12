﻿#ifndef __WOLFCC_MUTEXT_H__
#define __WOLFCC_MUTEXT_H__
#include <pthread.h>
#include <sys/sem.h>

class Mutex
{
public:
    virtual ~Mutex() {}
    virtual int Acquire(bool block = true) const = 0;
    virtual int Release() const = 0;
};

class LockGuard
{
public:
    LockGuard(const Mutex * mutex) : mutex_(mutex), acquired_(false) {}

    ~LockGuard()
    {
        if (acquired_) {
            mutex_->Release();
        }
    }

    int Lock(bool block = true) const
    {
        if (acquired_) {
            return -1;
        }

        if (mutex_->Acquire(block) != 0) {
            return -1;
        }

        acquired_ = true;
        return 0;
    }

    int Unlock() const
    {
        if (!acquired_) {
            return -1;
        }
        if (mutex_->Release() != 0) {
            return -1;
        }
        acquired_ = false;
        return 0;
    }

    bool IsLocked() const
    {
        return acquired_;
    }

private:
    const Mutex * mutex_;
    mutable bool acquired_;
};

class NullMutex : public Mutex
{
public:
    virtual int Acquire(bool block = true) const { (void)block; return 0; }
    virtual int Release() const { return 0; }
public:
    static NullMutex * Instance();
};

class ThreadMutex : public Mutex
{
private:
    ThreadMutex(const ThreadMutex&);
    ThreadMutex& operator=(const ThreadMutex&);

public:
    ThreadMutex();
    virtual ~ThreadMutex();

    virtual int Acquire(bool block = true) const;
    virtual int Release() const;

private:
    mutable pthread_mutex_t mutex_;

    friend class ThreadMutexCond;
};

class SemMutex : public Mutex
{
private:
    SemMutex(const SemMutex&);
    SemMutex& operator=(const SemMutex&);

public:
    SemMutex(key_t key, int oflag = 0644);
    virtual int Acquire(bool block = true) const;
    virtual int Release() const;

private:
    int semid_;
};

#endif
