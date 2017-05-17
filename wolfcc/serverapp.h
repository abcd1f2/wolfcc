#ifndef __WOLFCC_SERVERMANAGER_H__
#define __WOLFCC_SERVERMANAGER_H__
#include <string>
#include "pros/package.h"
#include "utils/queue.h"
#include "utils/allocator.h"
#include "utils/objectallocator.h"
#include "utils/handlerregistry.h"
#include "nets/pusheventhandler.h"
#include "utils/workthreadpool.h"
#include "utils/errorcallback.h"
#include "configmanager.h"

class AddressFilteredAcceptor;
class Reactor;
class PackProcessor;
class Processor;

class ServerApp
{
public:
    ServerApp(const std::string& config_file);
    ~ServerApp();

public:
    bool Init();
    void Start();
    void Stop();
    
private:
	bool InitApp();
    bool InitConfig();
    bool InitDB();
    bool InitServer();
    bool InitCluster();

private:
	void RunOnce();
	void Run();
	void HandlerBeforeSleep();
	bool Listen(const SockAddr& addr, int protocol, Processor* processor, std::list<std::string>* limitaddresses = NULL, uint32_t timeout = 3);
	bool Listen(const char* address, int protocol, Processor* processor, std::list<std::string>* limitaddresses = NULL, uint32_t timeout = 3);
	bool Listen(const SockAddr& addr, int protocol, uint32_t processorid, std::list<std::string>* limitaddresses = NULL, uint32_t timeout = 3);
	uint32_t RegisterProcessor(Processor* processor);
	void RemoveProcessor(uint32_t processor_id);

	Processor *CreateProcessor();

private:
    const std::string config_file_;
    ServerConfig server_config_;
	bool run_status_;

	std::list<Acceptor*> list_accept_;

	uint32_t pid_index_;
	std::map<uint32_t, Processor*> map_processors_;
    
	SyncStack<Package*> recv_queue_;
	SyncStack<Package*> send_queue_;
	NewAllocator new_allocator_;
    Reactor *reactor_p_;

	Processor *pack_processor_p_;
	ErrorCallback	error_callback_;
	WorkThreadPool	thread_pool_;

	PackageManager package_manager_;
	ObjectPoolAllocator<Package> package_allocator_;
	HandlerRegistry handler_registry_;
	PushEventHandler push_eventhandler_;

	StreamProtocolCallBack protocol_errorcallback_;

	SocketNotification send_notify_;
};

#endif
