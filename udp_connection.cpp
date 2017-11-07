#include "udp_connection.h"
namespace NetWork
{
UdpConnection::UdpConnection()
{
    m_sHost = "";
    m_iPort = 0;
}

UdpConnection::~UdpConnection()
{
    if (GetSocketId() != -1)
    {
        CloseConnect(GetSocketId());
    }
}

int32_t UdpConnection::ReadUdpData(char* buff, size_t buff_size)
{
    return Read(buff, buff_size);
}
int32_t UdpConnection::SendUdpdata(const char* buff, size_t buff_size)
{
    return Send(buff, buff_size);
}

int32_t UdpConnection::GetLocalPort()
{
    int socket_id = GetSocketId();
    if (socket_id)
    {
        struct sockaddr_storage local_addr;
        socklen_t len;
        len = sizeof(local_addr);
        if (getsockname(socket_id, (sockaddr*)&local_addr, &len) != 0)
        {
            return 3;
        }
        if (local_addr.ss_family == AF_INET6)
        {
            sockaddr_in6 *addr6 = (sockaddr_in6 *)&local_addr;
            return  ntohs(addr6->sin6_port);
        }
        else if (local_addr.ss_family == AF_INET)
        {
            sockaddr_in *addr4 = (sockaddr_in *)&local_addr;
            return ntohs(addr4->sin_port);
        }
        else
        {
            return 4;
        }
    }
    return 2;
}

std::string UdpConnection::GetLocalIP()
{
    //if (!m_stKcpSession) return "error: no session";

    int socket_id = GetSocketId();
    if (socket_id)
    {
        struct sockaddr_storage local_addr;
        socklen_t len;
        len = sizeof(local_addr);
        if (getsockname(socket_id, (sockaddr*)&local_addr, &len) != 0)
        {
            return "error: getsockname error";
        }
        if (local_addr.ss_family == AF_INET6)
        {
            sockaddr_in6 *addr6 = (sockaddr_in6 *)&local_addr;
            char ip_str[128] = { 0 };
            auto* ptr = inet_ntop(local_addr.ss_family, &addr6->sin6_addr, ip_str, sizeof(ip_str));
            if (ptr)
            {
                return ptr;
            }
            else
            {
                return "error: inet_ntop ipv6 error";
            }
        }
        else if (local_addr.ss_family == AF_INET)
        {
            sockaddr_in *addr4 = (sockaddr_in *)&local_addr;
            char ip_str[128] = { 0 };
            auto* ptr = inet_ntop(local_addr.ss_family, &addr4->sin_addr, ip_str, sizeof(ip_str));
            if (ptr)
            {
                return ptr;
            }
            else
            {
                return "error: inet_ntop ipv4 error";
            }
        }
        else
        {
            return "error: not ipv4 nor ipv6";
        }
    }
    return "error: socketid is 0";
}

void UdpConnection::CloseConn(const std::string reaseon)
{
    CloseConnect();
}

}

