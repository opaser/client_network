#ifndef NETWORK_TCP_CONNECTION_H_
#define NETWORK_TCP_CONNECTION_H_
#include "connection.h"
namespace NetWork 
{
class TcpConnection : public Socket
{
public:
    TcpConnection(const std::string& szAddr, uint16_t unPort);
    TcpConnection();
    ~TcpConnection();


    EnmConnectionStatus enmConnectionStatus();
    Address* GetTargetAddress();
    void SetTargetAddress(const std::string& szAddr, uint16_t unPort);
    void Connect(Address* stTargetAddr);
    void ReConnect(int iTimeOutMicroseconds = 1000000); //1s
    void ClosedByServer();
    void CloseConn(const std::string& strReason);
   
    int32_t SendTcpData(char *senddata_buffer, int32_t buffer_len); //�ϲ�ʹ��
    int32_t ReceiveTcpData(char *buf_data, int32_t buffer_len); //�ϲ�ʹ��
protected:
    void _ResetSocketHandle();
    void _InternalInit();
private:
    EnmConnectionStatus m_enmConnectionStatus;
    Address* m_stTargetAddress;
    int32_t m_iTimeOutMicroseconds; //1s���ӳ�ʱ
};
}
#endif