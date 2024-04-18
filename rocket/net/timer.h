#ifndef ROCKET_NET_TIMER_H
#define ROCKET_NET_TIMER_H

#include "rocket/net/fd_event.h"
#include <map>
#include "rocket/common/mutex.h"
#include "rocket/net/timer_event.h"


namespace rocket{
    class Timer : public FdEvent
    {
    private:
        /* data */
        std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events; //需要处理的定时任务事件
        Mutex m_mutex;
    
    private:
        void resetArriveTime();
    public:
        Timer();
        ~Timer();

        void addTimerEvent(TimerEvent::s_ptr event);

        void deleteTimerEvent(TimerEvent::s_ptr event);

        void onTimer(); //当发生了IO事件后，eventloop会执行这个回调函数
    };
}


#endif