#include <libconfig.h++>
#include "utils/logging.h"
#include "configmanager.h"

using namespace libconfig;

ServerConfig::ServerConfig(const std::string& path) :
	config_file_path_(path)
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
		cfg.readFile(config_file_path_.c_str());
    }
    catch (const std::exception &e)
    {
        log(LOG_FATAL, "config error %s", e.what());
        return false;
    }
    
	if (!cfg.lookupValue("version", version_)) {
		log(LOG_ERR, "wolfcc have no version");
	}

	if (!cfg.lookupValue("log.log_file", log_file_) || !cfg.lookupValue("log.log_level", log_level_)) {
		log(LOG_ERR, "log have no file or level");
	}

	Log::GetInstance().SetLogLevel(log_level_);
	Log::GetInstance().SetLogName(log_file_);

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
