#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/fd_event_group.h"
#include <unistd.h>
#include "rocket/common/log.h"
#include "rocket/net/coder/string_coder.h"
#include "rocket/net/coder/tinypb_coder.h"

namespace rocket{

    TcpConnection::TcpConnection(EventLoop* event_loop,int fd,int buffer_size,NetAddr::s_ptr peer_addr, TcpConnectionType type /*= TcpConnectionByServer*/)
    : m_event_loop(event_loop), m_peer_addr(peer_addr), m_state(NotConnection),m_fd(fd),m_connection_type(type)
    {
        m_in_buffer = std::make_shared<TcpBuffer>(buffer_size); //in and out buffer 初始化
        m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

        m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);

        //将fd的读事件绑定到read函数上
        m_fd_event->setNonBlock(); //设置为非阻塞  
        

        m_coder = new TinyPBCoder();
        //只有服务端才在一开始监听可读事件，客户端是在需要的时候监听可读事件
        if(m_connection_type == TcpConnectionByServer)
        {
            //监听可读事件
            listenRead();
            DEBUGLOG("TcpConnection...................");
        }
    }

    TcpConnection::~TcpConnection()
    {
        // 1.
        if(m_coder)
        {
            delete m_coder;
            m_coder = NULL;
        }
    }

    void TcpConnection::clear() //处理关闭事件
    {
        //服务器处理一些关闭连接后的清理动作
        if(m_state == Closed)
        {
            return;
        }
        m_fd_event->cancle(FdEvent::OUT_EVENT);
        m_fd_event->cancle(FdEvent::IN_EVENT);
        m_event_loop->deleteEpollEvent(m_fd_event);
        m_state = Closed;
    }

    void TcpConnection::onRead()
    { // 1.从socket缓冲区，调用系统的read函数读取字节 到in_buffer里面
        if(m_state != Connected)
        {
            ERRORLOG("onRead error,client has already disconnected, addr[%s], clientfd[%d]",m_peer_addr->toString().c_str(),m_fd);
            return;
        }

        bool is_read_all = false; //是否读完标志
        bool is_close = false; //是否关闭
        while (!is_read_all)
        {
            //没有写的空间，扩容
            if (m_in_buffer->writeAble() == 0)
            {
                m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
            }

            int read_count = m_in_buffer->writeAble();
            int write_index = m_in_buffer->writeIndex();

            int rt = read(m_fd,&(m_in_buffer->m_buffer[write_index]),read_count);
            DEBUGLOG("success read %d bytes from addr [%s], client fd[%d]",rt,m_peer_addr->toString().c_str(),m_fd);
            if(rt > 0)
            {
                m_in_buffer->moveWriteIndex(rt);
                
                if(rt == read_count) //读取的字数与 readcount一样，说明还有数据没读取完
                {
                    continue;
                }else if (rt < read_count) //读取了所有的数据
                {
                    is_read_all = true;
                    break;
                }
            } else if(rt == 0){ // <= 0

                is_close = true; //说明关闭了连接
                break;
            }else if (rt == -1 && errno == EAGAIN)
            {
                //说明数据读取完毕
                is_read_all = true;
            }
        }

        if(is_close) //对端关闭，本端关闭
        { //处理关闭连接
            INFOLOG("peer closed, peer addr [%d],clientfd [%d]",m_peer_addr->toString().c_str(),m_fd);
            clear();
        }else{ //对端未关闭
            if(!is_read_all) //数据未读取完
            {
                ERRORLOG("not read all data");
            }

            //to do 简单的echo,后面补充rpc协议解析
            excute();
        }
    }

    void TcpConnection::excute()
    { //将rpc请求执行业务逻辑，获取rpc响应，再把rpc响应发送回去
        
        if(m_connection_type == TcpConnectionByServer)
        {
            // std::vector<char> tmp;
            // int size = m_in_buffer->readAble();
            // tmp.resize(size);
            // m_in_buffer->readFromBuffer(tmp,size);

            // std::string msg;
            // for(size_t i=0;i<tmp.size();++i)
            // {
            //     msg+=tmp[i];
            // }

            // INFOLOG("success get request [%s] from client [%s]",msg.c_str(),m_peer_addr->toString().c_str());
            // m_out_buffer->writeToBuffer(msg.c_str(),msg.length());


            //1.针对每一个请求，调用rpc方法，获取响应的message
            //2.将响应的message放入到发送缓冲区，监听可写事件回包
            std::vector<AbstractProtocol::s_ptr> result;
            std::vector<AbstractProtocol::s_ptr> replay_message;
            m_coder->decode(result,m_in_buffer);
            for(size_t i =0; i<result.size(); ++i)
            {
                INFOLOG("success get request [%s] from client [%s]",result[i]->m_req_id.c_str(),m_peer_addr->toString().c_str());
                std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
                message->m_pb_data = "hello. this is rocket rpc test data";
                message->m_req_id = result[i]->m_req_id;
                replay_message.emplace_back(message);

            }
            m_coder->encode(replay_message,m_out_buffer);
            //绑定写事件
            listenWrite();
        }else{
            //从buffer里decode得到message对象，执行其回调
            std::vector<AbstractProtocol::s_ptr> result;
            m_coder->decode(result,m_in_buffer);

            for(size_t i=0;i<result.size();++i)
            {
                std::string req_id = result[i]->m_req_id;
                auto it = m_read_dones.find(req_id);
                if(it != m_read_dones.end())
                {
                    it->second(result[i]); //调用readMessage中设置的回调函数
                }
            }
        }
    }

    void TcpConnection::onWrite()
    {//1.将当前 out_buffer里面的数据全部发送给client
        // 
        if(m_state != Connected)
        {
            ERRORLOG("onWrite error,client has already disconnected, addr[%s], clientfd[%d]",m_peer_addr->toString().c_str(),m_fd);
            return;
        }

        if(m_connection_type == TcpConnectionByClient) //客户端调用时
        {
            //1. 将message encode得到字节流
            //2.将字节流写入到buffer里面，然后全部发送
            std::vector<AbstractProtocol::s_ptr> messages;

            for(size_t i=0; i< m_write_dones.size();++i)
            {
                messages.push_back(m_write_dones[i].first);
            }
            //写入到out_buffer中
            m_coder->encode(messages,m_out_buffer);
        }

        bool is_write_all = false;
        while(true)
        {
            if(m_out_buffer->readAble() ==0)
            {
                DEBUGLOG("no data need to send to client [%s]",m_peer_addr->toString().c_str());
                is_write_all = true;
                break;
            }

            int write_size = m_out_buffer->readAble();
            int read_index = m_out_buffer->readIndex();

            int rt = write(m_fd,&(m_out_buffer->m_buffer[read_index]),write_size);

            if(rt >= write_size)
            {
                DEBUGLOG("no data need to send to client [%s]",m_peer_addr->toString().c_str());
                m_out_buffer->moveReadIndex(rt); //自己添加的
                is_write_all = true;
                break;
            }else if(rt == -1 && errno == EAGAIN)
            {
                //发送缓冲区已满，不能在发送了
                //这种情况我们等下次fd可写的时候在此发送数据即可
                ERRORLOG("write data error, errno == EAGIN and rt == -1");
                break;
            }
        }

        if(is_write_all) //数据全部写完后
        {
            m_fd_event->cancle(FdEvent::OUT_EVENT);  //取消对可写事件的监听
            m_event_loop->addEpollEvent(m_fd_event);
        }

        if(m_connection_type == TcpConnectionByClient) //客户端调用时
        {
            for(size_t i = 0; i < m_write_dones.size();++i)
            {
                m_write_dones[i].second(m_write_dones[i].first); //调用回调函数
            }
            m_write_dones.clear();
        }
    }

    void TcpConnection::setState(const TcpState state)
    {
        m_state = state;
    }

    TcpState TcpConnection::getState()
    {
        return m_state;
    }

    void TcpConnection::shutdown() //服务器主动关闭连接
    { //当有很多TCP连接时，而有些TCP不进行数据的收发，服务器需要主动处理无效的TCP连接
    //保证系统的流畅运行
        if(m_state == Closed || m_state == NotConnection)
        {
            return;
        }

        //处理半关闭
        m_state = HalfClosing;

        //调用系统的shutdown关闭读写，意味着服务器不会在对这个fd进行读写操作
        //这一步是服务器主动发送FIN报文，触发四次挥手的第一个阶段
        //当fd发送可读事件，但是可读的数据为0，即 对端发送了FIN报文
        //然后会触发本地的onRead函数，但是可读数据为0，进而触发clear函数，
        ::shutdown(m_fd,SHUT_RDWR);
    }

    void TcpConnection::setConnectionType(TcpConnectionType type)
    {
        m_connection_type = type;
    }
    //监听可写事件
    void TcpConnection::listenWrite()
    {
        m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite,this));
        m_event_loop->addEpollEvent(m_fd_event);
    }

    void TcpConnection::listenRead()
    {
        m_fd_event->listen(FdEvent::IN_EVENT,std::bind(&TcpConnection::onRead,this));
        m_event_loop->addEpollEvent(m_fd_event);
    }

    void TcpConnection::pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done)
    {
        m_write_dones.push_back(std::make_pair(message,done));
    }

    void TcpConnection::pushReadMessage(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done)
    {
        m_read_dones.insert(std::make_pair(req_id,done));
    }
}