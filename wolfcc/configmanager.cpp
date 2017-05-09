#include <libconfig.h++>
#include "configmanager.h"
#include "attrib.h"
#include "logging.h"

using namespace libconfig;

ConfigManager::ConfigManager(const std::string& path) :
    file_path_(path)
{
    
}

ConfigManager::~ConfigManager()
{

}


ServerConfig::ServerConfig(const std::string& path) :
    ConfigManager(path)
{

}

ServerConfig::~ServerConfig()
{

}

bool ServerConfig::LoadConfig()
{
    Config cfg;
    try
    {
        cfg.readFile(file_path_);
    }
    catch (const std::exception &e)
    {
        LOG(LOG_FATAL, "config error %s", e.what());
        return false;
    }
    
    g_wolfserver.version = cfg.lookup("version");

    g_wolfserver.log_file = cfg.lookup("log.log_file");
    g_wolfserver.log_level = cfg.lookup("log.log_level");

    const Setting &server = cfg.lookup("server");
    int c = server.getLength();
    for (int i = 0; i < c; i++) {
        std::string add;
        int port;
        if (!server[i].lookupValue("addr", add) ||
            !server[i].lookupValue("port", port)) {
            LOG(LOG_FATAL, "read addr and port error");
            return false;
        }

        g_wolfserver.bindaddr.push_back(add);
        g_wolfserver.port.push_back(port);
        g_wolfserver.bindaddr_count++;
    }

    return true;
}
