#include <string>
#include "nets/unixaddr.h"

UnixAddr::UnixAddr(const std::string & sPath)
	: sPath_(sPath)
{
}

UnixAddr::UnixAddr(const struct sockaddr_un * pUnixAddr)
	: sPath_(pUnixAddr->sun_path)
{
}
	
std::string UnixAddr::ToString() const
{
	return GetPath();
}

