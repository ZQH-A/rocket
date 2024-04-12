#ifndef ROCKET_NET_TIMEREVENT
#define ROCKET_NET_TIMEREVENT

#include <functional>
#include <memory>


namespace rocket{

    class TimerEvent
    {
    private:
        /* data */
        int64_t m_arrive_time; //ms 时间戳
        int64_t m_interval; //间隔
        bool m_is_repeated {false}; //是否重复
        bool m_is_cancled {false}; //是否取消

        std::function<void()> m_task; //任务队列

    public:
        typedef std::shared_ptr<TimerEvent> s_ptr; //指向TimerEvent的智能指针

    public:
        TimerEvent(int interval, bool is_repeated, std::function<void()> cb);
        ~TimerEvent();
        int64_t getArriveTime() const;
        void setIsCancled(bool is_cancled);
        bool IsCancled() const;
        bool IsRepeated() const;
        std::function<void()> getCallback() const;
        void resetArriveTime();
    };
}



#endif