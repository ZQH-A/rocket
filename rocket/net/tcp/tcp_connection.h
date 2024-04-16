#ifndef ROCKET_NET_TCP_CONNECTION_H
#define ROCKET_NET_TCP_CONNECTION_H

#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/net/io_thread.h"
#include <memory>

namespace rocket{

    //状态的枚举值
    enum TcpState{
        NotConnection = 1, //未连接
        Connected = 2,  //连接成功
        HalfClosing = 3,  //半连接  半关闭状态
        Closed = 4,  //关闭
    };
    class TcpConnection
    {
    public:
        typedef std::shared_ptr<TcpConnection> s_ptr;
    private:
        /* data */
        IOThread* m_io_thread {NULL}; //代表持有该连接的 IO线程

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
    public:
        TcpConnection(IOThread* io_thread,int fd,int buffer_size,NetAddr::s_ptr peer_addr);
        ~TcpConnection();

        void onRead();

        void excute();

        void onWrite();

        void setState(const TcpState state);

        TcpState getState();

        void clear(); //处理关闭事件

        void shutdown(); //f服务器主动关闭连接
    };
        
}

#endif