#include <libconfig.h++>
#include "configmanager.h"
#include "attrib.h"
#include "utils/logging.h"

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
        log(LOG_FATAL, "config error %s", e.what());
        return false;
    }
    
    version_ = cfg.lookup("version");

    log_file_ = cfg.lookup("log.log_file");
    log_level_ = cfg.lookup("log.log_level");

	Log::SetLogLevel(log_level_);
	Log::SetLogName(log_file_);

    const Setting &server = cfg.lookup("server");
    int c = server.getLength();
    for (int i = 0; i < c; i++) {
		if (!(server[i].lookupValue("tcp_inet", server_tcp_addr_) || 
			server[i].lookupValue("unix_inet", server_unix_addr_))) {
            log(LOG_FATAL, "read tcp and unix error");
            return false;
        }
    }
	log(LOG_DEBUG, "tcp %s, unix %s", server_tcp_addr_.c_str(), server_unix_addr_.c_str());

    return true;
}
