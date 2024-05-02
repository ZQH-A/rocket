#ifndef ROCKET_NET_TCP_CLIENT_H
#define ROCKET_NET_TCP_CLIENT_H

#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/coder/abstract_protocol.h"
#include <memory>
#include "rocket/net/timer_event.h"

namespace rocket{

    class TcpClient
    {
    private:
        /* data */
        NetAddr::s_ptr m_peer_addr;
        NetAddr::s_ptr m_local_addr;

        EventLoop* m_event_loop {NULL};

        int m_fd {-1};
        FdEvent* m_fd_event {NULL};

        TcpConnection::s_ptr m_connection;

        int m_connect_errcode {0}; //连接失败错误码
        std::string m_connect_error_info; //连接失败错误信息
    public:
        typedef std::shared_ptr<TcpClient> s_ptr;
    public:
        TcpClient(NetAddr::s_ptr peer_addr);
        ~TcpClient();
        //如果connect成功，done会被执行
        void connect(std::function<void()> done); //异步的进行connect 

        //异步的发送Message
        //如果发送message成功，会调用done函数，函数入参就是message对象
        void writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

        //异步的读取Message
        //如果读取message成功，会调用done函数，函数入参就是message对象
        void readMessage(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done);

        void stop(); //结束eventloop循环

        int getConnectErrorCode();
        std::string getConnectErrorInfo();

        NetAddr::s_ptr getPeerAddr();

        NetAddr::s_ptr getLocalAddr();

        void initLocalAddr();

        void addTimerEvent(TimerEvent::s_ptr timer_event);

    };
        
}


#endif