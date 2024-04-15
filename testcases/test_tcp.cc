#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_server.h"
#include <memory>

void test_tcp_server()
{
    rocket::IPNetAddr::s_ptr addr= std::make_shared<rocket::IPNetAddr>("127.0.0.1",12345);

    DEBUGLOG("create addr %s",addr->toString().c_str());

    rocket::TcpServer tcp_server(addr);

    

    tcp_server.start();
}

int main()
{

    //初始化读取配置的类
    rocket::Config::setGlobalConfig("../conf/rocket.xml");
    //初始化Logger类
    rocket::Logger::setGloballLogger();

    test_tcp_server();

    // rocket::IPNetAddr addr("127.0.0.1",12345);

    // DEBUGLOG("create addr %s",addr.toString().c_str());
}