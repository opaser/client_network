#ifndef _NETWORK_UDP_CONNECTION_H__
#define _NETWORK_UDP_CONNECTION_H__
#include "udp_session.h"
namespace NetWork 
{
class UdpConnection : public UDPSession
{
public:
    UdpConnection();
    ~UdpConnection();
    int32_t ReadUdpData(char* buff, size_t buff_size);
    int32_t SendUdpdata(const char* buff, size_t buff_size);
    int32_t GetLocalPort();
    std::string GetLocalIP();
    void CloseConn(const std::string reaseon);
private:
    std::string m_sHost;
    uint16_t m_iPort;
};
}
#endif
