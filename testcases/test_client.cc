#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_server.h"
#include <memory>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

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

int main()
{

    //初始化读取配置的类
    rocket::Config::setGlobalConfig("../conf/rocket.xml");
    //初始化Logger类
    rocket::Logger::setGloballLogger();

    

    test_connect();
}