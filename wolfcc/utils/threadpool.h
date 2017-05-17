#ifndef __WOLFCC_THREADPOOL_H__
#define __WOLFCC_THREADPOOL_H__
#include <pthread.h>
#include <list>
#include "utils/thread.h"

class JobThread;

// NOTICE: ThreadPool 对象不能在多个线程中被同时操控
class ThreadPool
{
protected:
	typedef std::list<Thread*>	ThreadPtrContainer;

	ThreadPtrContainer	livethreads;
	ThreadPtrContainer	deadthreads;

	pthread_mutex_t		mutex;

	volatile size_t			stopcount;

public:

	ThreadPool();

	virtual ~ThreadPool();

	int Start(size_t count);

	void Stop(size_t count);

	void StopAll();

	size_t GetThreadNum();

protected:

	virtual bool RunOnce();

	virtual void StopThreads(size_t stopcount);


protected:

	Thread* CreateThread();

	void DestroyThread(Thread* thread);
	void DestroyThread(pthread_t pid);

	friend class JobThread;
};

#endif
