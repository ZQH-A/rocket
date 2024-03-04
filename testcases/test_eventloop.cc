#include "rocket/common/log.h"
#include <pthread.h>
#include <unistd.h>
#include "rocket/common/config.h"
#include "rocket/net/eventloop.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>


int main()
{

    //初始化读取配置的类
    rocket::Config::setGlobalConfig("../conf/rocket.xml");
    //初始化Logger类
    rocket::Logger::setGloballLogger();

    rocket::EventLoop* eventloop = new rocket::EventLoop();

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
    event.listen(rocket::FdEvent::IN_EVENT, [listenfd](){
        sockaddr_in peer_addr;
        socklen_t len = sizeof(peer_addr);
        memset(&peer_addr,0,sizeof(peer_addr));
        int clientfd = accept(listenfd,reinterpret_cast<sockaddr*>(&peer_addr),&len);
        DEBUGLOG("success get client fd[%d], peer addr:[%s:%d]",clientfd, inet_ntoa(peer_addr.sin_addr),ntohs(peer_addr.sin_port));
    });

    eventloop->addEpollEvent(&event);
    
    eventloop->loop();
    return 0;
}