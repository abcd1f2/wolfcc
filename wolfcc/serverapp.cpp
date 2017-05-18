#include "serverapp.h"
#include "utils/logging.h"
#include "nets/addressfilteracceptor.h"
#include "nets/libeventreactor.h"
#include "pros/packprocessor.h"

ServerApp::ServerApp(const std::string& config_file):
	pid_index_(0),
	pack_processor_p_(NULL),
    config_file_(config_file),
    server_config_(config_file),
	package_allocator_(128, 1000),
	run_status_(true),
	package_manager_(&package_allocator_),
	thread_pool_(&recv_queue_, &send_queue_, &map_processors_, &package_manager_)
{

}

ServerApp::~ServerApp()
{

}

bool ServerApp::Init()
{
	if (!InitConfig()) {
		log(LOG_FATAL, "init config error");
		return false;
	}

    if (!InitDB()) {
        log(LOG_FATAL, "init db error");
        return false;
    }

    if (!InitCluster()) {
        log(LOG_FATAL, "init cluster error");
        return false;
    }

	if (!InitApp()) {
		log(LOG_ERR, "init server app error");
		return false;
	}

    return true;
}

bool ServerApp::InitConfig()
{
    //init config
    if (!server_config_.LoadConfig()) {
        log(LOG_FATAL, "load server config error, %s", config_file_.c_str());
        return false;
    }

    return true;
}

bool ServerApp::InitApp()
{
	pack_processor_p_ = CreateProcessor();
	if (!server_config_.server_tcp_addr_.empty()) {
		if (!Listen(server_config_.server_tcp_addr_.c_str(), server_config_.server_protocol_type_, pack_processor_p_, &server_config_.limit_address_, server_config_.timeout_)) {
			log(LOG_ERR, "listen tcp %s error", server_config_.server_tcp_addr_.c_str());
			return false;
		}
	}

	if (!server_config_.server_unix_addr_.empty()) {
		if (Listen(server_config_.server_unix_addr_.c_str(), server_config_.server_protocol_type_, pack_processor_p_, &server_config_.limit_address_, server_config_.timeout_)){
			log(LOG_ERR, "listen unix %s error", server_config_.server_unix_addr_.c_str());
			return false;
		}
	}

	reactor_p_ = new LibEventReactor();
	assert(reactor_p_);

	push_eventhandler_.SetQueue(&send_queue_);
	push_eventhandler_.SetHandlerRegistry(&handler_registry_);
	push_eventhandler_.SetReactor(reactor_p_);

	if (-1 == push_eventhandler_.Open()) {
		log(LOG_ERR, "push open error");
		return false;
	}

	send_notify_.SetHandle(push_eventhandler_.GetAnotherHandle());
	send_queue_.SetNotification(&send_notify_);

	thread_pool_.SetErrorCallback(&error_callback_);
	thread_pool_.Start(server_config_.thread_count_);

	for (std::list<Acceptor*>::iterator listIte = list_accept_.begin(); listIte != list_accept_.end(); listIte++) {
		(*listIte)->SetReactor(reactor_p_);
		if ((*listIte)->Open() == -1) {
			log(LOG_ERR, "accept error %s", (*listIte)->GetAddr().ToString().c_str());
			return false;
		}
	}

    return true;
}

bool ServerApp::InitDB()
{
    // load db 
    return true;
}

bool ServerApp::InitServer()
{
    // init server
    return true;
}

bool ServerApp::InitCluster()
{
    //init cluster
    return true;
}

Processor *ServerApp::CreateProcessor()
{
	if (server_config_.server_protocol_type_ == PROCESSOR_PROTOCOL_MESSAGEPACK) {
		pack_processor_p_ = new MessagepackProcessor();
	}
	else if (server_config_.server_protocol_type_ == PROCESSOR_PROTOCOL_PROTOBUF) {
		pack_processor_p_ = new ProtoBufProcessor();
	}
}

uint32_t ServerApp::RegisterProcessor(Processor* processor)
{
	uint32_t t_index = pid_index_;
	map_processors_.insert(std::make_pair(t_index, processor));
	pid_index_++;
	return t_index;
}

void ServerApp::RemoveProcessor(uint32_t processor_id)
{
	map_processors_.erase(processor_id);
}

bool ServerApp::Listen(const char* address, int protocol, Processor* processor, std::list<std::string>* limitaddresses /* = NULL */, uint32_t timeout /* = 3 */)
{
	SockAddr addr = ParseAddress(address);
	return Listen(addr, protocol, processor, limitaddresses, timeout);
}

bool ServerApp::Listen(const SockAddr& addr, int protocol, Processor* processor, std::list<std::string>* limitaddresses /* = NULL */, uint32_t timeout /* = 3 */)
{
	uint32_t processor_index = RegisterProcessor(processor);
	if (!Listen(addr, protocol, processor_index, limitaddresses, timeout)){
		log(LOG_ERR, "listen %s error", addr.ToString().c_str());
		RemoveProcessor(processor_index);
		return false;
	}

	return true;
}

bool ServerApp::Listen(const SockAddr& addr, int protocol, uint32_t processorid, std::list<std::string>* limitaddresses /*= NULL*/, uint32_t timeout/* = 3*/)
{
	Procpack *stream_protocol_p = NULL;
	if (protocol == BINARY_PACKAGE_STREAM_PROTOCOL_1_0) {
		stream_protocol_p = new AppBinaryStreamProcpack(&recv_queue_, &new_allocator_, &package_manager_, server_config_.server_protocol_type_);
	}
	else if (protocol == TEXT_PACKAGE_STREAM_PROTOCOL_1_0) {
		stream_protocol_p = new AppTextStreamProcpack(&recv_queue_, &new_allocator_, &package_manager_, server_config_.server_protocol_type_);
	}

	stream_protocol_p->SetErrorCallback(&protocol_errorcallback_);
	AddressFilteredAcceptor *accept_p = new AddressFilteredAcceptor(addr, processorid, stream_protocol_p, &handler_registry_);
	accept_p->SetLimitAddresses(limitaddresses);
	accept_p->SetTimeout(timeout);
	list_accept_.push_back(static_cast<Acceptor*>(accept_p));
	return true;
}

void ServerApp::Start()
{
	while (run_status_) {
		HandlerBeforeSleep();
		RunOnce();
	}
}

void ServerApp::RunOnce()
{
	reactor_p_->RunOnce();
}

void ServerApp::Run()
{
	reactor_p_->Run();
}

void ServerApp::HandlerBeforeSleep()
{

}

void ServerApp::Stop()
{

}
