#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_server.h"
#include <memory>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "rocket/net/coder/string_coder.h"
#include "rocket/net/coder/abstract_protocol.h"
#include "rocket/net/coder/tinypb_coder.h"
#include "rocket/net/coder/tinypb_protocol.h"
#include "order.pb.h"
#include <google/protobuf/service.h>
#include "rocket/net/rpc/rpc_dispatcher.h"



class OrderImpl : public Order
{

public:
    void makeOrder(google::protobuf::RpcController* controller,
                       const ::makeOrderRequest* request,
                       ::makeOrderResponse* response,
                       ::google::protobuf::Closure* done)
                       {
                            DEBUGLOG("start sleeo 5s");
                            sleep(5);
                            DEBUGLOG("end sleeo 5s");
                            
                            if(request->price() < 10)
                            {
                                response->set_ret_code(-1);
                                response->set_res_info("short balance");
                                return;
                            }
                            response->set_order_id("20240430");
                       }

};

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

    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    rocket::RpcDispatcher::GetRpcDispatcher()->registerService(service);
    test_tcp_server();

    return 0;
}