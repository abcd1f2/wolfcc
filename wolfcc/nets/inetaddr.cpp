#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <sstream>
#include "nets/inetaddr.h"

using namespace std;

INetAddr * INetAddr::instance_=NULL;

INetAddr & INetAddr::Instance()
{
	if (!instance_) {
		instance_ = new INetAddr;
	}
	return *instance_;	
}

INetAddr::INetAddr(unsigned short uPort)
	: sHost_("0.0.0.0"), sIP_("0.0.0.0"), uPort_(uPort)
{
}

INetAddr::INetAddr(const std::string & sIP, unsigned short uPort)
	: sHost_(sIP), uPort_(uPort)
{
	sIP_ = GetIPByHostName(sHost_);
}

INetAddr::INetAddr(const struct sockaddr_in * pSockAddr)
{
	char szIP[16];
	sIP_ = inet_ntop(AF_INET, &pSockAddr->sin_addr, szIP, sizeof(szIP))==NULL?"0.0.0.0":szIP; 
	uPort_ = ntohs(pSockAddr->sin_port);
	sHost_ = sIP_;
}

std::string INetAddr::ToString() const
{
	ostringstream out;
	out << GetHost() << ":" << GetPort();
	return out.str();
}

std::string INetAddr::GetIPByHostName(const std::string & sHost)
{
	string sIP = sHost;

	if ( sIP != "0.0.0.0" ) {

		struct hostent ret;
		struct hostent * result = &ret;
		char buf[1024];
		int errop;

#if defined(__sun__)
		if ( gethostbyname_r(sHost.c_str(), &ret, buf, sizeof(buf), &errop) != 0 )
#else
		if ( gethostbyname_r(sHost.c_str(), &ret, buf, sizeof(buf), &result, &errop) != 0 )
#endif // LINUX
		{
			sIP = "0.0.0.0";
		}
		else {
			if ( result->h_addr_list[0] == NULL ) {
				sIP = "0.0.0.0";
			}
			else {
				char tmp[16];
				sIP = inet_ntop(AF_INET, result->h_addr_list[0], tmp, sizeof(tmp));
			}
		}
	}

	return sIP;
}

bool INetAddr::operator==(const INetAddr& o) const
{
	return sHost_ == o.sHost_ && sIP_ == o.sIP_ && uPort_ == o.uPort_;
}

bool INetAddr::operator!=(const INetAddr & o) const
{
	return !(*this == o);
}
