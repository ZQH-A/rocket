#ifndef ROCKET_NET_RPC_RPC_CHANNEL_H
#define ROCKET_NET_RPC_RPC_CHANNEL_H

#include <google/protobuf/service.h>
#include "rocket/net/tcp/net_addr.h"

namespace rocket{
    class RpcChannel : public google::protobuf::RpcChannel
    {
    private:
        /* data */
        NetAddr::s_ptr m_peer_addr {nullptr};
        NetAddr::s_ptr m_local_addr {nullptr};

    public:
        //调用远程的Rpc方法
        void CallMethod(const google::protobuf::MethodDescriptor* method,
                                google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                                google::protobuf::Message* response, Closure* done);

        RpcChannel(NetAddr::s_ptr peer_addr);
        ~RpcChannel();
    };

    
}



#endif