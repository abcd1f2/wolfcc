#include "utils/workthreadpool.h"
#include "utils/logging.h"
#include "utils/scopeguard.h"
#include "utils/errorcallback.h"
#include "utils/datablock.h"
#include <unistd.h>

#ifndef ACF_MSG_ELEMENT_MAX_SIZE
#define ACF_MSG_ELEMENT_MAX_SIZE (16 * 1024 * 1024)
#endif

const char* ACF_DATACODE_FIELD = "encode";
const char* ACF_DATA_FIELD = "data";
const char* ACF_CACHE_HIT = "hit";

struct FieldValue {
	const char* ptr;
	size_t size;
};

void DeleteReadStreamCache(void* p) {
	std::map<int, std::list<ReadStream*> >* cache = (std::map<int, std::list<ReadStream*> >*)p;
	std::map<int, std::list<ReadStream*> >::const_iterator it;
	for (it = cache->begin(); it != cache->end(); it++) {

		const std::list<ReadStream*>& lst = it->second;

		std::list<ReadStream*>::const_iterator it;
		for (it = lst.begin(); it != lst.end(); it++) {
			delete (*it);
		}
	}
	delete cache;
}

void DeleteWriteStreamCache(void* p) {
	std::map<int, std::list<WriteStream*> >* cache = (std::map<int, std::list<WriteStream*> >*)p;
	std::map<int, std::list<WriteStream*> >::const_iterator it;
	for (it = cache->begin(); it != cache->end(); it++) {

		const std::list<WriteStream*>& lst = it->second;

		std::list<WriteStream*>::const_iterator it;
		for (it = lst.begin(); it != lst.end(); it++) {
			delete (*it);
		}
	}
	delete cache;
}

size_t GetProtocolMaxSize(int protocol) {
	switch (protocol) {

/*
	case TEXT_STREAM_PROTOCOL:
	case COMPOSITE_TEXT_STREAM_PROTOCOL:
		return TEXT_PACKAGE_MAXLEN + TEXT_PACKLEN_LEN;

	case BINARY_STREAM_PROTOCOL:
	case COMPOSITE_BINARY_STREAM_PROTOCOL:
		return BINARY_PACKAGE_MAXLEN;

	case TEXT_STREAM_PROTOCOL_2:
	case SERIALNO_WRAPPER_PROTOCOL:
	case COMPOSITE_TEXT_STREAM_PROTOCOL_2:
		return TEXT_PACKAGE_MAXLEN_2 + TEXT_PACKLEN_LEN_2;*/

	default:
		assert(false);
	}
}


WorkThreadPool::WorkThreadPool(
	Queue<Package*>* packqueue_,
	Queue<Package*>* resultqueue_,
	std::map<size_t, Processor*>* processors_,
	PackageManager * pPackageManager_,
	Allocator* allocator_)
	: recvqueue_p_(packqueue_), sendqueue_p_(resultqueue_), processors_p_(processors_), pPackageManager_p_(pPackageManager_),
	allocator_p_(allocator_),debuglevel(PACKAGE_LOG_NONE), errorcb_p_(NULL)

{
	assert(pPackageManager_p_);
	assert(allocator_p_);
}

void WorkThreadPool::DestroyDataBlockList(void* p)
{
	std::list<DataBlock*>* lst = (std::list<DataBlock*>*)p;
	std::list<DataBlock*>::const_iterator it;
	for (it = lst->begin(); it != lst->end(); it++) {
		delete *it;
	}
	delete lst;
}

void WorkThreadPool::DestroyDataBlock(DataBlock* db)
{
	/*
	std::list<DataBlock*>* lst = (std::list<DataBlock*>*)dataBlockTSS.get();
	assert(lst);
	lst->push_back(db);
	*/
}

DataBlock* WorkThreadPool::CreateDataBlock()
{
/*
	std::list<DataBlock*>* lst = (std::list<DataBlock*>*)dataBlockTSS.get();
	if (!lst) {
		lst = new std::list<DataBlock*>;
		dataBlockTSS.set(lst);
	}

	if (lst->begin() == lst->end()) {
		return new DataBlock();
	}

	DataBlock* db = lst->back();
	lst->pop_back();
	return db;*/
	return NULL;
}


ReadStream* WorkThreadPool::CreateReadStream(int protocol, const char* p, size_t len)
{
/*
	if (protocol == SERIALNO_WRAPPER_PROTOCOL) {
		protocol = TEXT_STREAM_PROTOCOL_2;
	}

	std::map<int, std::list<ReadStream*> >* cache
		= (std::map<int, std::list<ReadStream*> >*)__readStreamCache.get();
	if (!cache) {
		cache = new std::map<int, std::list<ReadStream*> >();
		__readStreamCache.set(cache);
	}

	std::list<ReadStream*>& lst = (*cache)[protocol];

	protocol = protocol & ~COMPOSITE_FLAG;

	if (lst.begin() == lst.end()) {
		switch (protocol) {
		case TEXT_STREAM_PROTOCOL:
			return new TextReadStream(p, len);
		case BINARY_STREAM_PROTOCOL:
			return new BinaryReadStream(p, len);
		case TEXT_STREAM_PROTOCOL_2:
			return new TextReadStream2(p, len);
		default:
			return NULL;
		}
	}

	ReadStream* stream = lst.back();
	stream->Assign(p, len);
	lst.pop_back();
	return stream;*/
	return NULL;
}

void WorkThreadPool::DestroyReadStream(int protocol, ReadStream* readStream)
{
/*
	std::map<int, std::list<ReadStream*> >* cache
		= (std::map<int, std::list<ReadStream*> >*)__readStreamCache.get();
	assert(cache);

	protocol = protocol & ~COMPOSITE_FLAG;
	if (protocol == SERIALNO_WRAPPER_PROTOCOL) {
		protocol = TEXT_STREAM_PROTOCOL_2;
	}

	std::list<ReadStream*>& lst = (*cache)[protocol];
	lst.push_back(readStream);*/
}

WriteStream* WorkThreadPool::CreateWriteStream(int protocol, char* buf, size_t buflen)
{
/*
	std::map<int, std::list<WriteStream*> >* cache
		= (std::map<int, std::list<WriteStream*> >*)__writeStreamCache.get();
	if (!cache) {
		cache = new std::map<int, std::list<WriteStream*> >();
		__writeStreamCache.set(cache);
	}

	if (protocol == SERIALNO_WRAPPER_PROTOCOL) {
		protocol = TEXT_STREAM_PROTOCOL_2;
	}

	std::list<WriteStream*>& lst = (*cache)[protocol];

	if (lst.begin() == lst.end()) {

		switch (protocol) {
		case TEXT_STREAM_PROTOCOL:
			return new TextWriteStream(buf, buflen);
		case BINARY_STREAM_PROTOCOL:
			return new BinaryWriteStream(buf, buflen);
		case TEXT_STREAM_PROTOCOL_2:
			return new TextWriteStream2(buf, buflen);
		case COMPOSITE_TEXT_STREAM_PROTOCOL:
			return new CompositeTextWriteStream(buf, buflen);
		case COMPOSITE_BINARY_STREAM_PROTOCOL:
			return new CompositeBinaryWriteStream(buf, buflen);
		default:
			return NULL;
		}
	}

	WriteStream* stream = lst.back();
	stream->Assign(buf, buflen);
	lst.pop_back();
	return stream;*/
	return NULL;
}

void WorkThreadPool::DestroyWriteStream(int protocol, WriteStream* writeStream)
{
/*
	std::map<int, std::list<WriteStream*> >* cache
		= (std::map<int, std::list<WriteStream*> >*)__writeStreamCache.get();
	assert(cache);

	if (protocol == SERIALNO_WRAPPER_PROTOCOL) {
		protocol = TEXT_STREAM_PROTOCOL_2;
	}

	std::list<WriteStream*>& lst = (*cache)[protocol];
	lst.push_back(writeStream);*/
}


bool WorkThreadPool::RunOnce()
{
	Package* p = NULL;
	int ret = recvqueue_p_->Pop(p, true); // block
	assert(ret == 0);

	if (!p) {
		log(LOG_INFO, "received exit message, exit thread");
		return false;
	}

	ScopeGuard packageDestroyGuard = MakeObjGuard(*p, &Package::Destroy);

	gettimeofday(&p->gettime, NULL);

	size_t processorid = p->GetProcessorId();
	std::map<size_t, Processor*>::const_iterator iter;
	iter = processors_p_->find(processorid);
	if (iter == processors_p_->end()) {
		log(LOG_ERR, "invalid processorid: %zu", processorid);
		return true;
	}

	Processor *processor = iter->second;
	if (!ProcessProtocol(processor, p)) {
		return true;
	}

	if (sendqueue_p_->Push(p) != 0) {
		log(LOG_ERR, "put package into result queue fail");
		if (errorcb_p_) {
			errorcb_p_->OnOutputQueueFull();
		}
	}
	else {
		packageDestroyGuard.Dismiss();
		gettimeofday(&p->puttime2, NULL);
	}

	return true;
}

bool WorkThreadPool::ProcessProtocol(Processor* processor, Package* p)
{
	int protocol = p->GetProtocol();
	DataBlock* db = CreateDataBlock();
	db->Clear();

	if (protocol == SERIALNO_WRAPPER_PROTOCOL) {
		if (!ProcessSerialnoPackage(processor, p))
			return false;
	}
	else {
		if (!ProcessPackage(processor, p))
			return false;
	}

	return true;
}

bool WorkThreadPool::ProcessSerialnoPackage(Processor* processor, Package* p, Request* request, Response* responce)
{
	log(LOG_DEBUG, "SERIAL PACKAGE: %s", std::string(p->GetData(), p->GetDataLen()).c_str());

/*
	int count;
	if (!request->readStream->Read(count))
	{
		log(LOG_WARN, "Read SERIALNO_WRAPPER_PROTOCOL header count fail");
		return false;
	}

	std::map<std::string, std::string> headers;

	for (; count > 0; --count)
	{
		const char* p;
		size_t len;
		if (!request->readStream->ReadNoCopy(p, len))
		{
			log(LOG_WARN, "Read SERIALNO_WRAPPER_PROTOCOL header KEY fail");
			return false;
		}

		std::string key(p, len);

		const char* pv;
		size_t lenv;
		if (!request->readStream->ReadNoCopy(pv, lenv))
		{
			log(LOG_WARN, "Read SERIALNO_WRAPPER_PROTOCOL header VALUE fail");
			return false;
		}

		headers.insert(make_pair(key, std::string(pv, lenv)));
	}

	std::map<std::string, std::string>::iterator it = headers.find(ACF_DATACODE_FIELD);
	if (it == headers.end())
	{
		log(LOG_WARN, "Read SERIALNO_WRAPPER_PROTOCOL header field ACF_DATACODE_FIELD fail");
		return false;
	}

	int protocol = atoi(it->second.c_str());

	it = headers.find(ACF_DATA_FIELD);
	if (it == headers.end())
	{
		log(LOG_WARN, "Read SERIALNO_WRAPPER_PROTOCOL header field ACF_DATA_FIELD fail");
		return false;
	}

	p->SetData(it->second.c_str(), it->second.size());
	p->SetProtocol(protocol);

	if (!ProcessProtocol(processor, p, &responce->cacheHit)) {
		log(LOG_WARN, "Process protocol fail");
		return false;
	}

	p->SetProtocol(SERIALNO_WRAPPER_PROTOCOL);
	it->second.assign(p->GetData(), p->GetDataLen());

	char tmp[20];
	if (p->GetDataLen() != 0)
	{
		snprintf(tmp, sizeof(tmp), "%d", responce->cacheHit);
		headers.insert(std::make_pair(ACF_CACHE_HIT, tmp));

		if (!responce->writeStream->Write((int)headers.size()))
		{
			log(LOG_WARN, "Write header count fail");
			return false;
		}

		for (it = headers.begin(); it != headers.end(); it++)
		{
			if (!responce->writeStream->Write(it->first.c_str(), it->first.length()) ||
				!responce->writeStream->Write(it->second.c_str(), it->second.size()))
			{
				log(LOG_WARN, "Write headers fail");
				return false;
			}
		}

		responce->writeStream->Flush();
		p->SetData(responce->writeStream->GetData(), responce->writeStream->GetSize());
	}

	log(LOG_DEBUG, "SERIAL REPLY PACKAGE: %s, len: %d", std::string(p->GetData(), p->GetDataLen()).c_str(), p->GetDataLen());
*/

	return true;
}

bool WorkThreadPool::ProcessPackage(Processor* processor, Package* p, Request* request, Response* responce)
{
/*
	int ret = processor->Process(request, responce);

	if (debuglevel == PACKAGE_LOG_ALL ||
		(debuglevel == PACKAGE_LOG_ERROR && ret != 0))
	{
		AC_LOG(debugpriority, "PACKAGE: %s", string(p->GetData(), p->GetDataLen()).c_str());
	}

	p->SetRetCode(ret);

	if (responce->writeStream->IsValid())
	{
		responce->writeStream->Flush();
		p->SetData(responce->writeStream->GetData(), responce->writeStream->GetSize());
	}
	else
	{
		p->SetData("", 0);
	}*/

	gettimeofday(&p->processtime, NULL);

	return true;
}

void WorkThreadPool::StopThreads(size_t stopcount)
{
	size_t i = 0;
	while (i != stopcount) {
		if (recvqueue_p_->Push(0) != 0) {
			usleep(20);
		}
		else {
			++i;
		}
	}
}
