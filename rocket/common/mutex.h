#ifndef ROCKET_COMMON_MUTEX_H
#define ROCKET_COMMON_MUTEX_H


#include <pthread.h>
//封装锁
//用于防止多线程同时访问一个变量导致出现问题
namespace rocket{

    template <class T>
    class ScopeMutext {

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

    class Mutex{
        public:
        Mutex(){
            pthread_mutex_init(&m_mutex,NULL);
        }

        ~Mutex()
        {
            pthread_mutex_destroy(&m_mutex);
        }

        void lock()
        {
            pthread_mutex_lock(&m_mutex);
        }

        void unlock(){
            pthread_mutex_unlock(&m_mutex);
        }
        private:
        pthread_mutex_t m_mutex;
    };
}

#endif