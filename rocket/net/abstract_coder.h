#ifndef ROCKET_NET_ABSTRACT_CODER_H
#define ROCKET_NET_ABSTRACT_CODER_H

#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/net/abstract_protocol.h"
#include <vector>

namespace rocket{
    class AbstractCoder
    {
    private:
        /* data */
    public:

        //将message对象转化为字节流，写入到buffer
        virtual void encode(std::vector<AbstractProtocol::s_ptr>& messages,TcpBuffer::s_ptr out_buffer) =0;

        //将buffer里面的字节流转化为 message对象
        virtual void decode(std::vector<AbstractProtocol::s_ptr>& messages,TcpBuffer::s_ptr in_buffer) =0;

        //虚析构函数
        virtual ~AbstractCoder(){}
        AbstractCoder(){}
    };
    
}

#endif