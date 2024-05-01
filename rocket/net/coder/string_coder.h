#ifndef ROCKET_NET_STRING_CODER_H
#define ROCKET_NET_STRING_CODER_H



#include "rocket/net/coder/abstract_coder.h"
#include "rocket/net/coder/abstract_protocol.h"


namespace rocket{



    class StringProtocol : public AbstractProtocol{
        public:
        std::string info;
    };

    class StringCoder : public AbstractCoder
    {
    private:
        /* data */
    public:

        //将message对象转化为字节流，写入到buffer
        void encode(std::vector<AbstractProtocol::s_ptr>& messages,TcpBuffer::s_ptr out_buffer)
        {
            for(size_t i=0;i< messages.size();++i)
            {
                std::shared_ptr<StringProtocol> msg = std::dynamic_pointer_cast<StringProtocol>(messages[i]);

                out_buffer->writeToBuffer(msg->info.c_str(),msg->info.length());
            }
        }

        //将buffer里面的字节流转化为 message对象
        void decode(std::vector<AbstractProtocol::s_ptr>& messages,TcpBuffer::s_ptr in_buffer)
        {
           
            std::vector<char> re;
            in_buffer->readFromBuffer(re,in_buffer->readAble());
            std::string info;

            for(size_t i=0;i<re.size();++i)
            {
                info +=re[i];
            }


            std::shared_ptr<StringProtocol> msg = std::make_shared<StringProtocol>();
            msg->info = info;
            msg->m_msg_id = "12345";
            messages.push_back(msg);
        }
    };
    
}

#endif