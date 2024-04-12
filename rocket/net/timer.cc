#include "rocket/net/timer.h"
#include <sys/timerfd.h>
#include "rocket/common/util.h"
#include "rocket/common/log.h"
#include <string.h>
#include "rocket/net/timer_event.h"

namespace rocket{

    Timer::Timer() : FdEvent()
    {
        //使用Linux自带的timerfd 
        m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

        DEBUGLOG("timer fd=%d",m_fd);
        //把fd可读事件放到了eventloop上监听
        listen(FdEvent::IN_EVENT,std::bind(&Timer::onTimer,this));
    }
    
    Timer::~Timer()
    {
        
    }


    void Timer::addTimerEvent(TimerEvent::s_ptr event) //添加TimerEvent事件到任务队列中
    {
         bool is_reset_timerfd = false;

         ScopeMutext<Mutex> lock(m_mutex);

         if(m_pending_events.empty()){  //如果任务队列为空，则一定要重设timerfd的到达时间
            is_reset_timerfd = true;
            
         }else{
            auto it = m_pending_events.begin();
            if(it->second->getArriveTime() > event->getArriveTime())  //如果第一个的任务的达到时间大于新添加的任务到达时间
            { //也要重设timerfd的到达时间
                is_reset_timerfd = true;
            }
         }
         DEBUGLOG("Timer  reset_timerfd %d.......",is_reset_timerfd);
         m_pending_events.emplace(event->getArriveTime(),event); //将event添加到任务队列，会找到到达时间从小到大排序
         lock.unlock();

         if(is_reset_timerfd)
         {
            resetArriveTime();
         }
    }

    void Timer::deleteTimerEvent(TimerEvent::s_ptr event)
    { //删除任务
        event->setIsCancled(true);

        ScopeMutext<Mutex> lock(m_mutex);
        //找到到达时间相同的第一个 和 最后一个
        auto begin = m_pending_events.lower_bound(event->getArriveTime());
        auto end = m_pending_events.upper_bound(event->getArriveTime());

        auto it = begin;
        //遍历 找到相同的后退出
        for(it= begin; it!= end;++it)
        {
            if(it->second == event)
            {
                break;
            }
        }
        //如果没找到则不删除，否则，删除
        if(it!=end)
        {
            m_pending_events.erase(it);
        }

        lock.unlock();

        DEBUGLOG("sucdess delete TimerEvent at arrive time %lld",event->getArriveTime());
    }

    void Timer::onTimer() //当发生了IO事件后，eventloop会执行这个回调函数
    {
        //处理缓冲区事件，防止下一次继续触发可读事件
        char buf[8];
        while(1)
        {
            if((read(m_fd,buf,8)==-1) && errno == EAGAIN)
            {
                break;
            }
        }

        //执行定时任务
        int64_t now = getNowMs();

        std::vector<TimerEvent::s_ptr> tmps; //要执行的timerevent 和 任务队列
        std::vector<std::pair<int64_t,std::function<void()>>> tasks;

        ScopeMutext<Mutex> lock(m_mutex);
        auto it = m_pending_events.begin();

        for(;it != m_pending_events.end();++it)
        {
            if(it->first <= now)  //当前的event已经超时
            {
                if(!it->second->IsCancled())  //并且还是未取消的
                {
                    tmps.push_back(it->second);
                    tasks.push_back(std::make_pair(it->second->getArriveTime(),it->second->getCallback()));
                }
            }else{
                break;
            }
        }

        m_pending_events.erase(m_pending_events.begin(),it);

        lock.unlock();

        //需要把重复的Event再次添加进去  因为当前的event属性是重复的，就是说这次执行了，达到一定的时间下次还要执行
        //将它们删除是为 更新timerfd的arrivetime，因为重复的event的arrivetime已经改变
        for (auto i = tmps.begin(); i!=tmps.end();++i)
        {
            if((*i)->IsRepeated())
            {
                //调整arriveTime
                (*i)->resetArriveTime();
                addTimerEvent(*i);
            }
        }
        //
        resetArriveTime();
        //执行超时任务队列
        for(auto i:tasks){
            if(i.second)
            {
                i.second();
            }
        }
    }

    void Timer::resetArriveTime() //重新设置fd的达到时间
    {   //加锁
        ScopeMutext<Mutex> lock(m_mutex);
        auto tmp = m_pending_events;
        lock.unlock();

        if(tmp.size() == 0)  //如果 m_pending_events中没有任务，直接返回
        {
            return;
        }

        int64_t now = getNowMs(); //获取当前时间

        auto it = tmp.begin();  //任务中是按照时间从小到大排序的， 获取的第一个任务的达到时间最小
        int64_t interval = 0;
        if(it->second->getArriveTime() > now) //如果第一个任务的到达时间大于当前时间
        {
            interval = it->second->getArriveTime() - now;  //则将fd的触发事件设置为第一个任务的到达时间
        }else{
            interval = 100; //如果当前的时间大于第一个任务的到达时间，说明第一个任务已经超时，需要马上执行
        }

        timespec ts;
        memset(&ts,0,sizeof(ts));
        ts.tv_sec = interval / 1000;
        ts.tv_nsec = (interval % 1000) * 1000000;

        itimerspec value;
        memset(&value,0,sizeof(value));
        value.it_value = ts;

        int rt = timerfd_settime(m_fd,0,&value,NULL);

        
        if(rt != 0)
        {
            ERRORLOG("timerfd_settime error,errno=%d, error=%s",errno,strerror(errno));
        }
        DEBUGLOG("timer reset to %lld",now+interval);
    }
}