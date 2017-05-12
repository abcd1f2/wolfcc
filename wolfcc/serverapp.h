#ifndef __WOLFCC_SERVERMANAGER_H__
#define __WOLFCC_SERVERMANAGER_H__
#include <string>
#include "configmanager.h"
#include "nets/addressfilteracceptor.h"

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
    bool InitConfig();
    bool InitNet();
    bool InitDB();
    bool InitServer();
    bool InitCluster();

private:
    const std::string config_file_;
    ServerConfig server_config_;
    AddressFilteredAcceptor *accept_p_;
};

#endif
