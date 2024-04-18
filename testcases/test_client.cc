#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_server.h"
#include <memory>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/net/string_coder.h"
#include "rocket/net/abstract_protocol.h"

void test_connect()
{
    //调用 connect 连接server
    //write 一个字符串
    //等待read返回结果

    DEBUGLOG("success start connect");
    int fd = socket(AF_INET,SOCK_STREAM,0);

    if(fd < 0)
    {
        ERRORLOG("invalid fd %d",fd);
        exit(0);
    }
    sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_aton("127.0.0.1",&server_addr.sin_addr);

    int rt = connect(fd,reinterpret_cast<sockaddr*>(&server_addr),sizeof(server_addr));

    std::string msg = "hello rocket!";

    rt = write(fd,msg.c_str(),msg.length());

    DEBUGLOG("success write %d bytes, [%s]",rt,msg.c_str());

    char buf[100];
    rt = read(fd,buf, 1000);
    DEBUGLOG("success read %d bytes,[%s]",rt,std::string(buf).c_str());
} 

void test_tcp_client()
{
    rocket::IPNetAddr::s_ptr addr= std::make_shared<rocket::IPNetAddr>("127.0.0.1",12345); //服务端的地址
    rocket::TcpClient client(addr); 
    client.connect([addr,&client]()
    {
        DEBUGLOG("connect success, addr = %s",addr->toString().c_str());
        std::shared_ptr<rocket::StringProtocol> message = std::make_shared<rocket::StringProtocol>();

        message->info = "hello rocket";
        message->setReqId("12345");
        client.writeMessage(message,[](rocket::AbstractProtocol::s_ptr msg_prt){ //有对可写事件的监听 有关闭
            DEBUGLOG("send message success");
        });
        client.readMessage("12345",[](rocket::AbstractProtocol::s_ptr msg_prt){ //有对读事件的监听 无关闭
            std::shared_ptr<rocket::StringProtocol> message = std::dynamic_pointer_cast<rocket::StringProtocol>( msg_prt);

            DEBUGLOG("req_id [%s],get response %s",message->getReqId().c_str(),message->info.c_str());
        });
        client.writeMessage(message,[](rocket::AbstractProtocol::s_ptr msg_prt){
            DEBUGLOG("send message 2222 success");
        });
    });
}

int main()
{

    //初始化读取配置的类
    rocket::Config::setGlobalConfig("../conf/rocket.xml");
    //初始化Logger类
    rocket::Logger::setGloballLogger();

    

    test_tcp_client();
}