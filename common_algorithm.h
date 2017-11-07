#ifndef _NETWORK_COMMON_ALGORITHM_H__
#define _NETWORK_COMMON_ALGORITHM_H__
#include "common_base.h"
#include "common_codeengine.h"
namespace NetWork
{
//////////////////////////////////////////////////////////
//// 环形队列
//////////////////////////////////////////////////////////
class AurCodeRoundRobinQueue
{
public:
    AurCodeRoundRobinQueue(int32_t iTotalBufferSize)
        : m_iDataBufferLen(iTotalBufferSize + 1)
    {
        m_szDataBuffer = new char[m_iDataBufferLen];
        m_iReadPosFlag = 0;
        m_iWritePosFlag = 0;
    }
    ~AurCodeRoundRobinQueue()
    {
        delete m_szDataBuffer;
        m_iReadPosFlag = 0;
        m_iWritePosFlag = 0;
    }
    int32_t GetFreeBufferSize()
    {
        return (int32_t)((m_iReadPosFlag - m_iWritePosFlag + m_iDataBufferLen - 1) % (m_iDataBufferLen));
    }
    int32_t GetUsedBufferSize()
    {
        return (int32_t)((m_iWritePosFlag - m_iReadPosFlag + m_iDataBufferLen) % (m_iDataBufferLen));
    }
    //给上层用来从缓冲队列里读取一个完整数据包给上层处理
    int32_t GetOneCode(char* szDataCodeBuffer, const int32_t iDataCodeBufferLen)  //去掉ref 因为char[]本身就是引用传递
    {
        uint32_t uiUsedBufferSize = GetUsedBufferSize();
        if (uiUsedBufferSize < 4)
        {
            return 0;
        }
        char str_len_data[6];
        CopyDataFromBuffer(str_len_data, 4);
        uint32_t uiCodeLength;
        char* str = &str_len_data[0];
        CCodeEngine::decode_int32(&str, &uiCodeLength);
        if (uiCodeLength <= 0)
        {
            //cx log;
            printf("len < 0 error");
            return 0;
        }
        if (uiCodeLength <= uiUsedBufferSize)
        {
            if (m_iDataBufferLen - m_iReadPosFlag >= (int32_t)uiCodeLength)
            {
                memcpy(&szDataCodeBuffer[0], &m_szDataBuffer[m_iReadPosFlag], uiCodeLength);
                //szDataCodeBuffer[0..uiCodeLength] = m_szDataBuffer[m_iReadPosFlag..m_iReadPosFlag+uiCodeLength];
            }
            else
            {
                memcpy(&szDataCodeBuffer[0], &m_szDataBuffer[m_iReadPosFlag], m_iDataBufferLen - m_iReadPosFlag);
                memcpy(&szDataCodeBuffer[m_iDataBufferLen - m_iReadPosFlag], &m_szDataBuffer[0], uiCodeLength - (m_iDataBufferLen - m_iReadPosFlag));
                //szDataCodeBuffer[0..m_iDataBufferLen-m_iReadPosFlag] = m_szDataBuffer[m_iReadPosFlag..$];
                //szDataCodeBuffer[m_iDataBufferLen-m_iReadPosFlag..uiCodeLength] = m_szDataBuffer[0..uiCodeLength-$+m_iReadPosFlag];
            }

            m_iReadPosFlag = (int32_t)((m_iReadPosFlag + uiCodeLength) % m_iDataBufferLen);

            return uiCodeLength;
        }
        else
        {
            //AURORA_LOG(INFO) << "uiCodeLength:" << uiCodeLength << " > uiUsedBufferSize:" << uiUsedBufferSize << ", wait for left package";
            //cx log
            //mixin (AurLoggerInstance(EnmLogLevel.Info)).Writeln("uiCodeLength > uiUsedBufferSize:", uiUsedBufferSize, ", wait for left package");

            return 0;
        }

        return 0;
    }
    //给上层用来一次性写入一个完整包数据的。
    int32_t WriteOneCode(char* szDataCodeBuffer, const int32_t iDataCodeBufferLen)  //去掉ref 因为char[]本身就是引用传递
    {
        if (iDataCodeBufferLen <= 4)
        {
            //cx log
            return -1;
        }
        int32_t uiFreeBufferSize = GetFreeBufferSize();
        if (uiFreeBufferSize < iDataCodeBufferLen)
        {
            //cx log
            return -1;
        }
        uint32_t uiCodeLength;
        char* str = &szDataCodeBuffer[0];
        CCodeEngine::decode_int32(&str, &uiCodeLength);
        if (uiCodeLength != iDataCodeBufferLen)
        {
            //cx log
            return -1;
        }

        if (m_iDataBufferLen - m_iWritePosFlag >= (int32_t)uiCodeLength)
        {
            memcpy(&m_szDataBuffer[m_iWritePosFlag], &szDataCodeBuffer[0], uiCodeLength);
            //m_szDataBuffer[m_iWritePosFlag..m_iWritePosFlag+uiCodeLength] = szDataCodeBuffer[0..uiCodeLength];
        }
        else
        {
            memcpy(&m_szDataBuffer[m_iWritePosFlag], &szDataCodeBuffer[0], m_iDataBufferLen - m_iWritePosFlag);
            memcpy(&m_szDataBuffer[0], &szDataCodeBuffer[m_iDataBufferLen - m_iWritePosFlag], uiCodeLength - (m_iDataBufferLen - m_iWritePosFlag));
            //m_szDataBuffer[m_iWritePosFlag..$] = szDataCodeBuffer[0..m_iDataBufferLen-m_iWritePosFlag];
            //m_szDataBuffer[0..uiCodeLength-$+m_iWritePosFlag] = szDataCodeBuffer[m_iDataBufferLen-m_iWritePosFlag..uiCodeLength];
        }

        m_iWritePosFlag = (int32_t)((m_iWritePosFlag + uiCodeLength) % m_iDataBufferLen);

        return uiCodeLength;
    }
    //给底层socket发送数据时使用
    int32_t CopyDataFromBuffer(char* szDataCodeBuffer, const int32_t iDataCodeBufferLen) //去掉ref 因为本身就是引用传递
    {
        int32_t iCopyDataLen = GetUsedBufferSize();
        if (iDataCodeBufferLen < iCopyDataLen)
        {
            iCopyDataLen = (int32_t)iDataCodeBufferLen;
        }
        if (iCopyDataLen > 0)
        {
            if (m_iDataBufferLen - m_iReadPosFlag >= iCopyDataLen)
            {
                memcpy(&szDataCodeBuffer[0], &m_szDataBuffer[m_iReadPosFlag], iCopyDataLen);
            }
            else
            {
                memcpy(&szDataCodeBuffer[0], &m_szDataBuffer[m_iReadPosFlag], m_iDataBufferLen - m_iReadPosFlag);
                memcpy(&szDataCodeBuffer[m_iDataBufferLen - m_iReadPosFlag], &m_szDataBuffer[0], iCopyDataLen - (m_iDataBufferLen - m_iReadPosFlag));
            }
        }

        return iCopyDataLen;
    }
    //给底层socket发送数据时使用 发送成功的数量
    int32_t RemoveDataFromBuffer(int32_t iDataSize)
    {
        if (iDataSize >= 0 && iDataSize <= GetUsedBufferSize())
        {
            m_iReadPosFlag = (int32_t)((m_iReadPosFlag + iDataSize) % m_iDataBufferLen);

            return iDataSize;
        }

        return -1;
    }
    //给底层socket接收数据时使用 写入接收的数量
    int32_t WriteDataToBuffer(const char* szDataCodeBuffer, const int32_t iDataCodeBufferLen)  //去掉ref 因为本身就是引用传递
    {
        uint32_t uiFreeBufferSize = GetFreeBufferSize();
        if ((int32_t)uiFreeBufferSize < iDataCodeBufferLen || iDataCodeBufferLen <= 0)
        {
            //AURORA_LOG(ERROR) << "Error:uiFreeBufferSize[" << uiFreeBufferSize << "] < iDataCodeBufferLen:" << iDataCodeBufferLen << ", WriteOneCode failed";
            //mixin (AurLoggerInstance(EnmLogLevel.Info)).Writeln("Error:uiFreeBufferSize[", uiFreeBufferSize, "] < iDataCodeBufferLen:", iDataCodeBufferLen, ", WriteOneCode failed");
            //cx log
            return -1;
        }
        uint32_t uiCodeLength = (int32_t)iDataCodeBufferLen;
        if (m_iDataBufferLen - m_iWritePosFlag >= (int32_t)uiCodeLength)
        {
            memcpy(&m_szDataBuffer[m_iWritePosFlag], &szDataCodeBuffer[0], uiCodeLength);
            //m_szDataBuffer[m_iWritePosFlag..m_iWritePosFlag+uiCodeLength] = szDataCodeBuffer[0..uiCodeLength];
        }
        else
        {
            memcpy(&m_szDataBuffer[m_iWritePosFlag], &szDataCodeBuffer[0], m_iDataBufferLen - m_iWritePosFlag);
            memcpy(&m_szDataBuffer[0], &szDataCodeBuffer[m_iDataBufferLen - m_iWritePosFlag], uiCodeLength - (m_iDataBufferLen - m_iWritePosFlag));
            //m_szDataBuffer[m_iWritePosFlag..$] = szDataCodeBuffer[0..m_iDataBufferLen-m_iWritePosFlag];
            //m_szDataBuffer[0..uiCodeLength-$+m_iWritePosFlag] = szDataCodeBuffer[m_iDataBufferLen-m_iWritePosFlag..uiCodeLength];
        }

        m_iWritePosFlag = (int32_t)((m_iWritePosFlag + uiCodeLength) % m_iDataBufferLen);

        return uiCodeLength;
    }
private:
    AurCodeRoundRobinQueue();
private:
    const int32_t m_iDataBufferLen;
    char* m_szDataBuffer;
    int32_t m_iReadPosFlag;
    int32_t m_iWritePosFlag;
};
}
#endif
