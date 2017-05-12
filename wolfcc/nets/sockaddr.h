#ifndef __WOLFCC_SOCKADDR_H__
#define __WOLFCC_SOCKADDR_H__
#include <ostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include "nets/inetaddr.h"
#include "nets/unixaddr.h"

class SockAddr
{
public:
	SockAddr();

	SockAddr(const sockaddr * pSockaddr);

	SockAddr(const INetAddr & stINetAddr);

	SockAddr(const UnixAddr & stUnixAddr);	

	void Assign(const sockaddr * pSockaddr);

	void Assign(const INetAddr & stINetAddr);

	void Assign(const UnixAddr & stUnixAddr);

	inline const struct sockaddr * GetSockAddrPtr() const
		{ return &stAddr_.generic; }

	inline socklen_t GetSockAddrSize() const
		{ return iLen_; }

	inline int GetFamily() const
		{ return stAddr_.generic.sa_family; }

	const std::string & ToString() const
		{ return sString_; }


	int ToINetAddr(INetAddr &) const;

	int ToUnixAddr(UnixAddr &) const;


	// for stl container
	bool operator<(const SockAddr &) const;

	// for stl container
	bool operator==(const SockAddr &) const;
	bool operator!=(const SockAddr & stSockAddr) const { return !(*this==stSockAddr); }

private:
	std::string sString_;

	socklen_t iLen_;

	union {
		sockaddr generic;
		sockaddr_in  in_4;
		sockaddr_in6 in_6;
		sockaddr_un unix_domain;
	} stAddr_;
};

/**
 * Parse socket address
 * "inet://9999" => "inet://127.0.0.1:9999"
 * "inet://localhost:9999"
 * "inet://127.0.0.1:9999"
 * "local:///tmp/test.sock"
 * "local://foo"
 *
 * When inet's host not griven, bindhost be used.
 */
SockAddr ParseAddress(const char* addr, const char* bindhost=NULL);

std::string FormatAddress(const SockAddr& addr);

#endif
