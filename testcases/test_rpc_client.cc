#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_server.h"
#include <memory>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/net/coder/string_coder.h"
#include "rocket/net/coder/abstract_protocol.h"
#include "rocket/net/coder/tinypb_coder.h"
#include "rocket/net/coder/tinypb_protocol.h"
#include "order.pb.h"

void test_tcp_client()
{
    rocket::IPNetAddr::s_ptr addr= std::make_shared<rocket::IPNetAddr>("127.0.0.1",12345); //服务端的地址
    rocket::TcpClient client(addr); 
    client.connect([addr,&client]()
    {
        DEBUGLOG("connect success, addr = %s",addr->toString().c_str());
        std::shared_ptr<rocket::TinyPBProtocol> message = std::make_shared<rocket::TinyPBProtocol>();

        //message->m_pb_data = "test pb data";
        message->m_msg_id= "123456789";
        
        makeOrderRequest request;
        request.set_price(100);
        request.set_goods("apple");
        if(!request.SerializeToString(&(message->m_pb_data)))
        {
            ERRORLOG("serilize error");
            return;
        }
        message->m_method_name = "Order.makeOrder";

        client.writeMessage(message,[request](rocket::AbstractProtocol::s_ptr msg_prt){ //有对可写事件的监听 有关闭
            DEBUGLOG("send message success,request[%s]",request.ShortDebugString().c_str());
        });

        client.readMessage("123456789",[](rocket::AbstractProtocol::s_ptr msg_prt){ //有对读事件的监听 无关闭
            std::shared_ptr<rocket::TinyPBProtocol> message = std::dynamic_pointer_cast<rocket::TinyPBProtocol>( msg_prt);

            DEBUGLOG("msg_id [%s],get pb data [%s]",message->m_msg_id.c_str(),message->m_pb_data.c_str());
            
            makeOrderResponse response;
            if(!response.ParseFromString(message->m_pb_data))
            {
                ERRORLOG("deserialize error");
                return;
            }

            DEBUGLOG("get response success,response[%s]",response.ShortDebugString().c_str());

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