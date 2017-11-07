
#ifndef NETWORK_CONNECTION_H_
#define NETWORK_CONNECTION_H_
#include "common_base.h"
namespace NetWork {
#if AURORA_OS_WINDOWS == aurora_OS
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#ifndef __SSIZE_T
#define __SSIZE_T
    typedef int ssize_t;
#endif // __SSIZE_T
    #define SHUT_RD SD_RECEIVE
    #define SHUT_WR SD_SEND
    #define SHUT_RDWR SD_BOTH

    const int SIO_KEEPALIVE_VALS = IOC_IN | IOC_VENDOR | 4;

    struct StTcpKeepalive
    {
        uint32_t onoff;
        uint32_t keepalivetime;
        uint32_t keepaliveinterval;
    };
#endif
/**
* The communication domain used to resolve an address.
*/
enum EnmAddressFamily
{
    enmAddressFamily_UNSPEC = AF_UNSPEC,     /// Unspecified address family
    enmAddressFamily_UNIX = AF_UNIX,       /// Local communication
    enmAddressFamily_INET = AF_INET,       /// Internet Protocol version 4
    enmAddressFamily_IPX = AF_IPX,        /// Novell IPX
    enmAddressFamily_APPLETALK = AF_APPLETALK,  /// AppleTalk
    enmAddressFamily_INET6 = AF_INET6,      /// Internet Protocol version 6
};

/**
* Communication semantics
*/
enum EnmSocketType
{
    enmSocketType_STREAM = SOCK_STREAM,           /// Sequenced, reliable, two-way communication-based byte streams
    enmSocketType_DGRAM = SOCK_DGRAM,            /// Connectionless, unreliable datagrams with a fixed maximum length; data may be lost or arrive out of order
    enmSocketType_RAW = SOCK_RAW,              /// Raw protocol access
    enmSocketType_RDM = SOCK_RDM,              /// Reliably-delivered message datagrams
    enmSocketType_SEQPACKET = SOCK_SEQPACKET,        /// Sequenced, reliable, two-way connection-based datagrams with a fixed maximum length
};
/**
* Protocol
*/
enum EnmProtocolType
{
    enmProtocolType_IP = IPPROTO_IP,         /// Internet Protocol version 4
    enmProtocolType_ICMP = IPPROTO_ICMP,       /// Internet Control Message Protocol
    enmProtocolType_IGMP = IPPROTO_IGMP,       /// Internet Group Management Protocol
    //GGP =   IPPROTO_GGP,        /// Gateway to Gateway Protocol
    enmProtocolType_TCP = IPPROTO_TCP,        /// Transmission Control Protocol
    enmProtocolType_PUP = IPPROTO_PUP,        /// PARC Universal Packet Protocol
    enmProtocolType_UDP = IPPROTO_UDP,        /// User Datagram Protocol
    enmProtocolType_IDP = IPPROTO_IDP,        /// Xerox NS protocol
    enmProtocolType_RAW = IPPROTO_RAW,        /// Raw IP packets
    enmProtocolType_IPV6 = IPPROTO_IPV6,       /// Internet Protocol version 6
};
/// The level at which a socket option is defined:
enum ESocketOptLevel
{
    enmSocketOptionLevel_SOCKET = SOL_SOCKET,               /// Socket level
    enmSocketOptionLevel_IP = enmProtocolType_IP,          /// Internet Protocol version 4 level
    enmSocketOptionLevel_ICMP = enmProtocolType_ICMP,        /// Internet Control Message Protocol level
    enmSocketOptionLevel_IGMP = enmProtocolType_IGMP,        /// Internet Group Management Protocol level
    //enmSocketOptionLevel_GGP =     enmProtocolType_GGP,         /// Gateway to Gateway Protocol level
    enmSocketOptionLevel_TCP = enmProtocolType_TCP,         /// Transmission Control Protocol level
    enmSocketOptionLevel_PUP = enmProtocolType_PUP,         /// PARC Universal Packet Protocol level
    enmSocketOptionLevel_UDP = enmProtocolType_UDP,         /// User Datagram Protocol level
    enmSocketOptionLevel_IDP = enmProtocolType_IDP,         /// Xerox NS protocol level
    enmSocketOptionLevel_RAW = enmProtocolType_RAW,         /// Raw IP packet level
    enmSocketOptionLevel_IPV6 = enmProtocolType_IPV6,        /// Internet Protocol version 6 level
};
/// Specifies a socket option:
enum ESocketOpt
{
    enmSocketOption_SO_DEBUG = SO_DEBUG,            /// Record debugging information
    enmSocketOption_SO_BROADCAST = SO_BROADCAST,        /// Allow transmission of broadcast messages
    enmSocketOption_SO_REUSEADDR = SO_REUSEADDR,        /// Allow local reuse of address
    enmSocketOption_SO_LINGER = SO_LINGER,           /// Linger on close if unsent data is present
    enmSocketOption_SO_OOBINLINE = SO_OOBINLINE,        /// Receive out-of-band data in band
    enmSocketOption_SO_SNDBUF = SO_SNDBUF,           /// Send buffer size
    enmSocketOption_SO_RCVBUF = SO_RCVBUF,           /// Receive buffer size
    enmSocketOption_SO_DONTROUTE = SO_DONTROUTE,        /// Do not route
    enmSocketOption_SO_SNDTIMEO = SO_SNDTIMEO,         /// Send timeout
    enmSocketOption_SO_RCVTIMEO = SO_RCVTIMEO,         /// Receive timeout
    enmSocketOption_SO_ERROR = SO_ERROR,            /// Retrieve and clear error status
    enmSocketOption_SO_KEEPALIVE = SO_KEEPALIVE,        /// Enable keep-alive packets
    enmSocketOption_SO_ACCEPTCONN = SO_ACCEPTCONN,       /// Listen
    enmSocketOption_SO_RCVLOWAT = SO_RCVLOWAT,         /// Minimum number of input bytes to process
    enmSocketOption_SO_SNDLOWAT = SO_SNDLOWAT,         /// Minimum number of output bytes to process
    enmSocketOption_SO_TYPE = SO_TYPE,             /// Socket type

    // SocketOptionLevel.TCP:
    enmSocketOption_TCP_NODELAY = TCP_NODELAY,        /// Disable the Nagle algorithm for send coalescing

#if AURORA_OS_WINDOWS != aurora_OS
    //enmSocketOption_TCP_KEEPIDLE =            TCP_KEEPIDLE,
    enmSocketOption_TCP_KEEPINTVL = TCP_KEEPINTVL,
#endif

    // SocketOptionLevel.IPV6:
    enmSocketOption_IPV6_UNICAST_HOPS = IPV6_UNICAST_HOPS,          /// IP unicast hop limit
    enmSocketOption_IPV6_MULTICAST_IF = IPV6_MULTICAST_IF,          /// IP multicast interface
    enmSocketOption_IPV6_MULTICAST_LOOP = IPV6_MULTICAST_LOOP,        /// IP multicast loopback
    enmSocketOption_IPV6_MULTICAST_HOPS = IPV6_MULTICAST_HOPS,        /// IP multicast hops
    enmSocketOption_IPV6_JOIN_GROUP = IPV6_JOIN_GROUP,            /// Add an IP group membership
    enmSocketOption_IPV6_LEAVE_GROUP = IPV6_LEAVE_GROUP,           /// Drop an IP group membership
    enmSocketOption_IPV6_V6ONLY = IPV6_V6ONLY,                /// Treat wildcard bind as AF_INET6-only
};

/// How a socket is shutdown:
enum EnmSocketShutdown
{
    enmSocketShutdown_RECEIVE = SHUT_RD,      /// socket receives are disallowed
    enmSocketShutdown_SEND = SHUT_WR,         /// socket sends are disallowed
    enmSocketShutdown_BOTH = SHUT_RDWR,         /// both RECEIVE and SEND
};
/// Flags may be OR'ed together:
enum ESocketFlags
{
    enmSocketFlags_NONE = 0,                 /// no flags specified

    enmSocketFlags_OOB = MSG_OOB,           /// out-of-band stream data
    enmSocketFlags_PEEK = MSG_PEEK,          /// peek at incoming data without removing it from the queue, only for receiving
    enmSocketFlags_DONTROUTE = MSG_DONTROUTE,     /// data should not be subject to routing; this flag may be ignored. Only for sending
};
enum
{
    WINSOCK_TIMEOUT_SKEW = 500,
    Invalid_SocketID = -1,

#ifndef ADDR_ANY
    ADDR_ANY = INADDR_ANY,         /// Any IPv4 host address.
#endif
    ADDR_NONE = INADDR_NONE,       /// An invalid IPv4 host address.
    PORT_ANY = 0,                /// Any IPv4 port number.
};
//connection status
enum EnmConnectionStatus
{
    enmConnectionStatus_Closed = 0,
    enmConnectionStatus_Opened = 1,
    enmConnectionStatus_Connected = 2,
    enmConnectionStatus_Error = 3,
};
int LastErrorNo();
class Socket;
typedef int socket_t;
struct AddressInfo
{
    EnmAddressFamily family;   /// Address _family
    EnmSocketType type;        /// Socket _type
    EnmProtocolType protocol;  /// Protocol
    sockaddr_in address;        /// Socket _address
    //string canonicalName;   /// Canonical name, when $(D AddressInfoFlags.CANONNAME) is used.
};
class Address
{
public:
    virtual void updateAddressAndPort(const std::string& szAddrStr, uint16_t iPort) = 0;
    virtual void updateAddressAndPort(Address* addr) = 0;
    virtual bool isValid() { return false; }
    virtual bool isIPv6() const { return false; }
    /// Returns pointer to underlying $(D sockaddr) structure.
    virtual sockaddr* name() = 0;
    virtual const sockaddr* name() const = 0;
    /// Returns actual size of underlying $(D sockaddr) structure.
    virtual socklen_t nameLen() const = 0;
    /// Family of this address.
    EnmAddressFamily addressFamily() const;
    std::string toAddrString() const;
    std::string toHostNameString() const;
    std::string toPortString() const;
    std::string toServiceNameString() const;
    std::string toString() const;
protected:
    virtual void _updateAddressAndPort(const sockaddr_in& stSockaddrIn) = 0;
    virtual void _updateAddressAndPort(const sockaddr_in6& stSockaddrIn6) = 0;

private:
    std::string toHostString(bool numeric) const;
    std::string toServiceString(bool numeric) const;
};
class InternetAddress : public Address
{
public:
    InternetAddress();
    InternetAddress(const std::string& szAddrStr, uint16_t port);
    
    virtual void updateAddressAndPort(const std::string& addr, uint16_t iPort);
    virtual void updateAddressAndPort(Address* addr);

    virtual bool isValid();
    virtual bool isIPv6() const;

    sockaddr* name();
    const sockaddr* name() const;
    socklen_t nameLen() const;
    uint16_t port() const;
    uint32_t addr() const;
    std::string toAddrString() const;
    std::string toPortString() const;
    std::string toHostNameString() const;

    static uint32_t parse(char* addr);
    static std::string addrToString(uint32_t addr);
protected:
    virtual void _updateAddressAndPort(const sockaddr_in& stSockaddrIn);
    virtual void _updateAddressAndPort(const sockaddr_in6& stSockaddrIn6);
protected:
    sockaddr_in m_stSockAddrIn;
    sockaddr_in6 m_stSockAddrIn6;
    bool m_bValid;
    bool m_bIPv6;
};
class SocketSet
{
public:
    SocketSet(uint32_t max);
    /// Uses the default capacity for the system.
    SocketSet();

    int GetCount();
    void Reset();
    bool Add(socket_t s);
    bool Add(Socket& stSocket);
    bool Remove(socket_t s);
    bool Remove(Socket& stSocket);
    int IsSet(socket_t s) const;
    int IsSet(Socket& stSocket) const;
    uint32_t Max() const;
    fd_set* GetFdSetPtr();
    int Selectn() const;

private:
    int fdsetMax;
    fd_set setData;
    int maxfd;
    uint32_t count;
};
class Socket 
{
protected:
    Socket();
    void SetSock(socket_t iSocketID);
public:
    Socket(EnmAddressFamily af, EnmSocketType type, EnmProtocolType protocol);
    Socket(EnmAddressFamily af, EnmSocketType type);
    Socket(const AddressInfo& stAddrInfo);
    Socket(socket_t iSocketID, EnmAddressFamily af);
    ~Socket();
    socket_t GetSocketID() const;
    bool IsBlocking() const;
    void SetBlocking(bool isBlocking);
    void SetAddressFamily(EnmAddressFamily af);
    EnmAddressFamily GetAddressFamily();
    bool IsAlive() const;
    int Bind(const Address& stSocketAddress);
    void Connect(const Address& stSocketAddress);
    int Listen(int iMaxPendingConn);
    void Shutdown(EnmSocketShutdown enmShutDown);
    static void CloseSocketID(socket_t iSocketID);
    static std::string GetHostName();
    int32_t GetRemoteAddress(const sockaddr_in& stSockAddrIn);
    int32_t GetLocalAddress(const sockaddr_in& stSockAddrIn);
    ssize_t Send(void* szCodeBuffer, size_t iCodeBufferSize, ESocketFlags enmSockFlags);
    ssize_t Send(void* szCodeBuffer, size_t iCodeBufferSize);
    ssize_t SendTo(void* szCodeBuffer, size_t iCodeBufferSize, ESocketFlags flags, const sockaddr_in& stSockAddrIn);
    ssize_t SendTo(void* szCodeBuffer, size_t iCodeBufferSize, const sockaddr_in& stSockAddrIn);
    ssize_t SendTo(void* szCodeBuffer, size_t iCodeBufferSize, ESocketFlags flags);
    ssize_t SendTo(void* szCodeBuffer, size_t iCodeBufferSize);
    ssize_t Receive(void* szCodeBuffer, size_t iCodeBufferSize, ESocketFlags flags);
    ssize_t Receive(void* szCodeBuffer, size_t iCodeBufferSize);
    ssize_t ReceiveFrom(void* szCodeBuffer, size_t iCodeBufferSize, ESocketFlags flags, const sockaddr_in& stSockAddrIn);
    ssize_t ReceiveFrom(void* szCodeBuffer, size_t iCodeBufferSize, const sockaddr_in& stSockAddrIn);
    ssize_t ReceiveFrom(void* szCodeBuffer, size_t iCodeBufferSize, ESocketFlags flags);
    ssize_t ReceiveFrom(void* szCodeBuffer, size_t iCodeBufferSize);
    int GetOption(ESocketOptLevel level, ESocketOpt option, void* szResultBuffer, socklen_t& iResultBufferSize);
    int GetOption(ESocketOptLevel level, ESocketOpt option, int32_t& result);
    int GetOption(ESocketOptLevel level, ESocketOpt option, linger& result);
    int GetOption(ESocketOptLevel level, ESocketOpt option, timeval& result);
    int SetOption(ESocketOptLevel level, ESocketOpt option, void* szResultBuffer, socklen_t iResultBufferSize);
    int SetOption(ESocketOptLevel level, ESocketOpt option, int32_t value);
    int SetOption(ESocketOptLevel level, ESocketOpt option, bool value);
    int SetOption(ESocketOptLevel level, ESocketOpt option, linger& value);
    int SetOption(ESocketOptLevel level, ESocketOpt option, timeval& value);
    std::string GetErrorText();
    void SetKeepAlive(int iTime, int iInterval);
    static int Select(SocketSet& checkRead, SocketSet& checkWrite, SocketSet& checkError);
    static int Select(SocketSet& checkRead, SocketSet& checkWrite, SocketSet& checkError, timeval* timeout);
    static int Select(SocketSet& checkRead, SocketSet& checkWrite, SocketSet& checkError, long microseconds);

protected:
    void Close();
private:
    socket_t m_iSocketID;
    EnmAddressFamily m_enmAddrFamily;
    /// Property to get or set whether the socket is blocking or nonblocking.
    bool m_isBlocking;

};
}
#endif NETWORK_CONNECTION_H_