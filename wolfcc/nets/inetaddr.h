#ifndef __WOLFCC_INETADDR_H__
#define __WOLFCC_INETADDR_H__
#include <string>
#include <arpa/inet.h>

#define ANY_ADDR Instance()
    
class INetAddr
{
public:
	static INetAddr & Instance();

public:

	INetAddr(unsigned short uPort=0);

	INetAddr(const std::string & sIP, unsigned short uPort);

	INetAddr(const struct sockaddr_in * stSockAddr);

	
	inline const std::string & GetHost() const { return sHost_; }

	inline const std::string & GetIP() const { return sIP_; }

	inline unsigned short GetPort() const { return uPort_; }

	std::string ToString() const;

	bool operator==(const INetAddr & o) const;

	bool operator!=(const INetAddr & o) const;

public:	
	static std::string GetIPByHostName(const std::string & sHost);

private:
	std::string sHost_;
	std::string sIP_;
	unsigned short uPort_;

	static INetAddr * instance_;
};

#endif
