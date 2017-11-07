#ifndef _NETWORK_UDP_SESSION_H__
#define _NETWORK_UDP_SESSION_H__
#include "common_base.h"
namespace NetWork
{
class UDPSession 
{
public:
    UDPSession();
    ~UDPSession();
    inline void SetSockfd(int sock_fd) { m_sockfd = sock_fd; }
    int GetSocketId() { return m_sockfd; };
    int32_t Connect(const char *ip, uint16_t port);
    void CloseConnect(int iSocketID);
    void CloseConnect();
    int32_t Read(char* buff, size_t buff_size);
    int32_t Send(const char* buff, size_t buff_size);
private:
    int m_sockfd{ -1 };
};
}
#endif