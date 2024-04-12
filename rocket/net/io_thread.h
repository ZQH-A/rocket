#ifndef ROCKET_NET_IO_THREAD_H
#define ROCKET_NET_IO_THREAD_H

#include "rocket/net/eventloop.h"
#include <pthread.h>
#include <semaphore.h>

namespace rocket{

    class IOThread{
        public:

        IOThread();

        ~IOThread();

        EventLoop* getEventLoop();

        void join();

        void start(); //主动唤醒新线程并执行循环
        public:

        static void* Main(void *arg); //线程执行的函数
        private:

        pid_t m_thread_id {-1}; //线程号
        pthread_t m_thread; //线程句柄
        EventLoop * m_event_loop {NULL}; //当前io线程的loop对象
        sem_t m_init_semaphore; //信号量
        sem_t m_start_semaphore; //启动信号量
    };
}

#endif