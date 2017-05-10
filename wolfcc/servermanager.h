#ifndef __WOLFCC_SERVERMANAGER_H__
#define __WOLFCC_SERVERMANAGER_H__
#include <string>
#include "configmanager.h"
#include "netmanager.h"

class ServerManager
{
public:
    ServerManager(const std::string& config_file);
    ~ServerManager();

public:
    bool Init();
    void Start();
    void Stop();
    
private:
    bool InitConfig();
    bool InitNet();
    bool InitDB();
    bool InitServer();
    bool InitCluster();

private:
    const std::string config_file_;
    ServerConfig server_config_;
    NetManager *net_manager_p_;
};

#endif
