#include <ostream>
#include <string.h>
#include <algorithm>
#include <arpa/inet.h>
#include <assert.h>
#include "nets/sockaddr.h"

#define ADDR_PREFIX_INET "inet://"
#define ADDR_PREFIX_LOCAL "local://"
#define ADDR_PREFIX_FILE "file://"
#define ADDR_LOCALHOST "127.0.0.1"

SockAddr::SockAddr()
	:	sString_(), iLen_(0), stAddr_()
{
	memset(&stAddr_, 0, sizeof(stAddr_));
}

SockAddr::SockAddr(const sockaddr * pSockaddr)
	:	sString_(), iLen_(0), stAddr_()
{
	Assign(pSockaddr);
}

SockAddr::SockAddr(const INetAddr & stINetAddr)
	:	sString_(), iLen_(0), stAddr_()
{
	Assign(stINetAddr);
}

SockAddr::SockAddr(const UnixAddr & stUnixAddr)
	:	sString_(), iLen_(0), stAddr_()
{
	Assign(stUnixAddr);
}

void SockAddr::Assign(const sockaddr * pSockaddr)
{
	if ( pSockaddr->sa_family == AF_INET ) {
		Assign(INetAddr((const sockaddr_in *)pSockaddr));
	}
	else if ( pSockaddr->sa_family == AF_LOCAL ) {
		Assign(UnixAddr((const sockaddr_un *)pSockaddr));
	}
	else {
		assert(0);
	}
}

void SockAddr::Assign(const INetAddr & stINetAddr)
{
	memset(&stAddr_, 0, sizeof(stAddr_));
	
	stAddr_.in_4.sin_family = AF_INET;
	stAddr_.in_4.sin_port = htons(stINetAddr.GetPort());
	inet_pton(AF_INET, stINetAddr.GetIP().c_str(), &stAddr_.in_4.sin_addr);	

	iLen_ = sizeof(stAddr_.in_4);
	sString_ = stINetAddr.ToString();
}

void SockAddr::Assign(const UnixAddr & stUnixAddr)
{
	memset(&stAddr_, 0, sizeof(stAddr_));	

	stAddr_.unix_domain.sun_family = AF_LOCAL;
	strncpy(stAddr_.unix_domain.sun_path, stUnixAddr.GetPath().c_str(), sizeof(stAddr_.unix_domain.sun_path) - 1);
	iLen_ = sizeof(stAddr_.unix_domain);
	sString_ = stUnixAddr.ToString();
}

int SockAddr::ToINetAddr(INetAddr & stINetAddr) const
{
	if ( GetFamily() != AF_INET ) {
		return -1;
	}

	stINetAddr = INetAddr((const sockaddr_in*)GetSockAddrPtr());
	return 0;
}

int SockAddr::ToUnixAddr(UnixAddr & stUnixAddr) const
{
	if ( GetFamily() != AF_LOCAL ) {
		return -1;
	}

	stUnixAddr = UnixAddr((const sockaddr_un*)GetSockAddrPtr());
	return 0;
}

bool SockAddr::operator<(const SockAddr & OtherSockAddr) const
{
	return memcmp(&stAddr_, &OtherSockAddr.stAddr_, sizeof(stAddr_)) < 0;
}

bool SockAddr::operator==(const SockAddr & OtherSockAddr) const
{
	return memcmp(&stAddr_, &OtherSockAddr.stAddr_, sizeof(stAddr_)) == 0;
}

SockAddr ParseAddress(const char* addr, const char* bindhost)
{
	assert(addr);
	if (strncmp(addr, ADDR_PREFIX_INET, strlen(ADDR_PREFIX_INET)) == 0) {
		addr += strlen(ADDR_PREFIX_INET);

		const char* host = addr;
		unsigned short port = 0;

		const char* pos = NULL;
		if ((pos = (const char*)strstr(addr, ":")) != NULL) {
			port = atoi(pos + 1);
			return INetAddr(std::string(host, pos).c_str(), port);
		}
		else {
			if (bindhost) {
				host = bindhost;
			}
			else {
				host = ADDR_LOCALHOST;
			}

			port = atoi(addr);
			return INetAddr(host, port);
		}
	}
	else if (strncmp(addr, ADDR_PREFIX_LOCAL, strlen(ADDR_PREFIX_LOCAL)) == 0) {
		return UnixAddr(addr + strlen(ADDR_PREFIX_LOCAL));
    }
	else if (strncmp(addr, ADDR_PREFIX_FILE, strlen(ADDR_PREFIX_FILE)) == 0) {
		return UnixAddr(addr + strlen(ADDR_PREFIX_FILE));
	}
	else {
        return UnixAddr(addr);
	}
}

std::string FormatAddress(const SockAddr& addr)
{
	std::string result = addr.ToString();
	if (addr.GetFamily() == AF_LOCAL) {
		return ADDR_PREFIX_LOCAL + result;
	}
	else {
		return ADDR_PREFIX_INET + result;
	}
}
