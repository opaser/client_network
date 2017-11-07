#include "connection.h"
#include "common_string.h"
namespace NetWork
{
int LastErrorNo()
{
#if AURORA_OS_WINDOWS == aurora_OS
    return WSAGetLastError();
#else
    //TODO: 跨平台处理
    return errno;
#endif
}
EnmAddressFamily Address::addressFamily() const
{
    return (EnmAddressFamily)name()->sa_family;
}

std::string Address::toAddrString() const
{
    return toHostString(true);
}

std::string Address::toHostNameString() const
{
    return toHostString(false);
}

std::string Address::toPortString() const
{
    return toServiceString(true);
}

std::string Address::toServiceNameString() const
{
    return toServiceString(false);
}

std::string Address::toString() const
{
    std::string host = toAddrString();
    std::string port = toPortString();
    // TODO joewan
    if (host.find(':') >= 0)
    {
        return "[" + host + "]:" + port;
    }
    else
    {
        return host + ":" + port;
    }
}

std::string Address::toHostString(bool numeric) const
{
    char buf[NI_MAXHOST];
    int ret = getnameinfo(name(), nameLen(), buf, NI_MAXHOST, NULL, 0, numeric ? NI_NUMERICHOST : NI_NAMEREQD);

    if (!numeric)
    {
        if (ret == EAI_NONAME)
        {
            return "";
        }
    }

    if (0 != ret)
    {
        return "";
    }
    return std::string(buf);
}

std::string Address::toServiceString(bool numeric) const
{
    char buf[NI_MAXSERV];
    int32_t ret = getnameinfo(name(), nameLen(), NULL, 0, buf, sizeof(buf) / sizeof(buf[0]), numeric ? NI_NUMERICSERV : NI_NAMEREQD);

    if (0 != ret)
    {
        return "";
    }
    
    return std::string(buf);
}


InternetAddress::InternetAddress()
{
    m_bValid = false;
}

InternetAddress::InternetAddress(const std::string& szAddrStr, uint16_t port)
{

    m_bValid = false;
    updateAddressAndPort(szAddrStr, port);
}

void InternetAddress::updateAddressAndPort(const std::string& szAddrStr, uint16_t port)
{
#if AURORA_OS_WINDOWS == aurora_OS
    WSADATA wsaData;
    int n = WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    m_bValid = false;
    m_bIPv6 = false;
    char* addr = (char*)szAddrStr.c_str();

    struct addrinfo *answer, hint, *curr;
    bzero(&hint, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    struct sockaddr_in  *sockaddr_ipv4 = nullptr;
    struct sockaddr_in6 *sockaddr_ipv6 = nullptr;

    int ret = getaddrinfo(addr, NULL, &hint, &answer);
    
    if (ret != 0)
    {
        ///cx log
        return;
    }

    bool bIpv6Flag = false;

    for (curr = answer; curr != NULL; curr = curr->ai_next)
    {
        switch (curr->ai_family)
        {
        case AF_UNSPEC:
        {
            //do something here
            ///cx log
            m_bValid = false;
            bIpv6Flag = false;
        }
        break;
        case AF_INET:
        {
            sockaddr_ipv4 = reinterpret_cast<struct sockaddr_in *>(curr->ai_addr);
            memset(&m_stSockAddrIn, 0, sizeof(m_stSockAddrIn));
            m_stSockAddrIn.sin_family = enmAddressFamily_INET;
            m_stSockAddrIn.sin_addr = sockaddr_ipv4->sin_addr;
            m_stSockAddrIn.sin_port = htons(port);
            m_bValid = true;

        }
        break;
        case AF_INET6:
        {
            sockaddr_ipv6 = reinterpret_cast<struct sockaddr_in6 *>(curr->ai_addr);
            memset(&m_stSockAddrIn6, 0, sizeof(m_stSockAddrIn6));
            m_stSockAddrIn6.sin6_family = enmAddressFamily_INET6;
            m_stSockAddrIn6.sin6_addr = sockaddr_ipv6->sin6_addr;
            m_stSockAddrIn6.sin6_port = htons(port);
            m_bValid = true;
            bIpv6Flag = true;
        }
        break;
        }
    }

    freeaddrinfo(answer);
    m_bIPv6 = bIpv6Flag;
}

void InternetAddress::updateAddressAndPort(Address* addr)
{
    m_bValid = true;
    m_bIPv6 = addr->isIPv6();
    if (m_bIPv6)
    {
        sockaddr_in6* pstTargetAddIn = (sockaddr_in6*)addr->name();
        _updateAddressAndPort(*pstTargetAddIn);
    }
    else
    {
        sockaddr_in* pstTargetAddIn = (sockaddr_in*)addr->name();
        _updateAddressAndPort(*pstTargetAddIn);
    }
}

bool InternetAddress::isValid()
{
    return m_bValid;
}
bool InternetAddress::isIPv6() const
{
    return m_bIPv6;
}

sockaddr* InternetAddress::name()
{
    if (m_bIPv6)
    {
        return (sockaddr*)&m_stSockAddrIn6;
    }
    else
    {
        return (sockaddr*)&m_stSockAddrIn;
    }
}

const sockaddr* InternetAddress::name() const
{
    if (m_bIPv6)
    {
        return (const sockaddr*)&m_stSockAddrIn6;
    }
    else
    {
        return (const sockaddr*)&m_stSockAddrIn;
    }
}

socklen_t InternetAddress::nameLen() const
{
    if (m_bIPv6)
    {
        return (socklen_t) sizeof(m_stSockAddrIn6);
    }
    else
    {
        return (socklen_t) sizeof(m_stSockAddrIn);
    }
}
/// Returns the IPv4 _port number (in host byte order).
uint16_t InternetAddress::port() const
{
    return ntohs(m_stSockAddrIn.sin_port);
}
/// Returns the IPv4 address number (in host byte order).
uint32_t InternetAddress::addr() const
{
    return ntohl(m_stSockAddrIn.sin_addr.s_addr);
}

std::string InternetAddress::toAddrString() const
{
    return std::string(inet_ntoa(m_stSockAddrIn.sin_addr));
}
/// Human readable std::string representing the IPv4 port.
std::string InternetAddress::toPortString() const
{
    char szPortStr[32] = { 0 };
    snprintf(szPortStr, sizeof(szPortStr) - 1, "%d", port());

    return std::string(szPortStr);
}
std::string InternetAddress::toHostNameString() const
{
    return Address::toHostNameString();
}

uint32_t InternetAddress::parse(char* addr)
{
    return ntohl(inet_addr(addr));
}

std::string InternetAddress::addrToString(uint32_t addr)
{
    in_addr sin_addr;
    sin_addr.s_addr = htonl(addr);

    return std::string(inet_ntoa(sin_addr));
}

void InternetAddress::_updateAddressAndPort(const sockaddr_in& stSockaddrIn)
{
    m_bValid = true;
    m_bIPv6 = false;
    m_stSockAddrIn = stSockaddrIn;
}
void InternetAddress::_updateAddressAndPort(const sockaddr_in6& stSockaddrIn6)
{
    m_bValid = true;
    m_bIPv6 = true;
    m_stSockAddrIn6 = stSockaddrIn6;
}

SocketSet::SocketSet(uint32_t max)
{
    if (max > FD_SETSIZE)
    {
        return;
    }

    fdsetMax = max;
    Reset();
}

SocketSet::SocketSet()
{
    fdsetMax = FD_SETSIZE;
    Reset();
}

int SocketSet::GetCount()
{
    return count;
}

void SocketSet::Reset()
{
    FD_ZERO(&setData);

    maxfd = -1;
    count = 0;
}

bool SocketSet::Add(socket_t s)
{
    if (s >= fdsetMax)
    {
        return false;
    }

    FD_SET(s, &setData);

    ++count;
    if (s > maxfd)
    {
        maxfd = s;
    }

    return true;
}

bool SocketSet::Add(Socket& stSocket)
{
    return Add(stSocket.GetSocketID());
}

bool SocketSet::Remove(socket_t s)
{
    if (s >= fdsetMax)
    {
        return false;
    }

    FD_CLR(s, &setData);
    --count;
    // note: adjusting maxfd would require scanning the set, not worth it

    return true;
}

bool SocketSet::Remove(Socket& stSocket)
{
    return Remove(stSocket.GetSocketID());
}

int SocketSet::IsSet(socket_t s) const
{
    if (s >= fdsetMax)
    {
        return -1;
    }

    return FD_ISSET(s, &setData);
}

int SocketSet::IsSet(Socket& stSocket) const
{
    return IsSet(stSocket.GetSocketID());
}

uint32_t SocketSet::Max() const
{
    return fdsetMax;
}

fd_set* SocketSet::GetFdSetPtr()
{
    return &setData;
}

int SocketSet::Selectn() const
{
    return maxfd + 1;
}
Socket::Socket()
{
    m_isBlocking = false;
    m_iSocketID = Invalid_SocketID;
    m_enmAddrFamily = enmAddressFamily_INET;
}

Socket::Socket(EnmAddressFamily af, EnmSocketType type, EnmProtocolType protocol)
{
    m_enmAddrFamily = af;
    socket_t iSocketID = socket(af, type, protocol);

    if (Invalid_SocketID == iSocketID)
    {
        //cx log
    }

    SetSock(iSocketID);
}

Socket::Socket(EnmAddressFamily af, EnmSocketType type)
{
    m_isBlocking = false;
    m_iSocketID = Invalid_SocketID;
    m_enmAddrFamily = enmAddressFamily_UNSPEC;

    Socket(af, type, enmProtocolType_IP);         // Pseudo protocol number.
}

Socket::Socket(const AddressInfo& stAddrInfo)
{
    m_isBlocking = false;
    m_iSocketID = Invalid_SocketID;
    m_enmAddrFamily = enmAddressFamily_UNSPEC;

    Socket(stAddrInfo.family, stAddrInfo.type, stAddrInfo.protocol);
}

Socket::Socket(socket_t iSocketID, EnmAddressFamily af)
{
    m_isBlocking = false;
    //assert(m_iSocketID != socket_t.init);
    m_iSocketID = iSocketID;
    m_enmAddrFamily = af;
}

void Socket::SetSock(socket_t iSocketID)
{
    m_iSocketID = iSocketID;
}

Socket::~Socket()
{
    Close();
}

socket_t Socket::GetSocketID() const
{
    return m_iSocketID;
}

bool Socket::IsBlocking() const
{
#if (AURORA_OS_ANDROID == aurora_OS) || (AURORA_OS_LINUX == aurora_OS) || (AURORA_OS_IOS == aurora_OS)

    return !(fcntl(m_iSocketID, F_GETFL, 0) & O_NONBLOCK);

#elif (AURORA_OS_WINDOWS == aurora_OS)

    return m_isBlocking;
#endif
}

void Socket::SetBlocking(bool isBlocking)
{
#if (AURORA_OS_ANDROID == aurora_OS) || (AURORA_OS_IOS == aurora_OS) || (AURORA_OS_LINUX == aurora_OS)

    int x = fcntl(m_iSocketID, F_GETFL, 0);
    if (-1 == x)
    {
        //throw new SocketOSException("Unable to set socket blocking");
        return;
    }

    if (isBlocking)
    {
        x &= ~O_NONBLOCK;
    }
    else
    {
        x |= O_NONBLOCK;
    }

    if (-1 == fcntl(m_iSocketID, F_SETFL, x))
    {
        //throw new SocketOSException("Unable to set socket blocking");
        return;
    }

#elif (AURORA_OS_WINDOWS == aurora_OS)

    u_long num = !isBlocking;
    if (SOCKET_ERROR == ioctlsocket(m_iSocketID, FIONBIO, &num))
    {
        //throw new SocketOSException("Unable to set socket blocking");
        return;
    }
    m_isBlocking = isBlocking;
#endif
}

void Socket::SetAddressFamily(EnmAddressFamily af)
{
    m_enmAddrFamily = af;
}

EnmAddressFamily Socket::GetAddressFamily()
{
    return m_enmAddrFamily;
}

bool Socket::IsAlive() const
{
    int type = 0;
    socklen_t typesize = sizeof(type);
    int iCode = getsockopt(m_iSocketID, SOL_SOCKET, SO_ERROR, (char*)&type, &typesize);
    return (iCode == 0);
}
/// Associate a local address with this socket.
int Socket::Bind(const Address& stSocketAddress)
{
    return ::bind(m_iSocketID, stSocketAddress.name(), stSocketAddress.nameLen());
}

void Socket::Connect(const Address& stSocketAddress)
{
    //NET_LOG(INFO) << "Connect IP:" << stSocketAddress.toAddrString() << "| Port:" << stSocketAddress.toPortString();
    //cx log
    int iConnectResult = connect(m_iSocketID, stSocketAddress.name(), stSocketAddress.nameLen());

    if (0 != iConnectResult)
    {
        //NET_LOG(INFO) << "Connect iConnectResult: " << iConnectResult << " errno:" << LastErrorNo() << " | errstr:" << LastErrorString();
        //cx log
        int err = LastErrorNo();

        if (!IsBlocking())
        {
#if (AURORA_OS_ANDROID == aurora_OS) || (AURORA_OS_LINUX == aurora_OS) || (AURORA_OS_IOS == aurora_OS)

            if (EINPROGRESS == err)
                return;

#elif (AURORA_OS_WINDOWS == aurora_OS)

            if (WSAEWOULDBLOCK == err)
                return;
#endif
        }

        //throw new SocketOSException("Unable to connect socket", err);
    }
}
int Socket::Listen(int iMaxPendingConn)
{
    return listen(m_iSocketID, iMaxPendingConn);

    //throw new SocketOSException("Unable to listen on socket");
}

void Socket::Shutdown(EnmSocketShutdown enmShutDown)
{
    shutdown(m_iSocketID, enmShutDown);
}

void Socket::CloseSocketID(socket_t iSocketID)
{
#if (AURORA_OS_ANDROID == aurora_OS) || (AURORA_OS_LINUX == aurora_OS) || (AURORA_OS_IOS == aurora_OS)
    close(iSocketID);
#elif (AURORA_OS_WINDOWS == aurora_OS)
    closesocket(iSocketID);
#endif
}

std::string Socket::GetHostName()
{
    char szResult[256];         // Host names are limited to 255 chars.

    if (0 != gethostname(&szResult[0], sizeof(szResult)))
    {
        return NULL;
    }

    return std::string(szResult);
}

int32_t Socket::GetRemoteAddress(const sockaddr_in& stSockAddrIn)
{
    socklen_t socklen = sizeof(stSockAddrIn);

    return getpeername(m_iSocketID, (sockaddr*)&stSockAddrIn, &socklen);
}

int32_t Socket::GetLocalAddress(const sockaddr_in& stSockAddrIn)
{
    socklen_t socklen = sizeof(stSockAddrIn);

    return getsockname(m_iSocketID, (sockaddr*)&stSockAddrIn, &socklen);
}
ssize_t Socket::Send(void* szCodeBuffer, size_t iCodeBufferSize, ESocketFlags enmSockFlags)
{
    //NET_LOG(INFO) << "Send: m_iSocketID:" << m_iSocketID << "  iCodeBufferSize:" << iCodeBufferSize;
    return send(m_iSocketID, (char*)szCodeBuffer, iCodeBufferSize, enmSockFlags);
}

ssize_t Socket::Send(void* szCodeBuffer, size_t iCodeBufferSize)
{
    return Send(szCodeBuffer, iCodeBufferSize, enmSocketFlags_NONE);
}
ssize_t Socket::SendTo(void* szCodeBuffer, size_t iCodeBufferSize, ESocketFlags flags, const sockaddr_in& stSockAddrIn)
{
    socklen_t socklen = sizeof(stSockAddrIn);
    return sendto(m_iSocketID, (char*)szCodeBuffer, iCodeBufferSize, flags, (const sockaddr*)&stSockAddrIn, socklen);
}

ssize_t Socket::SendTo(void* szCodeBuffer, size_t iCodeBufferSize, const sockaddr_in& stSockAddrIn)
{
    return SendTo(szCodeBuffer, iCodeBufferSize, enmSocketFlags_NONE, stSockAddrIn);
}

//assumes you connect()ed
ssize_t Socket::SendTo(void* szCodeBuffer, size_t iCodeBufferSize, ESocketFlags flags)
{
    return sendto(m_iSocketID, (char*)szCodeBuffer, iCodeBufferSize, flags, NULL, 0);
}

//assumes you connect()ed
ssize_t Socket::SendTo(void* szCodeBuffer, size_t iCodeBufferSize)
{
    return SendTo(szCodeBuffer, iCodeBufferSize, enmSocketFlags_NONE);
}
ssize_t Socket::Receive(void* szCodeBuffer, size_t iCodeBufferSize, ESocketFlags flags)
{

    return (NULL != szCodeBuffer && iCodeBufferSize > 0)
        ? recv(m_iSocketID, (char*)szCodeBuffer, iCodeBufferSize, flags)
        : 0;

}

/// ditto
ssize_t Socket::Receive(void* szCodeBuffer, size_t iCodeBufferSize)
{
    return Receive(szCodeBuffer, iCodeBufferSize, enmSocketFlags_NONE);
}
ssize_t Socket::ReceiveFrom(void* szCodeBuffer, size_t iCodeBufferSize, ESocketFlags flags, const sockaddr_in& stSockAddrIn)
{
    if (NULL == szCodeBuffer || iCodeBufferSize <= 0)         //return 0 and don't think the connection closed
    {
        return 0;
    }

    socklen_t nameLen = sizeof(stSockAddrIn);

    return recvfrom(m_iSocketID, (char*)szCodeBuffer, iCodeBufferSize, flags, (sockaddr*)&stSockAddrIn, &nameLen);
}

ssize_t Socket::ReceiveFrom(void* szCodeBuffer, size_t iCodeBufferSize, const sockaddr_in& stSockAddrIn)
{
    return ReceiveFrom(szCodeBuffer, iCodeBufferSize, enmSocketFlags_NONE, stSockAddrIn);
}

//assumes you connect()ed
ssize_t Socket::ReceiveFrom(void* szCodeBuffer, size_t iCodeBufferSize, ESocketFlags flags)
{

    if (NULL == szCodeBuffer || iCodeBufferSize <= 0)         //return 0 and don't think the connection closed
    {
        return 0;
    }

    return recvfrom(m_iSocketID, (char*)szCodeBuffer, iCodeBufferSize, flags, NULL, NULL);
}

//assumes you connect()ed
ssize_t Socket::ReceiveFrom(void* szCodeBuffer, size_t iCodeBufferSize)
{
    return ReceiveFrom(szCodeBuffer, iCodeBufferSize, enmSocketFlags_NONE);
}
int Socket::GetOption(ESocketOptLevel level, ESocketOpt option,
    void* szResultBuffer, socklen_t& iResultBufferSize)
{
    return getsockopt(m_iSocketID, level, option, (char*)szResultBuffer, &iResultBufferSize);
}

/// Common case of getting integer and boolean options.
int Socket::GetOption(ESocketOptLevel level, ESocketOpt option, int32_t& result)
{
    socklen_t iResultBufferSize = sizeof(result);

    return GetOption(level, option, (void*)&result, iResultBufferSize);
}

/// Get the linger option.
int Socket::GetOption(ESocketOptLevel level, ESocketOpt option, linger& result)
{
    socklen_t iResultBufferSize = sizeof(result);
    //return getOption(cast(EnmSocketOptionLevel)EnmenmSocketOptionLevel_SOCKET, SocketOption.LINGER, (&result)[0 .. 1]);
    return GetOption(level, option, (void*)&result, iResultBufferSize);
}

/// Get a timeout (duration) option.
int Socket::GetOption(ESocketOptLevel level, ESocketOpt option, timeval& result)
{
    socklen_t iResultBufferSize = sizeof(result);
    //return getOption(cast(EnmSocketOptionLevel)EnmenmSocketOptionLevel_SOCKET, SocketOption.LINGER, (&result)[0 .. 1]);
    return GetOption(level, option, (void*)&result, iResultBufferSize);
}

// Set a socket option.
int Socket::SetOption(ESocketOptLevel level, ESocketOpt option, void* szResultBuffer, socklen_t iResultBufferSize)
{
    return setsockopt(m_iSocketID, level, option, (char*)szResultBuffer, iResultBufferSize);
}

/// Common case for setting integer and boolean options.
int Socket::SetOption(ESocketOptLevel level, ESocketOpt option, int32_t value)
{
    socklen_t iBufferSize = sizeof(value);

    return SetOption(level, option, (void*)&value, iBufferSize);
}

int Socket::SetOption(ESocketOptLevel level, ESocketOpt option, bool value)
{
    socklen_t iBufferSize = sizeof(value);

    return SetOption(level, option, (void*)&value, iBufferSize);
}

/// Set the linger option.
int Socket::SetOption(ESocketOptLevel level, ESocketOpt option, linger& value)
{
    socklen_t iBufferSize = sizeof(value);
    //setOption(cast(EnmSocketOptionLevel)EnmenmSocketOptionLevel_SOCKET, SocketOption.LINGER, (&value)[0 .. 1]);
    return SetOption(level, option, (void*)&value, iBufferSize);
}
int Socket::SetOption(ESocketOptLevel level, ESocketOpt option, timeval& value)
{
    socklen_t iBufferSize = sizeof(value);

    return SetOption(level, option, (void*)&value, iBufferSize);
}
std::string Socket::GetErrorText()
{
    int32_t iErrorNo;
    char szErrorBuffer[80];

    GetOption(enmSocketOptionLevel_SOCKET, enmSocketOption_SO_ERROR, iErrorNo);

#if defined(strerror_r)
    strerror_r(iErrorNo, szErrorBuffer, sizeof(szErrorBuffer));
#else
    szErrorBuffer[0] = '\0';
#endif

    return std::string(szErrorBuffer);
}
void Socket::SetKeepAlive(int iTime, int iInterval)
{
#if(AURORA_OS_WINDOWS == aurora_OS)
    StTcpKeepalive options;
    options.onoff = 1;
    options.keepalivetime = iTime * 1000;
    options.keepaliveinterval = iInterval * 1000;
    DWORD cbBytesReturned;
    WSAIoctl(this->m_iSocketID, SIO_KEEPALIVE_VALS, &options, sizeof(options), NULL, 0, &cbBytesReturned, NULL, NULL);
#else
    //SetOption(enmSocketOptionLevel_TCP, enmSocketOption_TCP_KEEPIDLE, iTime);
    SetOption(enmSocketOptionLevel_TCP, enmSocketOption_TCP_KEEPINTVL, iInterval);
    SetOption(enmSocketOptionLevel_SOCKET, enmSocketOption_SO_KEEPALIVE, true);
#endif

}
int Socket::Select(SocketSet& checkRead, SocketSet& checkWrite, SocketSet& checkError)
{
    return Select(checkRead, checkWrite, checkError, (timeval*)NULL);
}

int Socket::Select(SocketSet& checkRead, SocketSet& checkWrite, SocketSet& checkError, timeval* timeout)
{
    if (&checkRead == &checkWrite || &checkRead == &checkError || &checkWrite == &checkError)
    {
        return -1;
    }

    fd_set* fr = NULL;
    fd_set* fw = NULL;
    fd_set* fe = NULL;
    int n = 0;

    if (checkRead.GetCount() > 0)
    {
        fr = checkRead.GetFdSetPtr();
        n = checkRead.Selectn();
    }
    else
    {
        fr = NULL;
    }

    if (checkWrite.GetCount() > 0)
    {
        fw = checkWrite.GetFdSetPtr();
        int _n;
        _n = checkWrite.Selectn();
        if (_n > n)
            n = _n;
    }
    else
    {
        fw = NULL;
    }

    if (checkError.GetCount() > 0)
    {
        fe = checkError.GetFdSetPtr();
        int _n;
        _n = checkError.Selectn();
        if (_n > n)
            n = _n;
    }
    else
    {
        fe = NULL;
    }

    int result = select(n, fr, fw, fe, timeout);

    if (-1 == result && errno == EINTR)
    {
        return -1;
    }

    return result;
}

// This overload is explicitly not documented. Please do not use it. It will
// likely be deprecated in the future. It is against Phobos policy to have
// functions which use naked numbers for time values.
int Socket::Select(SocketSet& checkRead, SocketSet& checkWrite, SocketSet& checkError, long microseconds)
{
    timeval tv;
    tv.tv_sec = (microseconds / 1000000);
    tv.tv_usec = (microseconds % 1000000);
    return Select(checkRead, checkWrite, checkError, &tv);
}

void Socket::Close()
{
    if (m_iSocketID != Invalid_SocketID)
    {
        CloseSocketID(m_iSocketID);
        m_iSocketID = Invalid_SocketID;
    }
}

}

