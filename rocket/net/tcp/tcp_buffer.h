#ifndef ROCKET_NET_TCP_TCP_BUFFER_H
#define ROCKET_NET_TCP_TCP_BUFFER_H

#include <vector>
#include <memory>

namespace rocket{

    class TcpBuffer
    {
    private:
        int m_read_index {0}; //读索引
        int m_write_index {0}; //写索引
        int m_size {0}; //容量大小
    public:
        std::vector<char> m_buffer; //
    public:

        typedef std::shared_ptr<TcpBuffer> s_ptr; //智能指针

        TcpBuffer(int size);
        ~TcpBuffer();

        int readAble(); //返回可读字节数

        int writeAble(); //返回可写字节数

        int readIndex();

        int writeIndex();

        void writeToBuffer(const char* buf, int size);

        void readFromBuffer(std::vector<char>& re, int size);

        void resizeBuffer(int new_size);

        void adjustBuffer();

        void moveReadIndex(int size); //调整 读索引

        void moveWriteIndex(int size); //调整 写索引
    };
    
    
    
}

#endif