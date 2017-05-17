#ifndef __WOLFCC_WORKTHREADPOOL_H__
#define __WOLFCC_WORKTHREADPOOL_H__
#include <map>
#include "pros/package.h"
#include "utils/queue.h"
#include "utils/threadpool.h"

class ReadStream;
class WriteStream;
class Processor;
class ErrorCallback;
class Request;
class Response;

enum PackageLogLevel
{
	PACKAGE_LOG_NONE,
	PACKAGE_LOG_ERROR,
	PACKAGE_LOG_ALL
};

class WorkThreadPool : public ThreadPool
{
	Queue<Package*>*  recvqueue_p_;
	Queue<Package*>*  sendqueue_p_;
	std::map<size_t, Processor*>* processors_p_;
	PackageManager *      pPackageManager_p_;
	Allocator*        allocator_p_;
	PackageLogLevel       debuglevel;
	ErrorCallback*        errorcb_p_;

public:
	WorkThreadPool(Queue<Package*>* recvqueue,
		Queue<Package*>* sendqueue,
		std::map<size_t, Processor*>* processors,
		PackageManager * pPackageManager,
		Allocator* allocator = Allocator::Instance());

	void SetErrorCallback(ErrorCallback* ecb)
	{
		errorcb_p_ = ecb;
	}

protected:
	Package* CreatePackage();

	void DestroyPackage(Package * p);

	virtual bool RunOnce();

	virtual void StopThreads(size_t stopcount);

private:
	ReadStream* CreateReadStream(int protocol, const char* p, size_t len);
	void DestroyReadStream(int protocol, ReadStream* readStram);

	WriteStream* CreateWriteStream(int protocol, char* buf, size_t buflen);
	void DestroyWriteStream(int protocol, WriteStream* writeStram);

	bool ProcessProtocol(Processor* processor, Package* package);
	bool ProcessSerialnoPackage(Processor* processor, Package* package, Request* q, Response* s);
	bool ProcessPackage(Processor* processor, Package* package, Request* q, Response* s);

	static void DestroyDataBlock(DataBlock* db);
	static DataBlock* CreateDataBlock();

	static void DestroyDataBlockList(void* p);
};

#endif
