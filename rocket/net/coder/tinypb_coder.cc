#include "rocket/net/coder/tinypb_coder.h"
#include <vector>
#include "rocket/common/util.h"
#include "rocket/common/log.h"
#include "rocket/net/coder/tinypb_protocol.h"
#include <string.h>
#include <arpa/inet.h>


namespace rocket{

    //将messages 对象转化为字节流，写入到buffer
    void TinyPBCoder::encode(std::vector<AbstractProtocol::s_ptr>& messages,TcpBuffer::s_ptr out_buffer)
    {
        for(auto &i : messages)
        {
            std::shared_ptr<TinyPBProtocol> msg = std::dynamic_pointer_cast<TinyPBProtocol>(i);
            int len = 0;
            const char* buf = encodeTinyPB(msg,len);

            if(buf !=NULL && len !=0)
            {
                out_buffer->writeToBuffer(buf,len);
            }

            if(buf)
            {
                free((void*)buf);
                buf = NULL;
            }
        }
    }

        //将buffer里面的字节流转化为 message对象
    void TinyPBCoder::decode(std::vector<AbstractProtocol::s_ptr>& messages,TcpBuffer::s_ptr in_buffer)
    { //遍历buffer 找到PB_START，找到之后，解析出整包的长度，然后得到结束符的位置，判断是否为PB_END， 
      //如果不为PB_END说明整个包没有在buffer里面


        while(1){
            std::vector<char> tmp = in_buffer->m_buffer;
            int start_index = in_buffer->readIndex();
            int end_index = -1;

            int pk_len = 0; //整包的长度
            
            bool parse_success = false; //判断是否解析成功
            int i;
            for(i = start_index; i<in_buffer->writeIndex(); ++i)
            {
                if(tmp[i] == TinyPBProtocol::PB_START) //检测到开始符
                {

                    if(i+1 < in_buffer->writeIndex())
                    {                    
                        pk_len = getInt32FromNetByte(&tmp[i+1]); //感觉就只用了一个char字符
                        DEBUGLOG("get pk_len = %d", pk_len);

                        //结束符索引
                        int j= i+pk_len -1;
                        if(j >= in_buffer->writeIndex()) //说明整个包没有读完
                        {
                            continue;
                        }
                        if(tmp[j] == TinyPBProtocol::PB_END)
                        {
                            start_index = i;
                            end_index = j;
                            parse_success = true;
                            break;
                        }
                    }
                }
            }

            if(i >= in_buffer->writeIndex()){
                DEBUGLOG("decode end, read all buffer data");
                return;
            }

            if(parse_success)
            {
                in_buffer->moveReadIndex(end_index-start_index+1);
                std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();

                message->m_pb_len = pk_len;

                int req_id_len_index = start_index + sizeof(char)+ sizeof(message->m_pb_len);
                if(req_id_len_index >= end_index)
                {
                    message->parse_success = false;
                    ERRORLOG("parse error, req_id_len_index[%d] >= end_index[%d]",req_id_len_index,end_index);
                    continue;
                }

                message->m_req_id_len = getInt32FromNetByte(&tmp[req_id_len_index]);
                DEBUGLOG("parse req_id_len=%d",message->m_req_id_len);

                int req_id_index = req_id_len_index + sizeof(message->m_req_id_len);

                char req_id[100] = {0};
                memcpy(&req_id[0], &tmp[req_id_index], message->m_req_id_len);
                message->m_req_id = std::string(req_id);
                DEBUGLOG("parse req_id =%s",message->m_req_id.c_str());

                int method_name_len_index = req_id_index + message->m_req_id_len;
                if(method_name_len_index >= end_index)
                {
                    message->parse_success = false;
                    ERRORLOG("parse error, method_name_len_index[%d] >= end_index[%d]",method_name_len_index,end_index);
                    continue;
                }
                message->m_method_name_len = getInt32FromNetByte(&tmp[method_name_len_index]);
                DEBUGLOG("parse method_name_len=%d",message->m_method_name_len);

                int method_name_index = method_name_len_index + sizeof(message->m_method_name_len);
                char method_name[512] = {0};
                memcpy(&method_name[0],&tmp[method_name_index],message->m_method_name_len);
                message->m_method_name = std::string(method_name);
                DEBUGLOG("parse req_id =%s",message->m_method_name.c_str());

                int err_code_index = method_name_index + message->m_method_name_len;
                if(err_code_index >= end_index)
                {
                    message->parse_success = false;
                    ERRORLOG("parse error, err_code_index[%d] >= end_index[%d]",err_code_index,end_index);
                    continue;
                }
                message->m_err_code = getInt32FromNetByte(&tmp[err_code_index]);


                int err_code_info_len_index = err_code_index + sizeof(message->m_err_code);
                if(err_code_info_len_index >= end_index)
                {
                    message->parse_success = false;
                    ERRORLOG("parse error, err_code_info_len_index[%d] >= end_index[%d]",err_code_info_len_index,end_index);
                    continue;
                }
                message->m_err_info_len = getInt32FromNetByte(&tmp[err_code_info_len_index]);

                int err_info_index = err_code_info_len_index + sizeof(message->m_err_info_len);
                char err_info[512] = {0};
                memcpy(&err_info[0],&tmp[err_info_index],message->m_err_info_len);
                message->m_err_info = std::string(err_info);
                DEBUGLOG("parse req_id =%s",message->m_err_info.c_str());

                int pb_data_len = message->m_pb_len - message->m_method_name_len - message->m_req_id_len - message->m_err_info_len -2 -24;
                int pb_data_index = err_info_index + message->m_err_info_len;
                message->m_pb_data = std::string(&tmp[pb_data_index],pb_data_len);

                //这里校验和去解析
                message->parse_success = true;

                messages.push_back(message);
            }
        }
    }

    //将一个TinyPBProtocol对象转化成字符串数组
    const char* TinyPBCoder::encodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int& len)
    {
        if(message->m_req_id.empty())
        {
            message->m_req_id = "123456789";
        }

        DEBUGLOG("req_id = %s",message->m_req_id.c_str());
        int pb_len = 2 + 24 + message->m_req_id.length() + message->m_method_name.length() + message->m_pb_data.length();
        DEBUGLOG("pb_len = %d",pb_len);

        char* buf = reinterpret_cast<char*> (malloc(pb_len));
        char* tmp = buf;

        *tmp = TinyPBProtocol::PB_START;
        tmp++;

        int32_t pb_len_net = htonl(pb_len);
        memcpy(tmp,&pb_len_net,sizeof(pb_len_net));
        tmp += sizeof(pb_len_net);

        int req_id_len = message->m_req_id.length();
        int32_t req_id_len_net = htonl(req_id_len);
        memcpy(tmp,&req_id_len_net,sizeof(req_id_len_net));
        tmp += sizeof(req_id_len_net);

        if(!message->m_req_id.empty())
        {
            memcpy(tmp,&(message->m_req_id[0]),req_id_len);
            tmp += req_id_len;
        }

        int method_name_len = message->m_method_name.length();
        int32_t method_name_len_net = htonl(method_name_len);
        memcpy(tmp,&method_name_len_net,sizeof(method_name_len_net));
        tmp += sizeof(method_name_len_net);

        if(!message->m_method_name.empty())
        {
            memcpy(tmp,&(message->m_method_name[0]),method_name_len);
            tmp += method_name_len;
        }


        int err_code_len = htonl(message->m_err_code);
        memcpy(tmp,&err_code_len,sizeof(err_code_len));
        tmp += sizeof(err_code_len);

        int err_info_len = message->m_err_info.length();
        int32_t err_info_len_net = htonl(err_info_len);
        memcpy(tmp,&err_info_len_net,sizeof(err_info_len_net));
        tmp += sizeof(err_info_len_net);

        if(!message->m_err_info.empty())
        {
            memcpy(tmp,&(message->m_err_info[0]),err_info_len);
            tmp += err_info_len;
        }

        if(!message->m_pb_data.empty())
        {
            memcpy(tmp,&(message->m_pb_data[0]),message->m_pb_data.length());
            tmp += message->m_pb_data.length();
        }

        int32_t check_sum_net = htonl(1);
        memcpy(tmp,&check_sum_net,sizeof(check_sum_net));
        tmp += sizeof(check_sum_net);

        *tmp = TinyPBProtocol::PB_END;

        message->m_pb_len = pb_len;
        message->m_req_id_len = req_id_len;
        message->m_method_name_len = method_name_len;
        message->m_err_info_len = err_info_len;
        message->parse_success = true;

        len = pb_len;
        
        DEBUGLOG("encode messag[%s],method_name_len [%d]",message->m_req_id.c_str(),message->m_method_name_len);
        return buf;
    }
}