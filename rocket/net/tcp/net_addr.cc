#include "rocket/net/tcp/net_addr.h"
#include <string.h>
#include "rocket/common/log.h"

namespace rocket{

    IPNetAddr::IPNetAddr(const std::string& ip, uint16_t port): m_ip(ip),m_port(port)
    {
        memset(&m_addr,0, sizeof(m_addr));  //结构体全部初始化为0 好习惯

        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
        m_addr.sin_port = htons(m_port);
    }

    IPNetAddr::IPNetAddr(const std::string addr)
    { //从string字符串中提取ip 和 port
        size_t i = addr.find_first_of(":"); //找到第一个 ： 的地方
        if ( i == addr.npos)  //如果在末尾，则是没有找到，说明不是正确的网络Ip
        {
            ERRORLOG("invalid ipv4 addr %s", addr.c_str());
            return;
        }

        m_ip = addr.substr(0,i);
        m_port = std::atoi(addr.substr(i+1,addr.size() - i - 1).c_str());

        memset(&m_addr,0, sizeof(m_addr));  //结构体全部初始化为0 好习惯

        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
        m_addr.sin_port = htons(m_port);
    }

    IPNetAddr::IPNetAddr(sockaddr_in addr): m_addr(addr)  //使用IPv4地址赋值
    {
        m_ip = std::string(inet_ntoa(m_addr.sin_addr));
        m_port = ntohs(m_addr.sin_port);
    }

    sockaddr* IPNetAddr::getSockAddr()
    {
        return reinterpret_cast<sockaddr*> (&m_addr);
    }

    socklen_t IPNetAddr::getSockLen()
    {
        return sizeof(m_addr);
    }

    int IPNetAddr::getFamily()
    {
        return AF_INET;
    }

    std::string IPNetAddr::toString() //将IP地址和端口号打印成string字符串
    {
        std::string re;
        re = m_ip + ":" + std::to_string(m_port);
        return re;
    }

    bool IPNetAddr::checkValid()
    {
        if(m_ip.empty())
        {
            return false;
        }

        if(m_port < 0 || m_port > 65536)
        {
            return false;
        }

        if(inet_addr(m_ip.c_str()) == INADDR_NONE)
        {
            return false;
        }

        return true;
    }

}