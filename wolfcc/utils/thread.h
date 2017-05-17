#ifndef __WOLFCC_THREAD_H__
#define __WOLFCC_THREAD_H__
#include <pthread.h>

class Thread
{
public:
	Thread();
	virtual ~Thread();
	virtual int Start();
	virtual void Stop();
	void Join();

	inline bool IsAlive() const { return alive; }
	inline pthread_t GetThreadID() const { return hdl; }

private:
	virtual void Run() = 0;

private:
	pthread_t hdl;
	bool alive;

	friend void* __THREAD_FUNC(void* p);
};

#endif
