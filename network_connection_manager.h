#ifndef NETWORK_CONNECTION_MANAGER_H_
#define NETWORK_CONNECTION_MANAGER_H_
#include "common_define.h"
#include "ikcp.h"
#include "common_algorithm.h"
#include "tcp_connection.h"
#include "udp_connection.h"
namespace NetWork {

class ConnectionManager
{
public:
    enum KCP_CMD_TCP_TYPE
    {
        KCP_KEY_LENGTH = 6,
        IKCP_OVERHEAD_TYPE = 24,
        KCP_CMD_PUSH = 81,
        KCP_CMD_ASK_CONV_TYPE = 85,
        KCP_CMD_TELL_EXIT_TYPE = 86,
        KCP_CMD_NOTIFY_BUFF_FULL = 87,
        KCP_MTU_LEN = 128 * 3,
    };
    enum KCP_STATUS
    {
        KCP_UN_CONNECt_STATUS,
        KCP_CONNECT_STATUS,
        KCP_CONNECT_EXIT_STATUS,
    };
    enum TRANSPORT_USE_PROTOCOL
    {
        TRANSPORT_USE_UNKNOW_PROTOCOL,
        TRANSPORT_USE_TCP_PROTOCOL,
        TRANSPORT_USE_UDP_PROTOCOL,
    };
    enum CONNECT_STATUS
    {
        UNCONNECT_STATUS,
        CONNECT_ING_STATUS,
        CONNECT_SUCCESS_STATUS,
    };
    static ConnectionManager* GetInstance();
    static int32_t KcpOutput(const char* buf, int len, ikcpcb* kcp, void* ptr);
    static int32_t KcpCanOutput(ikcpcb* kcp, int len, void* ptr);

    bool ChangeAddress(const std::string& szAddr, uint16_t unPort);
    bool ConnectLogicServer();

    void CloseLogicConnect(const std::string& reason = "");

    void ReconnectLogicServer();

    bool ReconnectUdpLogicServer();
    void CloseUdpLogicConnect(const std::string& strReason);

    bool ReconnectTcpLogicServer();
    void CloseTcpLogicConnect(const std::string& strReason);

    int32_t GetLogicPingValue();
    int32_t GetDistanceLastReceiveTime();

    bool CheckHaveUnSendData();
    bool CheckHaveWillSendData();
    //kcp
    void SetUseUdpInLogic(bool buse);
    bool GetUseUdpInLoigc() { return m_use_udp_inlogic; }
    void SetUseTcpInLogic(bool buse);
    bool GetUseTcpInLogic() { return m_use_tcp_inlogic; }
    bool GetKcpBuffFull() { return m_kcp_buff_full; }
    void SetKcpBuffFull(bool buff_full) { m_kcp_buff_full = buff_full; }
    int32_t GetKcpRxRto();


    bool KCPConnectLogicInitUdp();
    bool KCPConnectLogicInitTcp();
    UdpConnection* GetLogicUdpConn();
    TcpConnection* GetLogicTcpConn();

    int32_t GetConv() { return m_conv; }
    void SetConv(int32_t conv) { m_conv = conv; }

    int32_t SendOneCode(char* szSendCodeBuffer, int32_t iBufferLen);
    int32_t ReceiveOneCode(char* szReceiveCodeBuffer, int32_t iReceiveCodeBufferLength);
    int32_t GetConvFromServer(uint32_t current);

    void UpdateSessionKey(const char session_key[XTEA_KEY_LENGTH]);

    KCP_STATUS GetKcpStatus() { return m_kcp_status; }
    void SetKcpStatus(KCP_STATUS status) { m_kcp_status = status; }
    TRANSPORT_USE_PROTOCOL GetTransportUseProtocol() { return m_transport_use_protocol; }
    void SetTransportUseProtocol(TRANSPORT_USE_PROTOCOL transport_protocol) { m_transport_use_protocol = transport_protocol; }
    CONNECT_STATUS GetConnectStatus() { return m_connect_status; }


    void SetConnectStatus(CONNECT_STATUS status) { m_connect_status = status; }

    void SetUdpPort(uint16_t port) { m_server_udp_port = port; }
    uint16_t GetUdpPort() { return m_server_udp_port; }

    void SetSignature(char *signature, int16_t signature_len);
    void SetUin(int32_t uin) { m_uin = uin; }
    char m_sz_session_key[XTEA_KEY_LENGTH]; //Ç©Ãû
    char m_kcp_key[KCP_KEY_LENGTH + 2];
private:
    bool CheckKcpKye(const char *buf, int32_t buf_len);
    void ClearTempData();

    int32_t ProtocolReceiveTcpData(char* data_buf, int32_t buf_len);
    int32_t ProtocolReceiveUdpData(char* data_buf, int32_t buf_len);
    int32_t ReceiveData(uint32_t current);
    int32_t SendData();
    int32_t SendData(const char *buf, int32_t len);
    int32_t CanSendData(int32_t len);
    int32_t FlushSendDataToKcp();
    int32_t Update(uint32_t current);
    void _CloseConnections();
    void NewKcp();
    void DestroyKcp();
    IUINT32 _iClock();
    IUINT64 _iClock64();
    ConnectionManager();
    ~ConnectionManager();
private:
    static ConnectionManager* ms_connection_manager;
    TcpConnection* m_logic_tcp_connection;
    UdpConnection* m_logic_udp_connection;
    std::string m_server_ip;
    uint16_t m_server_port;
    uint16_t m_server_udp_port;
    // TcpConnection* m_stCrossTcpConnection;
    bool m_bIsCrossServerConnected;
    bool m_kcp_buff_full;
    bool m_use_udp_inlogic;
    bool m_use_tcp_inlogic;
    int32_t m_uin;
    char m_Signature[256];
    int16_t m_signature_len;
    //bool m_bUseKCPInCross;
    //bool m_bUseKCPForBattle;
    IUINT64 m_last_notify_kcp_full_time;
    IUINT32 m_last_active_time;
    int32_t m_conv;
    ikcpcb *m_kcp;
    KCP_STATUS m_kcp_status;
    TRANSPORT_USE_PROTOCOL m_transport_use_protocol;
    CONNECT_STATUS m_connect_status;


    AurCodeRoundRobinQueue* m_receive_queue;
    AurCodeRoundRobinQueue* m_send_queue;


    AurCodeRoundRobinQueue* m_un_receive_queue;
    AurCodeRoundRobinQueue* m_un_send_queue;
};

}
#endif