#pragma once

#include <base/copyable.h>
#include <base/StringPiece.h>

#include <netinet/in.h>

class InetAddress : public copyable{
public:
	explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);//port
	InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);//ip + port
	explicit InetAddress(const struct sockaddr_in& addr) //sockaddr_in
		: addr_(addr) {}

	explicit InetAddress(const struct sockaddr_in6& addr) //sockaddr_in6
		: addr6_(addr) {}

	sa_family_t family() const { return addr_.sin_family; }
	string toIp() const;
	string toIpPort() const;
	uint16_t toPort() const;

	const struct sockaddr*    getSockAddr() const {
		return static_cast<const struct sockaddr*>(implicit_cast<const void*>(&addr6_));
	}
	void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

	uint32_t ipNetEndian() const;
	uint16_t portNetEndian() const { return addr_.sin_port; }

	// hostname to IP address
	static bool resolve(StringArg hostname, InetAddress* result);

private:
	union
	{
		struct sockaddr_in addr_;
		struct sockaddr_in6 addr6_;
	};
};
