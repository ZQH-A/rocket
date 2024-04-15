#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"


int main()
{

    //初始化读取配置的类
    rocket::Config::setGlobalConfig("../conf/rocket.xml");
    //初始化Logger类
    rocket::Logger::setGloballLogger();

    rocket::IPNetAddr addr("127.0.0.1",12345);

    DEBUGLOG("create addr %s",addr.toString().c_str());
}