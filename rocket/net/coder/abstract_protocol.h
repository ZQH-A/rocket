#ifndef ROCKET_NET_ABSTRACT_PROTOCOL_H
#define ROCKET_NET_ABSTRACT_PROTOCOL_H

#include <memory>


namespace rocket{
    struct AbstractProtocol :public std::enable_shared_from_this<AbstractProtocol>
    {
    private:
        /* data */

    public:
        typedef std::shared_ptr<AbstractProtocol> s_ptr;

        // std::string getMsgId(){
        //     return m_msg_id;
        // }

        // void setMsgId(const std::string msg_id)
        // {
        //     m_msg_id = msg_id;
        // }

        //虚析构函数
        virtual ~AbstractProtocol(){}
        AbstractProtocol(){}
    public:
        std::string m_msg_id; //请求号，唯一表示一个请求或者响应
    };
        
}


#endif