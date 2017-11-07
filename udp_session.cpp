#include "udp_session.h"
namespace NetWork
{
UDPSession::UDPSession()
{
    m_sockfd = -1;
}
UDPSession::~UDPSession()
{
    m_sockfd = -1;
}

int32_t UDPSession::Connect(const char *ip, uint16_t port)
{
    if (m_sockfd != -1)
    {
        CloseConnect(m_sockfd);
        m_sockfd = 0;
    }
    struct addrinfo *answer, hint, *curr;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_DGRAM;

    struct sockaddr_in  sockaddr_ipv4;
    struct sockaddr_in6 sockaddr_ipv6;

    int ret = getaddrinfo(ip, NULL, &hint, &answer);
    if (ret != 0)
    {
        //NET_LOG(KEY) << "getaddrinfo fail errno:" << LastErrorNo() << " | errstr:" << LastErrorString();
        //cx log
        return -1;
    }

    bool bValid = false;
    bool bIpv6Flag = false;
    for (curr = answer; curr != NULL; curr = curr->ai_next)
    {
        switch (curr->ai_family)
        {
        case AF_INET:
        {
            auto* sockaddr_ipv4_ptr = reinterpret_cast<struct sockaddr_in *>(curr->ai_addr);
            memset(&sockaddr_ipv4, 0, sizeof(sockaddr_ipv4));
            sockaddr_ipv4.sin_family = curr->ai_family;
            sockaddr_ipv4.sin_addr = sockaddr_ipv4_ptr->sin_addr;
            sockaddr_ipv4.sin_port = htons(port);
            bValid = true;
        }
        break;
        case AF_INET6:
        {
            auto* sockaddr_ipv6_ptr = reinterpret_cast<struct sockaddr_in6 *>(curr->ai_addr);
            memset(&sockaddr_ipv6, 0, sizeof(sockaddr_ipv6));
            sockaddr_ipv6.sin6_family = curr->ai_family;
            sockaddr_ipv6.sin6_addr = sockaddr_ipv6_ptr->sin6_addr;
            sockaddr_ipv6.sin6_port = htons(port);
            bValid = true;
            bIpv6Flag = true;
        }
        break;
        }
    }

    freeaddrinfo(answer);

    if (!bValid)
    {
        //NET_LOG(KEY) << "getaddrinfo does not get valid sock_addr";
        //cx log
        return -1;
    }

    int sockfd = -1;
    if (!bIpv6Flag)
    {
        sockfd = socket(PF_INET, SOCK_DGRAM, 0);
        if (sockfd == -1)
        {
            //NET_LOG(KEY) << "create ipv4 socket id failed errno:" << LastErrorNo() << " | errstr:" << LastErrorString();
            //cx log
            return -1;
        }

        if (connect(sockfd, (struct sockaddr *)&sockaddr_ipv4, sizeof(sockaddr_ipv4)) < 0)
        {
            //NET_LOG(KEY) << "KcpConnect ipv4 failed errno:" << LastErrorNo() << " | errstr:" << LastErrorString();
            //cx log
            CloseConnect(sockfd);
            return -1;
        }
    }
    else
    {
        sockfd = socket(PF_INET6, SOCK_DGRAM, 0);
        if (sockfd == -1)
        {
            //NET_LOG(KEY) << "create ipv6 socket id failed errno:" << LastErrorNo() << " | errstr:" << LastErrorString();
            // cx log
            return -1;
        }

        if (connect(sockfd, (struct sockaddr *)&sockaddr_ipv6, sizeof(sockaddr_ipv6)) < 0)
        {
            //NET_LOG(KEY) << "KcpConnect ipv6 failed errno:" << LastErrorNo() << " | errstr:" << LastErrorString();
            // cx log
            CloseConnect(sockfd);
            return -1;
        }
    }

#if aurora_OS == AURORA_OS_IOS
    {
        int set = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
    }
#endif
#if (AURORA_OS_ANDROID == aurora_OS) || (AURORA_OS_IOS == aurora_OS) || (AURORA_OS_LINUX == aurora_OS)
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0)
    {
        CloseConnect(sockfd);
        return -1;
    }

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        CloseConnect(sockfd);
        return -1;
    }
#elif (AURORA_OS_WINDOWS == aurora_OS)
    u_long num = 1;
    if (SOCKET_ERROR == ioctlsocket(sockfd, FIONBIO, &num))
    {
        CloseConnect(sockfd);
        return -1;
    }
#endif
    m_sockfd = sockfd;
    return sockfd;
}

void UDPSession::CloseConnect(int iSocketID)
{
#if (AURORA_OS_ANDROID == aurora_OS) || (AURORA_OS_LINUX == aurora_OS) || (AURORA_OS_IOS == aurora_OS)
    close(iSocketID);
#elif (AURORA_OS_WINDOWS == aurora_OS)
    closesocket(iSocketID);
#endif
}

void UDPSession::CloseConnect()
{
    if (m_sockfd != -1)
    {
        CloseConnect(m_sockfd);
    }
    m_sockfd = -1;
}

int32_t UDPSession::Read(char* buff, size_t buff_size)
{
    return  recv(m_sockfd, buff, buff_size, 0);
}

int32_t UDPSession::Send(const char* buff, size_t buff_size)
{
    int32_t n = send(m_sockfd, buff, buff_size, 0);
    return n;
}

}

