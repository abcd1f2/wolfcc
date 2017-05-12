#ifndef __WOLFCC_UNIXADDR_H__
#define __WOLFCC_UNIXADDR_H__
#include <string>
#include <sys/un.h>

class UnixAddr
{
public:
	UnixAddr(const std::string & sPath);
	UnixAddr(const struct sockaddr_un * pUnixAddr);
	
public:
	inline const std::string & GetPath() const { return sPath_; }
	std::string ToString() const;
	
private:
	std::string sPath_;
};

#endif
