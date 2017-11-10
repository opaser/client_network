#include "network_connection_manager.h"
#include "common_datetime.h"
#include <stdio.h>
namespace NetWork
{
ConnectionManager* ConnectionManager::ms_connection_manager = NULL;
ConnectionManager::ConnectionManager()
{
    printf("Hello World\n");
    m_logic_tcp_connection = NULL;
    m_logic_udp_connection = NULL;
    m_kcp = NULL;
    m_use_tcp_inlogic = true;
    m_use_tcp_inlogic = true;
    //m_bUseKCPInCross = false;
    //m_bUseKCPForBattle = false;
    //m_stLogicKcpConnection = nullptr;
    //m_stCrossKcpConnection = nullptr;

    m_bIsCrossServerConnected = false;

    m_last_active_time = 0;
    m_conv = 0;
    m_kcp_status = KCP_UN_CONNECt_STATUS;
    m_transport_use_protocol = TRANSPORT_USE_UNKNOW_PROTOCOL;
    m_connect_status = UNCONNECT_STATUS;
    m_receive_queue = new AurCodeRoundRobinQueue(max_client_pkg_size);
    m_send_queue = new AurCodeRoundRobinQueue(max_client_pkg_size);

    m_un_receive_queue = new AurCodeRoundRobinQueue(max_client_pkg_size);
    m_un_send_queue = new AurCodeRoundRobinQueue(max_client_pkg_size);
    memset(m_sz_session_key, 0, sizeof(m_sz_session_key));
    m_last_notify_kcp_full_time = 0;
    m_kcp_buff_full = false;
    m_signature_len = 0;
}

ConnectionManager::~ConnectionManager()
{
    SafeDelete(m_logic_tcp_connection);
    SafeDelete(m_logic_udp_connection);

    SafeDelete(m_receive_queue);
    SafeDelete(m_send_queue);
    SafeDelete(m_un_send_queue);
    SafeDelete(m_un_receive_queue);
    DestroyKcp();
}

ConnectionManager* ConnectionManager::GetInstance()
{
    if (ms_connection_manager == NULL)
    {
        ms_connection_manager = new ConnectionManager();
    }
    return ms_connection_manager;
}

int32_t ConnectionManager::KcpOutput(const char* buf, int len, ikcpcb* kcp, void* ptr)
{
    assert(NULL != ptr);
    ConnectionManager* connect_tcp = static_cast<ConnectionManager*>(ptr);
    connect_tcp->SendData(buf, len);
    return 0;
}

int32_t ConnectionManager::KcpCanOutput(ikcpcb* kcp, int len, void* ptr)
{
    assert(NULL != ptr);
    ConnectionManager* connect_tcp = static_cast<ConnectionManager*>(ptr);
    return connect_tcp->CanSendData(len) > 0 ? 0 : -1;
}

bool ConnectionManager::ChangeAddress(const std::string& szAddr, uint16_t unPort)
{
    KCPConnectLogicInitUdp();
    KCPConnectLogicInitTcp();
    m_server_port = unPort;
    m_server_ip = szAddr;
    memset(m_sz_session_key, 0, sizeof(m_sz_session_key));
    GetLogicTcpConn()->SetTargetAddress(szAddr, unPort);
    DestroyKcp();
    NewKcp();
    return true;
}

bool ConnectionManager::ConnectLogicServer()
{
    if (GetConnectStatus() != UNCONNECT_STATUS)
    {
        ReconnectLogicServer();
        return true;
    }
    if (m_use_udp_inlogic)
    {
        ClearTempData();
        m_un_send_queue->WriteDataToBuffer(m_kcp_key, KCP_KEY_LENGTH);
        GetLogicUdpConn()->Connect(m_server_ip.c_str(), GetUdpPort());
        SetTransportUseProtocol(TRANSPORT_USE_UDP_PROTOCOL);
        SetConnectStatus(CONNECT_ING_STATUS);

    }
    else if (m_use_tcp_inlogic)
    {
        GetLogicTcpConn()->Shutdown(enmSocketShutdown_BOTH);
        GetLogicTcpConn()->CloseConn("ConnectionManager::ConnectLogicServer");

        GetLogicTcpConn()->Connect(m_logic_tcp_connection->GetTargetAddress());

        ClearTempData();
        m_un_send_queue->WriteDataToBuffer(m_kcp_key, KCP_KEY_LENGTH);
        SetTransportUseProtocol(TRANSPORT_USE_TCP_PROTOCOL);
        SetConnectStatus(CONNECT_ING_STATUS);
    }
    return true;
}

void ConnectionManager::CloseLogicConnect(const std::string& reason /*= ""*/)
{
    if (GetTransportUseProtocol() == TRANSPORT_USE_UDP_PROTOCOL)
    {
        CloseUdpLogicConnect(reason);
    }
    else if (GetTransportUseProtocol() == TRANSPORT_USE_TCP_PROTOCOL)
    {
        CloseTcpLogicConnect(reason);
    }
    SetConnectStatus(UNCONNECT_STATUS);
    SetTransportUseProtocol(TRANSPORT_USE_UNKNOW_PROTOCOL);
    SetKcpStatus(KCP_UN_CONNECt_STATUS);
    ClearTempData();
}

void ConnectionManager::ReconnectLogicServer()
{
    if (GetUseUdpInLoigc() && GetUseTcpInLogic())
    {
        if (GetConnectStatus() == CONNECT_ING_STATUS)
        {
            if (GetTransportUseProtocol() == TRANSPORT_USE_TCP_PROTOCOL)
            {
                CloseTcpLogicConnect("reconnect");
                ReconnectUdpLogicServer();
            }
            else
            {
                CloseUdpLogicConnect("reconnect");
                ReconnectTcpLogicServer();
            }
        }
        else if (GetConnectStatus() == CONNECT_SUCCESS_STATUS)
        {
            if (GetTransportUseProtocol() == TRANSPORT_USE_TCP_PROTOCOL)
            {
                CloseTcpLogicConnect("reconnect");
                ReconnectTcpLogicServer();
            }
            else
            {
                ReconnectUdpLogicServer();
            }
        }
        else
        {
            ConnectLogicServer();
        }
    }
    else if (GetUseUdpInLoigc())
    {
        CloseTcpLogicConnect("reconnect");
        ReconnectUdpLogicServer();
    }
    else if (GetUseTcpInLogic())
    {
        ReconnectTcpLogicServer();
    }
}

bool ConnectionManager::ReconnectUdpLogicServer()
{
    ClearTempData();
    if (m_kcp == 0)
    {
        m_un_send_queue->WriteDataToBuffer(m_kcp_key, KCP_KEY_LENGTH);
    }
    GetLogicUdpConn()->Connect(m_server_ip.c_str(), GetUdpPort());
    SetTransportUseProtocol(TRANSPORT_USE_UDP_PROTOCOL);
    SetConnectStatus(CONNECT_ING_STATUS);
    return true;
}

void ConnectionManager::CloseUdpLogicConnect(const std::string& strReason)
{
    ClearTempData();
    SetTransportUseProtocol(TRANSPORT_USE_UNKNOW_PROTOCOL);
    SetConnectStatus(UNCONNECT_STATUS);
    GetLogicUdpConn()->CloseConn(strReason);
}

bool ConnectionManager::ReconnectTcpLogicServer()
{
    ClearTempData();
    m_un_send_queue->WriteDataToBuffer(m_kcp_key, KCP_KEY_LENGTH);
    GetLogicTcpConn()->ReConnect();
    SetTransportUseProtocol(TRANSPORT_USE_TCP_PROTOCOL);
    SetConnectStatus(CONNECT_ING_STATUS);
    return true;
}

void ConnectionManager::CloseTcpLogicConnect(const std::string& strReason)
{
    ClearTempData();
    SetTransportUseProtocol(TRANSPORT_USE_UNKNOW_PROTOCOL);
    SetConnectStatus(UNCONNECT_STATUS);
    GetLogicTcpConn()->Shutdown(enmSocketShutdown_BOTH);
    GetLogicTcpConn()->CloseConn(strReason);
}

int32_t ConnectionManager::GetLogicPingValue()
{
    if (m_kcp != NULL)
    {
        return m_kcp->rx_realrto;
    }
    return 0;
}

int32_t ConnectionManager::GetDistanceLastReceiveTime()
{
    uint32_t now_iclock = _iClock();
    if (now_iclock >= m_last_active_time)
    {
        return now_iclock - m_last_active_time;
    }
    else
    {
        uint32_t max_uint = 0xfffffffful;
        return max_uint - m_last_active_time + now_iclock;
    }
    return 0;
}

bool ConnectionManager::CheckHaveUnSendData()
{
    if (m_kcp != NULL)
    {
        if (CheckHaveWillSendData())
        {
            return true;
        }
    }
    return false;
}

bool ConnectionManager::CheckHaveWillSendData()
{
    if (m_kcp != NULL)
    {
        if (ikcp_sndbuf_count(m_kcp) > 0 || ikcp_sndque_count(m_kcp) > 0)
        {
            return true;
        }
        if (m_send_queue->GetUsedBufferSize() > 0)
        {
            return true;
        }
    }
    return false;
}

void ConnectionManager::SetUseUdpInLogic(bool buse)
{
    m_use_udp_inlogic = buse;
}

void ConnectionManager::SetUseTcpInLogic(bool buse)
{
    m_use_tcp_inlogic = buse;
}

int32_t ConnectionManager::GetKcpRxRto()
{
    if (m_kcp != NULL)
    {
        return m_kcp->rx_rto;
    }
    return 0;
}

bool ConnectionManager::KCPConnectLogicInitUdp()
{
    if (m_logic_udp_connection == NULL)
    {
        m_logic_udp_connection = new UdpConnection();
    }
    return true;
}

bool ConnectionManager::KCPConnectLogicInitTcp()
{
    if (NULL == m_logic_tcp_connection)
    {
        m_logic_tcp_connection = new TcpConnection();
    }
    return true;
}

UdpConnection* ConnectionManager::GetLogicUdpConn()
{
    if (NULL == m_logic_udp_connection)
    {
        m_logic_udp_connection = new UdpConnection();
    }
    return m_logic_udp_connection;
}

TcpConnection* ConnectionManager::GetLogicTcpConn()
{
    if (NULL == m_logic_tcp_connection)
    {
        m_logic_tcp_connection = new TcpConnection();
    }

    return m_logic_tcp_connection;
}

int32_t ConnectionManager::SendOneCode(char* szSendCodeBuffer, int32_t iBufferLen)
{
    uint32_t current = _iClock();
    GetConvFromServer(current);
    if (GetConv() == 0 && CheckHaveWillSendData())
    {
        Update(current);
        return 0;
    }
    FlushSendDataToKcp();
    int32_t send_result = m_send_queue->WriteOneCode(szSendCodeBuffer, iBufferLen);
    if (send_result <= 0)
    {
        //AURORA_LOG(ERROR) << "kcp:log error write on code to m_stSendCodeRoundRobinQueue fail  result:" << send_result;
        //cx log
        m_kcp_status = KCP_CONNECT_EXIT_STATUS;
        return send_result;
    }
    FlushSendDataToKcp();
    int32_t ret = Update(current);
    if (ret < 0)
    {
        return ret;
    }
    return send_result;
}

int32_t ConnectionManager::ReceiveOneCode(char* szReceiveCodeBuffer, int32_t iReceiveCodeBufferLength)
{
    if (GetConnectStatus() == UNCONNECT_STATUS)
        return 0;
    if (GetTransportUseProtocol() == TRANSPORT_USE_UNKNOW_PROTOCOL)
        return 0;
    if (GetKcpStatus() == KCP_CONNECT_EXIT_STATUS)
        return -1;

    int32_t receive_len = 0;

    int32_t iReceiveCodeSize = m_receive_queue->GetOneCode(szReceiveCodeBuffer, iReceiveCodeBufferLength);

    if (iReceiveCodeSize > 0)
    {
        return iReceiveCodeSize;
    }

    static char szReceiveBuffer[max_client_pkg_size];

    do
    {
        receive_len = Update(_iClock());
        if (receive_len < 0)
        {
            return receive_len;
        }
    } while (receive_len > 0);

    iReceiveCodeSize = m_receive_queue->GetOneCode(szReceiveCodeBuffer, iReceiveCodeBufferLength);
    if (iReceiveCodeSize > 0)
    {
        return iReceiveCodeSize;
    }
    return 0;
}

int32_t ConnectionManager::GetConvFromServer(uint32_t current)
{
    if (m_kcp->conv != 0)
    {
        return 0;
    }
    if (m_un_send_queue->GetUsedBufferSize() > 0)
    {  //有数据我就不发了 等发完再说
        if (m_un_send_queue->GetUsedBufferSize() != KCP_KEY_LENGTH)
        {
            return 0;
        }
        char buff_key[KCP_KEY_LENGTH + 1];
        m_un_send_queue->CopyDataFromBuffer(buff_key, KCP_KEY_LENGTH);
        if (CheckKcpKye(buff_key, KCP_KEY_LENGTH) == false)
        {
            return 0;
        }
    }
    static char buffer[IKCP_OVERHEAD_TYPE + 264];// 264: [2:签名最大长度]+[256:签名]+[uin:4]+[2:多余两位]
    IKCPSEG seg;
    char *buffer_tmp = &buffer[0];
    seg.conv = 0;
    seg.cmd = KCP_CMD_ASK_CONV_TYPE;
    seg.len = m_signature_len + 4 + 2;
    buffer_tmp = ikcp_encode_seg(buffer_tmp, &seg);
    buffer_tmp = ikcp_encode32u(buffer_tmp, (IUINT32)m_uin);
    buffer_tmp = ikcp_encode16u(buffer_tmp, (IUINT16)m_signature_len);
    memcpy(buffer_tmp, m_Signature, m_signature_len);
    buffer_tmp = buffer_tmp + m_signature_len;
    int32_t buffer_len = buffer_tmp - (&buffer[0]);
    if (buffer_len != IKCP_OVERHEAD_TYPE + seg.len)
    {
        return 0;
    }
    m_un_send_queue->WriteDataToBuffer(buffer, IKCP_OVERHEAD_TYPE + seg.len);
    return 0;
}

void ConnectionManager::UpdateSessionKey(const char session_key[XTEA_KEY_LENGTH])
{
    if (0 != memcmp(m_sz_session_key, session_key, sizeof(m_sz_session_key)))
    {
        memcpy(m_sz_session_key, session_key, sizeof(m_sz_session_key));
    }
}

bool ConnectionManager::CheckKcpKye(const char *buf, int32_t buf_len)
{
    if (buf_len < KCP_KEY_LENGTH)
    {
        return false;
    }
    for (int i = 0; i < KCP_KEY_LENGTH; ++i)
        if (m_kcp_key[i] != buf[i])
        {
            return false;
        }
    return true;
}

void ConnectionManager::ClearTempData()
{
    m_un_send_queue->RemoveDataFromBuffer(m_un_send_queue->GetUsedBufferSize());
    m_un_receive_queue->RemoveDataFromBuffer(m_un_receive_queue->GetUsedBufferSize());
}

int32_t ConnectionManager::ProtocolReceiveTcpData(char* data_buf, int32_t buf_len)
{
    int32_t receive_len_cnt = 0;
    int32_t free_size = m_un_receive_queue->GetFreeBufferSize();
    while (free_size > 0)
    {
        free_size = free_size > buf_len ? buf_len : free_size;
        int32_t receive_len = 0;
        if (m_logic_tcp_connection != nullptr)
        {
            receive_len = m_logic_tcp_connection->ReceiveTcpData(data_buf, free_size);
        }
        else
        {
            //cx log
            //AURORA_LOG(ERROR) << "kcp:log tcp receive from socket m_logic_tcp_connection is nil value";
        }
        if (receive_len <= 0)
        {
            if (receive_len < 0)
            {
                m_connect_status = UNCONNECT_STATUS; //tcp连接不可用了
            }
            break;
        }

        m_un_receive_queue->WriteDataToBuffer(data_buf, receive_len);
        receive_len_cnt += receive_len;
        free_size = m_un_receive_queue->GetFreeBufferSize();
    }
    if (GetTransportUseProtocol() == TRANSPORT_USE_TCP_PROTOCOL && GetConnectStatus() == CONNECT_ING_STATUS && receive_len_cnt > 0)
    {
        SetConnectStatus(CONNECT_SUCCESS_STATUS);
    }
    return receive_len_cnt;
}

int32_t ConnectionManager::ProtocolReceiveUdpData(char* data_buf, int32_t buf_len)
{
    int32_t free_size = m_un_receive_queue->GetFreeBufferSize();
    int32_t receive_len = GetLogicUdpConn()->ReadUdpData(data_buf, free_size);
    if (receive_len > 0)
    {
        if (GetTransportUseProtocol() == TRANSPORT_USE_UDP_PROTOCOL && GetConnectStatus() == CONNECT_ING_STATUS)
        {
            SetConnectStatus(CONNECT_SUCCESS_STATUS);
        }
        m_un_receive_queue->WriteDataToBuffer(data_buf, receive_len);
        return receive_len;
    }
    return 0;
}

int32_t ConnectionManager::ReceiveData(uint32_t current)
{
    static char szReceiveBuffer[max_client_pkg_size + 1];
    if (GetTransportUseProtocol() == TRANSPORT_USE_TCP_PROTOCOL)
    {
        ProtocolReceiveTcpData(szReceiveBuffer, max_client_pkg_size);
    }
    else if (GetTransportUseProtocol() == TRANSPORT_USE_UDP_PROTOCOL)
    {
        ProtocolReceiveUdpData(szReceiveBuffer, max_client_pkg_size);
    }

    int32_t remained_size = m_un_receive_queue->GetUsedBufferSize();
    while (true)
    {
        remained_size = remained_size > max_client_pkg_size ? max_client_pkg_size : remained_size;
        if (remained_size < IKCP_OVERHEAD_TYPE)
        {
            break;
        }
        if (m_un_receive_queue->CopyDataFromBuffer(szReceiveBuffer, IKCP_OVERHEAD_TYPE) != IKCP_OVERHEAD_TYPE)
        {
            //AURORA_LOG(ERROR) << "kcp:log socket copy data form m_un_receive_queue error";
            //cx log
            m_kcp_status = KCP_CONNECT_EXIT_STATUS;
            return -1;
        }

        IKCPSEG seg;
        ikcp_gethead(szReceiveBuffer, IKCP_OVERHEAD_TYPE, &seg);
        int32_t package_len = seg.len + IKCP_OVERHEAD_TYPE;
        if (package_len < IKCP_OVERHEAD_TYPE || package_len > KCP_MTU_LEN || remained_size < package_len) //判断数据是否够长
        {
            break;
        }
        m_last_active_time = current;
        if (KCP_CMD_ASK_CONV_TYPE == seg.cmd)
        {
            if (m_conv == 0)
            {
                m_conv = seg.conv;
                m_kcp->conv = seg.conv;
                if (seg.len == KCP_KEY_LENGTH)
                {
                    if (m_un_receive_queue->CopyDataFromBuffer(szReceiveBuffer, package_len) != package_len)
                    {
                        //AURORA_LOG(ERROR) << "kcp:log socket copy data form m_stRecieveCodeSocketQueue error";
                        //cx log
                        m_kcp_status = KCP_CONNECT_EXIT_STATUS;
                        return -1;
                    }
                    memcpy(m_kcp_key, szReceiveBuffer + IKCP_OVERHEAD_TYPE, KCP_KEY_LENGTH);
                }
            }
        }
        else if (KCP_CMD_TELL_EXIT_TYPE == seg.cmd)
        {
            //想个办法让提示退出
            m_kcp_status = KCP_CONNECT_EXIT_STATUS;
        }
        else
        {
            if (KCP_CMD_NOTIFY_BUFF_FULL == seg.cmd)
            {
                if (_iClock64() - m_last_notify_kcp_full_time > 50000)
                {
                    SetKcpBuffFull(true);
                    m_last_notify_kcp_full_time = _iClock64();
                }
            }
            if (m_un_receive_queue->CopyDataFromBuffer(szReceiveBuffer, package_len) != package_len)
            {
                //AURORA_LOG(ERROR) << "kcp:log socket copy data form m_stRecieveCodeSocketQueue error";
                //cx log
                m_kcp_status = KCP_CONNECT_EXIT_STATUS;
                return -1;
            }
            ikcp_input(m_kcp, szReceiveBuffer, package_len);
        }
        if (m_un_receive_queue->RemoveDataFromBuffer(package_len) < 0)
        {
            //AURORA_LOG(ERROR) << "kcp:log socket remove data form m_stRecieveCodeSocketQueue error";
            //cx log
            m_kcp_status = KCP_CONNECT_EXIT_STATUS;
        }
        remained_size = m_un_receive_queue->GetUsedBufferSize();
    }
    int32_t free_len = m_receive_queue->GetFreeBufferSize();
    free_len = free_len > max_client_pkg_size ? max_client_pkg_size : free_len;
    int32_t recieve_len = ikcp_recv(m_kcp, szReceiveBuffer, free_len);
    if (recieve_len > 0)
    {
        if (m_receive_queue->WriteDataToBuffer(szReceiveBuffer, recieve_len) < 0)
        {
            //AURORA_LOG(ERROR) << "kcp:log kcp read insert m_receive_queue error";
            //cx log
            m_kcp_status = KCP_CONNECT_EXIT_STATUS;
        }
        return recieve_len;
    }
    return 0;
}

int32_t ConnectionManager::SendData()
{
    static char send_buf[max_client_pkg_size];
    int32_t sended_len_sum = 0;
    int32_t pending_sendlen = m_un_send_queue->CopyDataFromBuffer(send_buf, (int32_t)sizeof(send_buf) / sizeof(send_buf[0]));
    char *m_tmp = send_buf;
    while (pending_sendlen > 0 && pending_sendlen + sended_len_sum <= sizeof(send_buf) / sizeof(send_buf[0]))
    {
        int32_t send_len = 0;
        if (GetTransportUseProtocol() == TRANSPORT_USE_TCP_PROTOCOL)
        {
            if (GetLogicTcpConn() != nullptr)
            {
                send_len = GetLogicTcpConn()->SendTcpData(m_tmp, pending_sendlen);
            }
            else
            {
                //cx log
                //AURORA_LOG(ERROR) << "kcp:log tcp send to socket m_logic_tcp_connection is nil value";
            }
        }
        else
        {
            if (GetLogicUdpConn() != nullptr)
            {
                if (GetConv() == 0)
                {
                    send_len = GetLogicUdpConn()->SendUdpdata(m_tmp, pending_sendlen);
                    if (send_len <= 0)
                    {
                        break;
                    }
                }
            }
        }
        if (send_len == 0)
        {
            break;
        }
        pending_sendlen -= send_len;
        sended_len_sum += send_len;
        m_tmp = send_buf + sended_len_sum;
    }

    if (m_un_send_queue->RemoveDataFromBuffer(sended_len_sum) < 0)
    {
        //cx log
        //AURORA_LOG(ERROR) << "kcp:log send message remove data from m_un_send_queue to socket len <" << sended_len_sum << " > error";
        m_kcp_status = KCP_CONNECT_EXIT_STATUS;
    }

    return sended_len_sum;
}

int32_t ConnectionManager::SendData(const char *buf, int32_t len)
{
    if (GetTransportUseProtocol() == TRANSPORT_USE_UDP_PROTOCOL)
    {
        static char send_buff[KCP_MTU_LEN + 20];
        if (len > KCP_MTU_LEN + 14)
        {
            return 0;
        }
        memcpy(send_buff, m_kcp_key, KCP_KEY_LENGTH);
        memcpy(send_buff + KCP_KEY_LENGTH, buf, len);
        GetLogicUdpConn()->SendUdpdata(send_buff, len + KCP_KEY_LENGTH);
    }
    else if (GetTransportUseProtocol() == TRANSPORT_USE_TCP_PROTOCOL)
    {
        if (len > m_un_send_queue->GetFreeBufferSize())
        {
            //cx log
            //AURORA_LOG(ERROR) << "kcp:log send message from kcp too large <" << len << " > error";
            m_kcp_status = KCP_CONNECT_EXIT_STATUS;
            return 0;
        }
        if (m_un_send_queue->WriteDataToBuffer(buf, len) < 0)
        {
            //cx log
            //AURORA_LOG(ERROR) << "kcp:log send message from kcp too large <" << len << " > error";
            m_kcp_status = KCP_CONNECT_EXIT_STATUS;
            return 0;
        }
        SendData();
    }
    return 0;
}

int32_t ConnectionManager::CanSendData(int32_t len)
{
    if (GetTransportUseProtocol() == TRANSPORT_USE_UDP_PROTOCOL)
    {
        return 1;
    }
    if (len > m_un_send_queue->GetFreeBufferSize())
    {
        SendData();
    }
    if (len < m_un_send_queue->GetFreeBufferSize())
    {
        return 1;
    }
    return 0;
}

int32_t ConnectionManager::FlushSendDataToKcp()
{
    static char send_buf[max_client_pkg_size];
    if (ikcp_check_sendque_full(m_kcp))
    {
        return 0;
    }
    int data_len = m_send_queue->GetUsedBufferSize();
    data_len = data_len > max_client_pkg_size ? max_client_pkg_size : data_len;
    if (data_len <= 0)
    {
        return data_len;
    }
    if (m_send_queue->CopyDataFromBuffer(send_buf, data_len) != data_len)
    {
//        AURORA_LOG(ERROR) << "kcp:log send message m_send_queue insert data to kcp len <" << data_len << " > error";
        //cx log
        m_kcp_status = KCP_CONNECT_EXIT_STATUS;
    }


    if (ikcp_send(m_kcp, send_buf, data_len) < 0)
    {
        //cx log
        //        AURORA_LOG(ERROR) << "kcp:log send message insert data to kcp len <" << data_len << " > error";
        m_kcp_status = KCP_CONNECT_EXIT_STATUS;
        return -1;
    }
    if (m_send_queue->RemoveDataFromBuffer(data_len) < 0)
    {
        //cx log
        //AURORA_LOG(ERROR) << "kcp:log send message remove data from m_stSendCodeRoundRobinQueue to kcp len <" << data_len << " > error";
        m_kcp_status = KCP_CONNECT_EXIT_STATUS;
    }

    return data_len;
}

int32_t ConnectionManager::Update(uint32_t current)
{
    if (GetTransportUseProtocol() == TRANSPORT_USE_UNKNOW_PROTOCOL)
    {
        return 0;
    }
    if (m_kcp == NULL)
    {
        return 0;
    }
    if (m_kcp_status == KCP_CONNECT_EXIT_STATUS)
    {
        return -1;
    }
    int32_t ret = 0;
    if (m_kcp->conv == 0)
    {
        SendData();
        ret = ReceiveData(current);
        return ret;
    }
    ret = ReceiveData(current);
    FlushSendDataToKcp();
    ikcp_update(m_kcp, current);
    FlushSendDataToKcp();
    SendData();
    return ret;
}

void ConnectionManager::_CloseConnections()
{
    //NET_LOG(KEY) << "ConnectionManager::_CloseConnections1 : close socket";
    //cx log
    if (m_logic_tcp_connection)
    {
        CloseTcpLogicConnect("ConnectionManager::_CloseConnections1");
        m_logic_tcp_connection->CloseConn("ConnectionManager::_CloseConnections1");
    }

    if (m_logic_udp_connection)
    {
        CloseUdpLogicConnect("ConnectionManager::_CloseConnections1");
    }
}

void ConnectionManager::NewKcp()
{
    if (m_kcp != NULL)
    {
        DestroyKcp();
    }
    m_kcp = ikcp_create(0, (void*)this);
    ikcp_setoutput(m_kcp, ConnectionManager::KcpOutput);
    ikcp_setcanoutput(m_kcp, ConnectionManager::KcpCanOutput);
    ikcp_nodelay(m_kcp, 1, 10, 2, 1);
    ikcp_setmtu(m_kcp, KCP_MTU_LEN);
    ikcp_wndsize(m_kcp, 64, 64);
    ikcp_setusestream(m_kcp);
    m_receive_queue->RemoveDataFromBuffer(m_receive_queue->GetUsedBufferSize());
    m_send_queue->RemoveDataFromBuffer(m_send_queue->GetUsedBufferSize());
    m_un_receive_queue->RemoveDataFromBuffer(m_un_receive_queue->GetUsedBufferSize());
    m_un_send_queue->RemoveDataFromBuffer(m_un_send_queue->GetUsedBufferSize());
    m_kcp_status = KCP_CONNECT_STATUS;
    m_transport_use_protocol = TRANSPORT_USE_UNKNOW_PROTOCOL;
    m_connect_status = UNCONNECT_STATUS;
    m_last_active_time = 0;
    m_conv = 0;
    m_kcp_buff_full = false;
    m_last_notify_kcp_full_time = 0;
}

void ConnectionManager::DestroyKcp()
{
    if (NULL != m_kcp)
    {
        ikcp_release(m_kcp);
        m_receive_queue->RemoveDataFromBuffer(m_receive_queue->GetUsedBufferSize());
        m_send_queue->RemoveDataFromBuffer(m_send_queue->GetUsedBufferSize());
        m_un_receive_queue->RemoveDataFromBuffer(m_un_receive_queue->GetUsedBufferSize());
        m_un_send_queue->RemoveDataFromBuffer(m_un_send_queue->GetUsedBufferSize());
        m_kcp = nullptr;
    }
    m_kcp_buff_full = false;
    m_last_notify_kcp_full_time = 0;
    m_conv = 0;
}

IUINT32 ConnectionManager::_iClock()
{
    IUINT64 value = _iClock64();
    return (IUINT32)(value & 0xfffffffful);
}

IUINT64 ConnectionManager::_iClock64()
{
    long s, u;
    struct timeval time;
    gettimeofday(&time, NULL);
    s = time.tv_sec;
    u = time.tv_usec;

    return ((IUINT64)s) * 1000 + (u / 1000);
}

void ConnectionManager::SetSignature(char *signature, int16_t signature_len)
{
    if (signature_len < 256)
    {
        memcpy(m_Signature, signature, signature_len);
        m_signature_len = signature_len;
    }
}

}