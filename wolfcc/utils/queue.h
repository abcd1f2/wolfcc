#ifndef __WOLFCC_QUEUE_H__
#define __WOLFCC_QUEUE_H__

#include <pthread.h>
#include <list>
#include <time.h>
#include "utils/notify.h"

template<class T>
class Queue
{
public:
	Queue() : notify_p_(NULL) {}

    virtual ~Queue() {}

    void SetNotification(Notification* notify) { 
        this->notify = notify_p_;
    }

    Notification* GetNotification() { 
        return this->notify_p_;
    }

    int Push(const T& value, T* removed = NULL, bool limit = true)
    {
        int ret = PushImp(value, removed, limit);
        if (ret == 0 && GetNotification()) {
            GetNotification()->notify(this);
        }
        return ret;
    }

    int Pop(T& value, bool block = false) {
        return PopImp(value, block);
    }

    virtual size_t GetSize() const = 0;

protected:
    virtual int PushImp(const T& value, T* removed = NULL, bool limit = true) = 0;
    virtual int PopImp(T& value, bool block) = 0;

private:
    Notification* notify_p_;
};

template<class T>
class SyncQueue : public Queue < T >
{
public:
    enum { DEFAULT_LIMIT = 1000 };

public:
    SyncQueue(size_t limit = DEFAULT_LIMIT)
        : limit_(limit), count_(0) {
        pthread_mutex_init(&mutex_, NULL);
        pthread_cond_init(&cond_, NULL);
    }

    virtual ~SyncQueue() {
        pthread_cond_destroy(&cond_);
        pthread_mutex_destroy(&mutex_);
    }

    virtual size_t GetSize() const {
        return count_;
    }

    size_t GetLimit() const {
        return limit_;
    }

    void SetLimit(size_t limit) {
        limit_ = limit;
    }

protected:

    virtual int PushImp(const T& value, T* removed = NULL, bool limit = true)
    {
        (void)removed;
        pthread_mutex_lock(&mutex_);

        if (limit && count_ >= limit_) {
            pthread_mutex_unlock(&mutex_);
            return -1;
        }

        queue_.push_back(value);
        count_++;

        //pthread_cond_broadcast(&cond);
        pthread_cond_signal(&cond_);
        pthread_mutex_unlock(&mutex_);

        return 0;
    }

    virtual int PopImp(T& value, bool block)
    {
        pthread_mutex_lock(&mutex_);
        while (true) {
            if (!queue_.empty()) {
                value = queue_.front();
                queue_.pop_front();
                count_--;
                pthread_mutex_unlock(&mutex_);
                return 0;
            }

            if (!block) {
                pthread_mutex_unlock(&mutex_);
                return -1;
            }

            timespec timeout;
            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_sec += 1;
            pthread_cond_timedwait(&cond_, &mutex_, &timeout);
        }
    }

protected:
    mutable pthread_mutex_t mutex_;
    mutable pthread_cond_t  cond_;

    std::list<T> queue_;
    size_t	limit_;
    size_t	count_;
};

template <class T>
class SyncStack : public SyncQueue < T >
{
protected:
    virtual int PushImp(const T& value, T* removed = NULL, bool limit = true)
    {
        pthread_mutex_lock(&this->mutex_);

        if (limit && this->count_ >= this->limit_) {
            if (removed) {
                *removed = this->queue_.back();
                this->queue_.pop_back();
                this->count_--;
            }
            else {
                pthread_mutex_unlock(&this->mutex_);
                return -1;
            }
        }

        this->queue_.push_front(value);
        this->count_++;

        //pthread_cond_broadcast(&cond);
        pthread_cond_signal(&this->cond_);
        pthread_mutex_unlock(&this->mutex_);

        return 0;
    }
};

#endif
