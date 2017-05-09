#include "servermanager.h"
#include "configmanager.h"
#include "logging.h"

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
    LOG(LOG_INFO, "");
    if (!server_config_.LoadConfig()) {
        LOG(LOG_FATAL, "load server config error");
        return false;
    }

    return true;
}

void ServerManager::Start()
{

}

void ServerManager::Stop()
{

}
