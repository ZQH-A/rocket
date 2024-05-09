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
#include "rocket/net/rpc/rpc_channel.h"
#include "rocket/net/rpc/rpc_controller.h"
#include "rocket/net/rpc/rpc_closure.h"

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

        //将TinyPBProtocol协议结构体编码为字节流，并写入到buffer中，并绑定对套接字对可写事件的监听
        client.writeMessage(message,[request](rocket::AbstractProtocol::s_ptr msg_prt){ //有对可写事件的监听 有关闭
            DEBUGLOG("send message success,request[%s]",request.ShortDebugString().c_str());
        });
        //先是绑定对读事件的监听，然后解码buffer中全部的TinyPBProtocol协议结构体，并找到msg_id为123456789的结构体，然后在回调函数中对其中的m_pb_data字段进行反序列化进行解析
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

void test_rpc_channel()
{
    //rocket::IPNetAddr::s_ptr addr= std::make_shared<rocket::IPNetAddr>("127.0.0.1",12345); //服务端的地址
    //std::shared_ptr<rocket::RpcChannel> channel = std::make_shared<rocket::RpcChannel>(addr);
    NEWRPCCHANNEL("127.0.0.1:12345",channel);
    //std::shared_ptr<makeOrderRequest> request = std::make_shared<makeOrderRequest>();

    NEWMESSAGE(makeOrderRequest,request);
    NEWMESSAGE(makeOrderResponse,response);
    

    request->set_price(100);
    request->set_goods("apple");

    //std::shared_ptr<makeOrderResponse> response = std::make_shared<makeOrderResponse>();

    //std::shared_ptr<rocket::RpcController> controller = std::make_shared<rocket::RpcController>();
    NEWRPCCONTROLLER(controller);
    controller->SetMsgId("99998888");
    controller->SetTimeout(10000);

    //回调函数
    std::shared_ptr<rocket::RpcClosure> closure = std::make_shared<rocket::RpcClosure>([request,response,channel,controller]() mutable{
        //判断rpc是否执行成功
        if(controller->GetErrorCode() == 0)
        {
            INFOLOG("call rpc success [%s], response [%s]",request->ShortDebugString().c_str(),response->ShortDebugString().c_str());
            //执行业务逻辑
            if(response->order_id() == "XX")
            {

            }
        }else{
            INFOLOG("call rpc failed,request [%s], error code [%d], error info [%s]",request->ShortDebugString().c_str(),
            controller->GetErrorCode(), controller->GetErrorInfo().c_str());
        }
        
        INFOLOG("now exit eventloop");
        //channel->getTcpClient()->stop();
        channel.reset();
    });

    

    //channel->Init(controller,request,response,closure);

    //Order_Stub stub(channel.get());

    //stub.makeOrder(controller.get(),request.get(),response.get(),closure.get());  //stub.makeOrder()方法会调用到channel的callmethod()方法
    CALLRPC("127.0.0.1:12345",makeOrder,controller,request,response,closure);
}

int main()
{

    //初始化读取配置的类
    rocket::Config::setGlobalConfig(NULL);
    //初始化Logger类
    rocket::Logger::setGloballLogger(0);

    test_rpc_channel();
}