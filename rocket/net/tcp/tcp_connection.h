#ifndef ROCKET_NET_TCP_CONNECTION_H
#define ROCKET_NET_TCP_CONNECTION_H

#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/net/io_thread.h"
#include <memory>
#include "rocket/net/eventloop.h"
#include <queue> 
#include "rocket/net/coder/abstract_protocol.h"
#include "rocket/net/coder/abstract_coder.h"

namespace rocket{

    //状态的枚举值
    enum TcpState{
        NotConnection = 1, //未连接
        Connected = 2,  //连接成功
        HalfClosing = 3,  //半连接  半关闭状态
        Closed = 4,  //关闭
    };

    enum TcpConnectionType{
        TcpConnectionByServer = 1,  //作为服务端使用，代表跟客户端的连接
        TcpConnectionByClient = 2, //作为客户端使用，代表跟服务端的连接
    };

    class TcpConnection
    {
    public:
        typedef std::shared_ptr<TcpConnection> s_ptr;
    private:
        /* data */
        EventLoop* m_event_loop {NULL}; //代表持有该连接的 IO线程

        NetAddr::s_ptr m_local_addr;  //本地地址
        NetAddr::s_ptr m_peer_addr; //对端地址

        //知道为什么要有一个读指针 一个写指针了，在接收缓冲区中，先使用系统调用write把接收到的数据写到接收缓冲区中
        //然后在使用readFromBuffer从buffer里面读。 发送缓冲区同理，先使用writeToBuffer写入数据到buffer，然后
        //使用read从buffer里面读取数据然后发送到对端。
        TcpBuffer::s_ptr  m_in_buffer; //接收缓冲区 
        TcpBuffer::s_ptr  m_out_buffer; //发送缓冲区

        
        FdEvent* m_fd_event {NULL};

        TcpState m_state; //当前连接的状态

        int m_fd {0};

        TcpConnectionType m_connection_type {TcpConnectionByServer};

        //存储写事件的回调函数  
        //1.为什么要用pair，且有个参数是AbstractProtocol::s_ptr，因为回调函数中的参数是这个，所以需要一直保存这个参数，直到调用的时候
        //而需要保存两个相对应的参数，所以使用pair
        std::vector<std::pair<AbstractProtocol::s_ptr,std::function<void(AbstractProtocol::s_ptr)>>> m_write_dones;

        std::map<std::string,std::function<void(AbstractProtocol::s_ptr)>> m_read_dones;

        AbstractCoder* m_coder {NULL}; 
    public:
        TcpConnection(EventLoop* event_loop,int fd,int buffer_size,NetAddr::s_ptr peer_addr, TcpConnectionType type = TcpConnectionByServer);
        ~TcpConnection();

        void onRead();

        void excute();

        void onWrite();

        void setState(const TcpState state);

        TcpState getState();

        void clear(); //处理关闭事件

        void shutdown(); //f服务器主动关闭连接
    public:
        void setConnectionType(TcpConnectionType type);
        //监听可写事件
        void listenWrite();
        //监听可读事件
        void listenRead();

        void pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

        void pushReadMessage(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done);
    };
        
}

#endif