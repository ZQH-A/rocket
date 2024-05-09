#ifndef ROCKET_COMMON_MUTEX_H
#define ROCKET_COMMON_MUTEX_H


#include <pthread.h>
//封装锁
//用于防止多线程同时访问一个变量导致出现问题
namespace rocket{

    //模板类
    template <class T>
    class ScopeMutext {  //使用ScopeMutext是为了防止在使用Mutex加锁后忘记解锁
    //而使用类，可以在类的析构函数中对Mutex进行解锁。
        public:

        ScopeMutext(T& mutex) : m_mutex(mutex)
        {
            m_mutex.lock();
            m_is_lock = true;
        }

        ~ScopeMutext(){
            m_mutex.unlock();
            m_is_lock = false;
        }

        void lock(){
            if(!m_is_lock)
            {
                 m_mutex.lock();
                 m_is_lock = true;
            }
        }

        void unlock(){
            if(m_is_lock)
            {
                m_mutex.unlock();
                m_is_lock = false;
            }
        }

        private:
    
        T& m_mutex;
        bool m_is_lock {false};
    };

    class Mutex{ //封装了一个互斥锁

        public:

        Mutex(){
            //对互斥锁进行初始化
            pthread_mutex_init(&m_mutex,NULL);
        }

        ~Mutex()
        {
            pthread_mutex_destroy(&m_mutex);
        }

        void lock()
        { //获取锁
            pthread_mutex_lock(&m_mutex);
        }

        void unlock(){
            //释放锁
            pthread_mutex_unlock(&m_mutex);
        }

        pthread_mutex_t* getMutex()
        {
            return &m_mutex;
        }

        private:

        pthread_mutex_t m_mutex;
    };
}

#endif