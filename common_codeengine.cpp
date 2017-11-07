#include "common_codeengine.h"
namespace NetWork
{
int32_t CCodeEngine::encode_int8(char **pOut, uint8_t Src)
{
    if (NULL == pOut || NULL == *pOut)
    {
        return 0;
    }

    **pOut = (char)Src;
    (*pOut)++;

    return int32_t(sizeof(uint8_t));
}

int32_t CCodeEngine::decode_int8(char **pIn, uint8_t *pOut)
{
    if (NULL == pIn || NULL == *pIn || NULL == pOut)
    {
        return 0;
    }

    *pOut = **pIn;
    (*pIn)++;

    return int32_t(sizeof(uint8_t));
}

int32_t CCodeEngine::encode_int16(char **pOut, uint16_t Src)
{
    if (NULL == pOut || NULL == *pOut)
    {
        return 0;
    }
    /*
    **pOut = char( (Src & 0xff00) >> 8 );
    (*pOut)++;

    **pOut = char(Src & 0xff);
    (*pOut)++;
    */
    *(uint16_t*)(*pOut) = htons(Src);
    *pOut += sizeof(uint16_t);
    return int32_t(sizeof(uint16_t));
}

int32_t CCodeEngine::decode_int16(char **pIn, uint16_t* pOut)
{
    if (NULL == pIn || NULL == *pIn || NULL == pOut)
    {
        return 0;
    }
    /*
    *pOut = (unsigned char)**pIn;
    *pOut <<= 8;
    (*pIn)++;
    *pOut += (unsigned char)**pIn;
    (*pIn)++;
    */
    *pOut = (uint16_t)ntohs((uint16_t)*(uint16_t*)(*pIn));
    *pIn += sizeof(uint16_t);
    return int32_t(sizeof(uint16_t));
}

int32_t CCodeEngine::encode_int32(char **pOut, uint32_t Src)
{
    if (NULL == pOut || NULL == *pOut)
    {
        return 0;
    }
    /*
    **pOut = (char)((Src & 0xff000000) >> 24 );
    (*pOut)++;
    **pOut = (char)((Src & 0xff0000) >> 16);
    (*pOut)++;
    **pOut = (char)((Src & 0xff00) >> 8);
    (*pOut)++;
    **pOut = (char)(Src & 0xff);
    (*pOut)++;
    */
    *(uint32_t*)(*pOut) = htonl(Src);
    *pOut += sizeof(uint32_t);
    return int32_t(sizeof(uint32_t));
}

int32_t CCodeEngine::decode_int32(char **pIn, uint32_t *pOut)
{
    if (NULL == pIn || NULL == *pIn || NULL == pOut)
    {
        return 0;
    }
    /*
    *pOut = (unsigned char)**pIn;
    *pOut <<= 8;
    (*pIn)++;

    *pOut += (unsigned char)**pIn;
    *pOut <<= 8;
    (*pIn)++;

    *pOut += (unsigned char)**pIn;
    *pOut <<= 8;
    (*pIn)++;

    *pOut += (unsigned char)**pIn;
    (*pIn)++;
    */
    *pOut = (uint32_t)ntohl((uint32_t)*(uint32_t*)(*pIn));
    *pIn += sizeof(uint32_t);
    return int32_t(sizeof(uint32_t));
}

int32_t CCodeEngine::encode_string(char **pOut, const char *pSrc, const int16_t nMaxStringLength)
{
    if (NULL == pOut || NULL == *pOut || NULL == pSrc || 0 >= nMaxStringLength)
    {
        return 0;
    }

    int16_t tmp_string_length = (int16_t)strlen(pSrc);
    //	if (tmp_string_length != 0)
    //	{
    tmp_string_length += 1;
    //	}

    if (tmp_string_length  > nMaxStringLength)
    {
        tmp_string_length = nMaxStringLength;
    }

    //首先编入字符串的长度
    int32_t coded_length = encode_int16(pOut, tmp_string_length);

    if (tmp_string_length == 0) //空串
    {

        return coded_length;
    }

    strncpy(*pOut, pSrc, tmp_string_length);
    (*pOut)[tmp_string_length - 1] = '\0';			//保证编码进去的string一定以'\0'结尾
    *pOut += (tmp_string_length);

    return (coded_length + tmp_string_length);
}

int32_t CCodeEngine::decode_string(char **pIn, char *pOut, const int16_t nMaxStringLength)
{
    if (NULL == pIn || NULL == *pIn || NULL == pOut || 0 >= nMaxStringLength)
    {
        return 0;
    }

    int16_t string_length = 0;

    int32_t tmp_length = decode_int16(pIn, (uint16_t*)&string_length);
    // 	if (0 == tmp_length)
    // 	{
    // 		return 0;
    // 	}

    if (string_length <= 0)
    {
        pOut[0] = '\0';
        return tmp_length;
    }

    int16_t real_length = string_length;
    if (string_length > nMaxStringLength)
    {
        real_length = nMaxStringLength;
    }

    strncpy(pOut, (*pIn), real_length);

    *pIn += string_length; //NOTE! 不能只移动real_length大小
    //pOut[real_length] = '\0';
    pOut[real_length - 1] = '\0';

    return(string_length + tmp_length);
}

int32_t CCodeEngine::encode_memory(char **pOut, const char *pSrc, const int32_t iMemorySize)
{
    if (NULL == pOut || NULL == *pOut || NULL == pSrc || 0 >= iMemorySize)
    {
        return 0;
    }

    memcpy((void*)*pOut, (const void*)pSrc, iMemorySize);
    *pOut += iMemorySize;

    return iMemorySize;
}

int32_t CCodeEngine::decode_memory(char **pIn, char *pOut, const int32_t iMemorySize)
{
    if (NULL == pIn || NULL == *pIn || NULL == pOut || 0 >= iMemorySize)
    {
        return 0;
    }

    memcpy((void*)pOut, (const void*)*pIn, iMemorySize);
    *pIn += iMemorySize;

    return iMemorySize;
}

int32_t CCodeEngine::encode_int64(char **pOut, uint64_t Src)
{
    if (NULL == pOut || NULL == *pOut)
    {
        return 0;
    }

    uint32_t part = 0;
    uint32_t tmp = 0;

    //high bits
    part = uint32_t((Src >> 32) & 0xffffffff);
    tmp = htonl(part);
    memcpy((void*)*pOut, (const void*)&tmp, sizeof(tmp));
    *pOut += sizeof(tmp);

    //low bits
    part = uint32_t(Src & 0xffffffff);
    tmp = htonl((uint32_t)part);
    memcpy((void*)*pOut, (const void*)&tmp, sizeof(tmp));
    *pOut += sizeof(tmp);

    return int32_t(sizeof(uint64_t));
}

int32_t CCodeEngine::decode_int64(char **pIn, uint64_t *pOut)
{
    uint32_t part = 0;
    uint64_t tmp = 0;

    memcpy((void*)&part, (const void*)*pIn, sizeof(part));
    tmp = ntohl(part);
    tmp <<= 32;
    *pIn += sizeof(part);

    memcpy((void*)&part, (const void*)*pIn, sizeof(part));
    tmp += ntohl(part);
    *pIn += sizeof(part);

    *pOut = tmp;

    return int32_t(sizeof(uint64_t));
}
}