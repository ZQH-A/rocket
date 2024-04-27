#ifndef ROCKET_NET_CODER_TINYPB_PROTOCOL_H
#define ROCKET_NET_CODER_TINYPB_PROTOCOL_H

#include "rocket/net/coder/abstract_protocol.h"
#include <string>

namespace rocket{
    struct TinyPBProtocol : public AbstractProtocol
    {
    public:
        TinyPBProtocol() {}
        ~TinyPBProtocol(){}
    public:
        /* data */
        int32_t m_pb_len {0}; //整包长度
        int32_t m_req_id_len {0};// req_id 继承于父类 req_id的长度

        int32_t m_method_name_len{0};  //方法名长度
        std::string m_method_name;  //方法名
        int32_t m_err_code {0};  //错误码
        int32_t m_err_info_len {0};  //错误信息长度
        std::string m_err_info;  //错误信息
        std::string m_pb_data;  //protobuf序列化数据
        int32_t m_check_sum {0};//校验码

        bool parse_success {false};
    public:
        static char PB_START; //开始符
        static char PB_END; //结束符
    };
    

}

#endif