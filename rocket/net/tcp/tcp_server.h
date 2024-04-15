#ifndef ROCKET_NET_TCP_SERVER_H
#define ROCKET_NET_TCP_SERVER_H

#include "rocket/net/tcp/tcp_acceptor.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/io_thread_group.h"

namespace rocket{
    class TcpServer
    {
    private:
        /* data */
        TcpAcceptor::s_ptr m_acceptor;
        NetAddr::s_ptr m_local_addr; //本地监听地址

        EventLoop* m_main_event_loop {NULL}; //主线程的 eventloop mainReactor
        IOThreadGroup* m_io_thread_group {NULL}; //subReactor 组

        FdEvent* m_listen_fd_event; //监听套接字

        int m_client_counts {0}; //当前连接的数量
    public:
        TcpServer(NetAddr::s_ptr local_addr);
        ~TcpServer();
        
        void start(); //开启event loop 循环
    private:
        void init();

        void onAccept(); //新连接到来时 需要执行
    }; 
}

#endif