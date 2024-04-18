#ifndef ROCKET_NET_ABSTRACT_PROTOCOL_H
#define ROCKET_NET_ABSTRACT_PROTOCOL_H

#include <memory>


namespace rocket{
    class AbstractProtocol :public std::enable_shared_from_this<AbstractProtocol>
    {
    private:
        /* data */

    public:
        typedef std::shared_ptr<AbstractProtocol> s_ptr;

        std::string getReqId(){
            return m_req_id;
        }

        void setReqId(const std::string req_id)
        {
            m_req_id = req_id;
        }

        //虚析构函数
        virtual ~AbstractProtocol(){}
        AbstractProtocol(){}
    protected:
        std::string m_req_id; //请求号，唯一表示一个请求或者响应
    };
        
}


#endif