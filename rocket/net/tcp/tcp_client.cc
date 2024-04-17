#include "rocket/net/tcp/tcp_client.h"
#include "rocket/common/log.h"
#include <sys/socket.h>
#include "rocket/net/fd_event_group.h"
#include <unistd.h>
#include <string.h>

namespace rocket{

    TcpClient::TcpClient(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr)
    {
        m_event_loop = EventLoop::GetGurrentEventLoop();
        m_fd = socket(peer_addr->getFamily(),SOCK_STREAM,0);

        if(m_fd < 0)
        {
            ERRORLOG("TcpClient::TcpClient() error, failed to create fd");
            return;
        }

        m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);

        m_connection = std::make_shared<TcpConnection>(m_event_loop,m_fd,128,peer_addr);
        m_connection->setConnectionType(TcpConnectionByClient);
    }
    TcpClient::~TcpClient()
    {
        if(m_fd > 0) //关闭连接
        {
            close(m_fd);
        }
    }
    //如果connect成功，done会被执行
    void TcpClient::connect(std::function<void()> done) //异步的进行connect 
    {
        int rt = ::connect(m_fd,m_peer_addr->getSockAddr(),m_peer_addr->getSockLen());

        if(rt ==0)
        {
            DEBUGLOG("connect [%s] success",m_peer_addr->toString().c_str());
            if(done)
            {
                done(); //执行done函数
            }

        }else if(rt == -1)
        {
            if(errno == EINPROGRESS)
            {
                //epoll监听可写事件，然后判断错误码
                m_fd_event->listen(FdEvent::OUT_EVENT,[this,done](){
                    int error = 0;
                    socklen_t error_len = sizeof(error);
                    getsockopt(m_fd,SOL_SOCKET,SO_ERROR,&error,&error_len);

                    if(error == 0)
                    {
                         DEBUGLOG("connect [%s] success",m_peer_addr->toString().c_str());
                        if(done)
                        {
                            done(); //执行done函数
                        }
                    }else{
                        ERRORLOG("connect error, errno =%d, error =%s",errno,strerror(errno));
                    }
                    m_fd_event->cancle(FdEvent::OUT_EVENT);
                    m_event_loop->addEpollEvent(m_fd_event);
                });
                m_event_loop->addEpollEvent(m_fd_event);

                if(!m_event_loop->isLooping())
                {
                    m_event_loop->loop();
                }
            }else{
                ERRORLOG("connect error, errno =%d, error =%s",errno,strerror(errno));
            }
        }

    }
    //异步的发送Message
    //如果发送message成功，会调用done函数，函数入参就是message对象
    void TcpClient::writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done)
    {

    }

    //异步的读取Message
    //如果读取message成功，会调用done函数，函数入参就是message对象
    void TcpClient::readMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done)
    {

    }
}