#include <net/InetAddress.h>
#include <log/Logger.h>
#include <net/SocketsOps.h>

#include <netdb.h>
#include <netinet/in.h>

//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6)
{
	if (ipv6){
		memZero(&addr6_, sizeof addr6_);
		addr6_.sin6_family = AF_INET6;
		in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
		addr6_.sin6_addr = ip;
		addr6_.sin6_port = htobe16(port);
	}
	else{
		memZero(&addr_, sizeof addr_);
		addr_.sin_family = AF_INET;
		in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
		addr_.sin_addr.s_addr = htobe32(ip);
		addr_.sin_port = htobe16(port);
	}
}

InetAddress::InetAddress(StringArg ip, uint16_t port, bool ipv6){
	if (ipv6){
		memZero(&addr6_, sizeof addr6_);
		sockets::fromIpPort(ip.c_str(), port, &addr6_);
	}
	else{
		memZero(&addr_, sizeof addr_);
		sockets::fromIpPort(ip.c_str(), port, &addr_);
	}
}

string InetAddress::toIpPort() const{
	char buf[64] = "";
	sockets::toIpPort(buf, sizeof buf, getSockAddr());
	return buf;
}

string InetAddress::toIp() const{
	char buf[64] = "";
	sockets::toIp(buf, sizeof buf, getSockAddr());
	return buf;
}

uint32_t InetAddress::ipNetEndian() const{
	assert(family() == AF_INET);
	return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::toPort() const{
	return be16toh(portNetEndian());
}

static __thread char t_resolveBuffer[64 * 1024];

/*
struct hostent
{
char *h_name;         //正式主机名
char **h_aliases;     //主机别名
int h_addrtype;       //主机IP地址类型：IPV4-AF_INET
int h_length;         //主机IP地址字节长度，对于IPv4是四字节，即32位
char **h_addr_list;   //主机的IP地址列表
};
*/
bool InetAddress::resolve(StringArg hostname, InetAddress* out){
	assert(out != NULL);
	struct hostent hent;
	struct hostent* he = NULL;
	int herrno = 0;
	memZero(&hent, sizeof(hent));

	int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
	if (ret == 0 && he != NULL){
		assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
		out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
		return true;
	}
	else{
		if (ret)
			LOG_ERROR << "InetAddress::resolve";
		return false;
	}
}

