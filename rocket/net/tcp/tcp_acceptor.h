#ifndef ROCKET_NET_TCP_ACCEPTOR_H
#define ROCKET_NET_TCP_ACCEPTOR_H

#include "rocket/net/tcp/net_addr.h"
#include <memory>

namespace rocket{
    class TcpAcceptor{

        public:
            typedef std::shared_ptr<TcpAcceptor> s_ptr; //定义智能指针并去别名

            TcpAcceptor(NetAddr::s_ptr local_addr);
            ~TcpAcceptor();

            std::pair<int,NetAddr::s_ptr> accept(); //连接

            int getListenFd();
        private:
            NetAddr::s_ptr m_local_addr; //服务端监听的地址： addr->ip:port 为什么要用基类指针？ 为了后面可以扩展到UNix等
            //listenfd
            int m_listenfd {-1}; //监听套接字

            int m_family {-1};  //地址协议族

    };
}
#endif