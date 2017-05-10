#include "servermanager.h"
#include "utils/logging.h"
#include "nets/epollevent.h"

ServerManager::ServerManager(const std::string& config_file):
    config_file_(config_file),
    server_config_(config_file)
{

}

ServerManager::~ServerManager()
{

}

bool ServerManager::Init()
{
    if (!InitConfig()) {
        log(LOG_FATAL, "init config error");
        return false;
    }

    if (!InitNet()) {
        log(LOG_FATAL, "init net error");
        return false;
    }

    if (!InitDB()) {
        log(LOG_FATAL, "init db error");
        return false;
    }

    if (!InitServer()) {
        log(LOG_FATAL, "init server error");
        return false;
    }

    if (!InitCluster()) {
        log(LOG_FATAL, "init cluster error");
        return false;
    }

    return true;
}

bool ServerManager::InitConfig()
{
    //init config
    if (!server_config_.LoadConfig()) {
        log(LOG_FATAL, "load server config error, %s", config_file_.c_str());
        return false;
    }

    return true;
}

bool ServerManager::InitNet()
{
    //init server net
    net_manager_p_ = new NetManager(new EpollEvent());

    if (!net_manager_p_ || !net_manager_p_->StartListen()) {
        log(LOG_ERR, "listen error");
        return false;
    }

    return true;
}

bool ServerManager::InitDB()
{
    // load db 
    return true;
}

bool ServerManager::InitServer()
{
    // init server
    return true;
}

bool ServerManager::InitCluster()
{
    //init cluster
    return true;
}

void ServerManager::Start()
{
    net_manager_p_->StartMainLoop();
}

void ServerManager::Stop()
{

}
