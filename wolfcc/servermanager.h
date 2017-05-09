#ifndef __WOLFCC_SERVERMANAGER_H__
#define __WOLFCC_SERVERMANAGER_H__
#include <string>

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
    const std::string config_file_;
    ServerConfig server_config_;
};

#endif
