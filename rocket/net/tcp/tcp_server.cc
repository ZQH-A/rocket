#include "rocket/net/tcp/tcp_server.h"
#include "rocket/common/log.h"
#include "rocket/net/eventloop.h"


namespace rocket{

    TcpServer::TcpServer(NetAddr::s_ptr local_addr):m_local_addr(local_addr)
    {
        init();

        INFOLOG("rocket TcpServer listen sucess on [%s]",m_local_addr->toString().c_str());

    }
    TcpServer::~TcpServer()
    {
        if(m_main_event_loop)
        {
            delete m_main_event_loop;
            m_main_event_loop = NULL;
        }

    }

    void TcpServer::start() //开启event loop 循环
    {
        m_io_thread_group->start();
        m_main_event_loop->loop();
    }

     void TcpServer::init() //初始化
     {
        m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);

        m_main_event_loop = EventLoop::GetGurrentEventLoop(); //获取主线程的eventloop
        m_io_thread_group = new IOThreadGroup(2);        //还需要给它里面的iothread中的eventloop添加eventfd进行监听

        //将监听套接字上树，并绑定其对应的触发函数。
        m_listen_fd_event =  new FdEvent(m_acceptor->getListenFd());

        m_listen_fd_event->listen(FdEvent::IN_EVENT,std::bind(&TcpServer::onAccept,this));
        m_main_event_loop->addEpollEvent(m_listen_fd_event);
     }

     void TcpServer::onAccept() //新连接到来时 需要执行
     {
        int client_fd = m_acceptor->accept();
        m_client_counts ++;

        // to do 把 client_fd添加到任意IO线程里面

        INFOLOG("TcpServer success get client, fd =%d",client_fd);
     }
}