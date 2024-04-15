#ifndef ROCKET_NET_TCP_NET_ADDR_H
#define ROCKET_NET_TCP_NET_ADDR_H


#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>

namespace rocket{

    class NetAddr{ //网络地址的基类
        public:

            typedef std::shared_ptr<NetAddr> s_ptr;

            virtual sockaddr* getSockAddr() = 0;

            virtual socklen_t getSockLen() = 0;

            virtual int getFamily() = 0;

            virtual std::string toString() = 0; //将IP地址和端口号打印成string字符串

            virtual bool checkValid() = 0;  //检测传进来的地址是否合法
        private:
    };

    class IPNetAddr :public NetAddr{

        public:
            IPNetAddr(const std::string& ip, uint16_t port);

            IPNetAddr(const std::string addr);

            IPNetAddr(sockaddr_in addr);  //使用IPv4地址赋值

            sockaddr* getSockAddr();

            socklen_t getSockLen();

            int getFamily();

            std::string toString(); //将IP地址和端口号打印成string字符串

            bool checkValid();

        private:
            std::string m_ip;
            uint16_t m_port {0};
            sockaddr_in m_addr;
    };
}

#endif