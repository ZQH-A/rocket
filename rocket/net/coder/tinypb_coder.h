#ifndef ROCKET_NET_CODER_TINYPB_CODER_H
#define ROCKET_NET_CODER_TINYPB_CODER_H


#include "rocket/net/coder/abstract_coder.h"
#include "rocket/net/coder/tinypb_protocol.h"

namespace rocket
{
    
    class TinyPBCoder : public AbstractCoder
    {
    private:
        /* data */
        const char* encodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int& len);
    public:
                //将message对象转化为字节流，写入到buffer
        void encode(std::vector<AbstractProtocol::s_ptr>& messages,TcpBuffer::s_ptr out_buffer);

        //将buffer里面的字节流转化为 message对象
        void decode(std::vector<AbstractProtocol::s_ptr>& messages,TcpBuffer::s_ptr in_buffer);

        ~TinyPBCoder() {}
        TinyPBCoder() {}
    };
    
} // namespace rocket



#endif