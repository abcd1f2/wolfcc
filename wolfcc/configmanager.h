#ifndef __CONFIGMANAGER_H__
#define __CONFIGMANAGER_H__

#include <string>

struct AttriBase;

class ConfigManager
{
public:
    ConfigManager(const std::string& path);
    virtual ~ConfigManager();

public:
    virtual bool LoadConfig() = 0;

protected:
    std::string file_path_;
};

class ServerConfig : public ConfigManager
{
public:
    ServerConfig(const std::string& path);
    virtual ~ServerConfig();

public:
    virtual bool LoadConfig();
};

#endif
