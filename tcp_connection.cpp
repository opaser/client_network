#include "tcp_connection.h"
#include "common_define.h"
namespace NetWork 
{
TcpConnection::TcpConnection(const std::string& szAddr, uint16_t unPort)
    : Socket(enmAddressFamily_INET, enmSocketType_STREAM, enmProtocolType_TCP)
{
    _InternalInit();
    m_stTargetAddress->updateAddressAndPort(szAddr, unPort);
    Connect(m_stTargetAddress);
    m_enmConnectionStatus = enmConnectionStatus_Opened;
}
TcpConnection::TcpConnection()
    : Socket(enmAddressFamily_INET, enmSocketType_STREAM, enmProtocolType_TCP)
{
    _InternalInit();
}

TcpConnection::~TcpConnection()
{
    delete m_stTargetAddress;
    m_stTargetAddress = NULL;
}

EnmConnectionStatus TcpConnection::enmConnectionStatus()
{
    return m_enmConnectionStatus;
}

Address* TcpConnection::GetTargetAddress()
{
    return m_stTargetAddress;
}

void TcpConnection::SetTargetAddress(const std::string& szAddr, uint16_t unPort)
{
    m_stTargetAddress->updateAddressAndPort(szAddr, unPort);
}

void TcpConnection::Connect(Address* pTargetAddr)
{
    //AURORA_LOG(ERROR) << "kcp:log tcp connect";
    //cx log
    if ((pTargetAddr == NULL) || !pTargetAddr->isValid())
    {
        //NET_LOG(ERROR) << "param pTargetAddr is invalid";
        //cx log
        return;
    }

    //移到成功连接后再清理接收队列 避免提前清理掉有用的数据
    //接收队列也需要清理一下，否则异常情况下上次只收到一半的包后连接上的新数据包会被用来填补上次的包，就一直全错了
    //cx m_stRecieveCodeRoundRobinQueue->RemoveDataFromBuffer(m_stRecieveCodeRoundRobinQueue->GetUsedBufferSize());

    m_stTargetAddress->updateAddressAndPort(pTargetAddr);
    bool bIpv6 = m_stTargetAddress->isIPv6();
    SetAddressFamily(bIpv6 ? enmAddressFamily_INET6 : enmAddressFamily_INET);

    if (enmConnectionStatus_Error == m_enmConnectionStatus)
    {
        CloseConn("TcpConnection::Connect1");
    }

    if (GetSocketID() == Invalid_SocketID)
    {
       // NET_LOG(INFO) << "connect handle invalid:" << GetSocketID();
        //cx log
        _ResetSocketHandle();

        int32_t iFlags = 1;
        linger stLinger;
        stLinger.l_linger = 0;
        stLinger.l_onoff = 1;

        SetOption(enmSocketOptionLevel_SOCKET, enmSocketOption_SO_REUSEADDR, iFlags);
        SetOption(enmSocketOptionLevel_SOCKET, enmSocketOption_SO_KEEPALIVE, iFlags);
#if AURORA_OS_IOS == aurora_OS
        {
            int set = 1;
            setsockopt(GetSocketID(), SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
        }
#endif
        //会影响server端收到socket close 事件
        //SetOption(enmSocketOptionLevel_SOCKET, enmSocketOption_SO_LINGER, stLinger);

        SetBlocking(false);
    }

    //NET_LOG(INFO) << "current socket ID:" << GetSocketID();
    //cx log
    Socket::Connect(*m_stTargetAddress);
    SetOption(enmSocketOptionLevel_TCP, enmSocketOption_TCP_NODELAY, 1);

    if (!IsBlocking())
    {
        //TODO: 后面如果有多线程同时使用的情况要加__thread
        static SocketSet stReadSocketSet;
        static SocketSet stWriteSocketSet;
        static SocketSet stErrorSocketSet;

        stReadSocketSet.Reset();
        stWriteSocketSet.Reset();
        stErrorSocketSet.Reset();
        stReadSocketSet.Add(*this);
        stWriteSocketSet.Add(*this);
        //stErrorSocketSet.add(this);

        int32_t iResult = Socket::Select(stReadSocketSet, stWriteSocketSet, stErrorSocketSet, m_iTimeOutMicroseconds);

        //NET_LOG(INFO) << "connect select result:" << iResult;
        //cx log
        if (iResult <= 0)
        {
            //NET_LOG(INFO) << "connect select errno:" << LastErrorNo() << " | errstr:" << LastErrorString();
            //cx log
            m_enmConnectionStatus = enmConnectionStatus_Error;

            return;
        }

        //NET_LOG(INFO) << "stReadSocketSet isset this:" << stReadSocketSet.IsSet(*this);
        //NET_LOG(INFO) << "stWriteSocketSet isset this:" << stWriteSocketSet.IsSet(*this);
        // cx log
        if (iResult > 0 && stReadSocketSet.IsSet(*this))
        {
            static char szBuffer[10];
            ssize_t iRecvResult = Receive(szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]));

            //NET_LOG(INFO) << "Socket Recieve Result:" << (int32_t)iRecvResult;
            //cx log
            if (iRecvResult < 0)
            {
                m_enmConnectionStatus = enmConnectionStatus_Error;

                //AURORA_LOG(ERROR) << "connect recieve error connect failed.";
                // cx log
                return;
            }
        }
    }
    m_enmConnectionStatus = enmConnectionStatus_Connected;
}

void TcpConnection::ReConnect(int iTimeOutMicroseconds /*= 1000000*/)
{
    m_iTimeOutMicroseconds = iTimeOutMicroseconds;
    if (m_stTargetAddress->isValid())
    {
        Connect(m_stTargetAddress);
    }
    else
    {
        //NET_LOG(ERROR) << "stTargetAddress is invalid";
        // cx log
    }
}

void TcpConnection::ClosedByServer()
{
    CloseConn("TcpConnection::ClosedByServer");
}

void TcpConnection::CloseConn(const std::string& strReason)
{
    Socket::Close();
    m_enmConnectionStatus = enmConnectionStatus_Closed;
}

int32_t TcpConnection::SendTcpData(char *senddata_buffer, int32_t buffer_len)
{
    char *m_tmp = senddata_buffer;
    int32_t remain_len = buffer_len;
    while (remain_len > 0)
    {
        int32_t iSendData = (int32_t)this->Send(m_tmp, remain_len);
        if (iSendData > 0)
        {
            m_tmp += iSendData;
            remain_len -= iSendData;
        }
        else
        {
            //NET_LOG(INFO) << "kcp:log tcp this.send errno:" << LastErrorNo() << " | errstr:" << LastErrorString();
            //cx log
            break;

            if (EAGAIN == LastErrorNo()) //tcp缓冲区满暂时没法发送 稍后再试
            {
                break;
            }

            if (EINTR != LastErrorNo())
            {
                break;
            }
        }
    }
    return buffer_len - remain_len;
}

int32_t TcpConnection::ReceiveTcpData(char *buf_data, int32_t buffer_len)
{
    buffer_len = buffer_len > max_client_pkg_size ? max_client_pkg_size : buffer_len;
    int32_t receive_socket_len = (int32_t)this->Receive(buf_data, buffer_len);
    if (receive_socket_len > 0)
    {
        return receive_socket_len;
    }
    else if (0 == receive_socket_len)
    {
        ClosedByServer();
        return -1;
    }
    return 0;
}

void TcpConnection::_ResetSocketHandle()
{
    CloseConn("TcpConnection::_ResetSocketHandle");

    socket_t iNewSocketID = socket(GetAddressFamily(), enmSocketType_STREAM, enmProtocolType_TCP);

    if (Invalid_SocketID == iNewSocketID)
    {
        //AURORA_LOG(ERROR) << "ResetSocketHandle Error: Invalid_SocketID == iNewSocketID";
        //cx log
    }
    else
    {
        SetSock(iNewSocketID);
    }
}

void TcpConnection::_InternalInit()
{
    m_enmConnectionStatus = enmConnectionStatus_Opened;
    m_stTargetAddress = new InternetAddress();
    m_iTimeOutMicroseconds = 1000000; //1s连接超时

    int32_t iFlags = 1;
    linger stLinger;
    stLinger.l_linger = 0;
    stLinger.l_onoff = 1;
    SetOption(enmSocketOptionLevel_SOCKET, enmSocketOption_SO_REUSEADDR, iFlags);
    SetOption(enmSocketOptionLevel_SOCKET, enmSocketOption_SO_KEEPALIVE, iFlags);

#if AURORA_OS_IOS == aurora_OS
    {
        int set = 1;
        setsockopt(GetSocketID(), SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
    }
#endif
    //会影响server端收到socket close 事件
    //SetOption(enmSocketOptionLevel_SOCKET, enmSocketOption_SO_LINGER, stLinger);
    SetBlocking(false);
}

}

