#include "rocket/common/log.h"
#include <pthread.h>
#include <unistd.h>
#include "rocket/common/config.h"
#include "rocket/net/eventloop.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include "rocket/net/timer_event.h"
#include <memory>
#include "rocket/net/io_thread.h"
#include "rocket/net/io_thread_group.h"


void test_to_thread()
{
    //创建套接字
    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd == -1)
    {
        ERRORLOG("listenfd = -1");
        exit(0);
    }

    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    //设置地址中的端口号
    addr.sin_port = htons(12345);
    addr.sin_family = AF_INET;
    inet_aton("127.0.0.1",&addr.sin_addr);

    int rt = bind(listenfd,reinterpret_cast<sockaddr*> (&addr),sizeof(addr));
    if(rt !=0)
    {
        ERRORLOG("bind error");
        exit(0);
    }

    rt = listen(listenfd,100);
    if(rt != 0)
    {
        ERRORLOG("listen error");
        exit(0);
    }

    rocket::FdEvent event(listenfd);
    event.listen(rocket::FdEvent::IN_EVENT, [listenfd](){ //绑定listenfd的读监听事件
        sockaddr_in peer_addr;
        socklen_t len = sizeof(peer_addr);
        memset(&peer_addr,0,sizeof(peer_addr));
        int clientfd = accept(listenfd,reinterpret_cast<sockaddr*>(&peer_addr),&len);
        DEBUGLOG("success get client fd[%d], peer addr:[%s:%d]",clientfd, inet_ntoa(peer_addr.sin_addr),ntohs(peer_addr.sin_port));
    }); //当有连接到来时，会触发该函数



    //添加TimerEvent事件

    int i= 0;
    rocket::TimerEvent::s_ptr timer_event = std::make_shared<rocket::TimerEvent>(1000,true,[&i]()
    {
        INFOLOG("trigger timer event,count=%d",i++);
    });

    //测试 IOThread
    // rocket::IOThread io_thread;

    // io_thread.getEventLoop()->addEpollEvent(&event);
    // io_thread.getEventLoop()->addTimerEvent(timer_event);
    // io_thread.start();
    // io_thread.join();

    //测试 IOThreadGroup

    rocket::IOThreadGroup io_thread_group(2);

    rocket::IOThread* io_thread = io_thread_group.getIOThread();
    io_thread->getEventLoop()->addEpollEvent(&event);
    io_thread->getEventLoop()->addTimerEvent(timer_event);

    rocket::IOThread* io_thread2 = io_thread_group.getIOThread();
    io_thread2->getEventLoop()->addTimerEvent(timer_event);

    io_thread_group.start();

    io_thread_group.join();

}
int main()
{

    //初始化读取配置的类
    rocket::Config::setGlobalConfig("../conf/rocket.xml");
    //初始化Logger类
    rocket::Logger::setGloballLogger();

    //测试IOthread
    test_to_thread();

    //测试 eventloop 模块
    // rocket::EventLoop* eventloop = new rocket::EventLoop();

    // //创建套接字
    // int listenfd = socket(AF_INET,SOCK_STREAM,0);
    // if (listenfd == -1)
    // {
    //     ERRORLOG("listenfd = -1");
    //     exit(0);
    // }

    // sockaddr_in addr;
    // memset(&addr,0,sizeof(addr));
    // //设置地址中的端口号
    // addr.sin_port = htons(12345);
    // addr.sin_family = AF_INET;
    // inet_aton("127.0.0.1",&addr.sin_addr);

    // int rt = bind(listenfd,reinterpret_cast<sockaddr*> (&addr),sizeof(addr));
    // if(rt !=0)
    // {
    //     ERRORLOG("bind error");
    //     exit(0);
    // }

    // rt = listen(listenfd,100);
    // if(rt != 0)
    // {
    //     ERRORLOG("listen error");
    //     exit(0);
    // }

    // rocket::FdEvent event(listenfd);
    // event.listen(rocket::FdEvent::IN_EVENT, [listenfd](){ //绑定listenfd的读监听事件
    //     sockaddr_in peer_addr;
    //     socklen_t len = sizeof(peer_addr);
    //     memset(&peer_addr,0,sizeof(peer_addr));
    //     int clientfd = accept(listenfd,reinterpret_cast<sockaddr*>(&peer_addr),&len);
    //     DEBUGLOG("success get client fd[%d], peer addr:[%s:%d]",clientfd, inet_ntoa(peer_addr.sin_addr),ntohs(peer_addr.sin_port));
    // }); //当有连接到来时，会触发该函数

    // eventloop->addEpollEvent(&event); //将listenfd上树
    


    // //添加TimerEvent事件

    // int i= 0;
    // rocket::TimerEvent::s_ptr timer_event = std::make_shared<rocket::TimerEvent>(1000,true,[&i]()
    // {
    //     INFOLOG("trigger timer event,count=%d",i++);
    // });
    // eventloop->addTimerEvent(timer_event);

    // eventloop->loop(); //开启loop循环
    return 0;
}