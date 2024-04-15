#include "rocket/net/tcp/tcp_buffer.h"
#include <memory>
#include <string.h>
#include "rocket/common/log.h"


namespace rocket{


    TcpBuffer::TcpBuffer(int size): m_size(size)
    {
        m_buffer.resize(m_size);
    }
    TcpBuffer::~TcpBuffer()
    {

    }

    int TcpBuffer::readAble() //返回可读字节数
    {
        return writeIndex() - readIndex();
    }

    int TcpBuffer::writeAble() //返回可写字节数
    {
        return m_size - writeIndex();
    }

    int TcpBuffer::readIndex()
    {
        return m_read_index;
    }

    int TcpBuffer::writeIndex()
    {
        return m_write_index;
    }

    void TcpBuffer::writeToBuffer(const char* buf, int size) //写数据到buffer里
    {
        if(size > writeAble()) //要写的数据比 能写的空间大， 进行扩容
        {
            int new_size = (int)(1.5*(m_write_index + size));
            resizeBuffer(new_size);
        }

        memcpy(&m_buffer[m_write_index],buf,size);

        m_write_index += size;
    }

    void TcpBuffer::readFromBuffer(std::vector<char>& re, int size) //读取buffer中的数据
    {
        if(readAble() == 0)
        {
            return;

        }

        int read_size = readAble() > size ? size : readAble();

        std::vector<char> tmp(read_size);

        memcpy(&tmp[0],&m_buffer[m_read_index],read_size);

        re.swap(tmp);
        m_read_index += read_size;

        adjustBuffer();
    }

    void TcpBuffer::resizeBuffer(int new_size) //扩容
    {
        std::vector<char> tmp(new_size);
        int count = std::min(new_size,readAble());

        memcpy(&tmp[0],&m_buffer[m_read_index],count);
        m_buffer.swap(tmp);

        m_read_index = 0;
        m_write_index = m_read_index + count;
    }

    void TcpBuffer::adjustBuffer() 
    { //因为buffer是个数组，当读完了前面的数据时，前面的数据就没有用了，而只移动了指针
       //导致空间的浪费，所以需要将后面可读的数据往前移动

       if(m_read_index < int(m_buffer.size()/3))  //当可读索引的位置 小于总空间大小的三分之一时，就不用调整
       {
            return;
       } 

       std::vector<char> buffer(m_buffer.size());
       int count = readAble();

       memcpy(&buffer[0],&m_buffer[m_read_index],count);

       m_buffer.swap(buffer);
       m_read_index = 0;
       m_write_index = m_read_index + count;

       buffer.clear();
    }

    void TcpBuffer::moveReadIndex(int size) //调整 读索引
    {
        size_t j  = m_read_index + size;
        if(j >= m_buffer.size())
        {
            ERRORLOG("moveReadIndex error, invalid size %d, old_read_index %d, buffer size %d",size, m_read_index, m_buffer.size());
            return;
        }

        m_read_index = j;
        adjustBuffer();
    }

    void TcpBuffer::moveWriteIndex(int size) //调整 写索引
    {
        size_t j  = m_write_index + size;
        if(j >= m_buffer.size())
        {
            ERRORLOG("moveWriteIndex error, invalid size %d, old_read_index %d, buffer size %d",size, m_write_index, m_buffer.size());
            return;
        }

        m_write_index = j;
        adjustBuffer();
    }
}