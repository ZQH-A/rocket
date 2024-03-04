#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include <sstream>
#include <sys/time.h>
#include <stdio.h>





namespace rocket{

    //单例模式
    static Logger * g_logger = nullptr;

    Logger* Logger::GetGloballLogger()
    {
            return g_logger;
    }

    void Logger::setGloballLogger()
    {
        //读取出来的配置 转维 LogLevel行，并赋值给Logger
        std::string global_log_level = Config::GetGlobalConfig()->get_level();
        printf("Init log level [%s]\n", global_log_level.c_str());
        LogLevel global_level = StringtoLogLevel(global_log_level);
        g_logger = new Logger(global_level);
    }

    //将LogLevel转换为 String
    std::string LogLeveltoString(LogLevel level)
    {
        switch (level)
        {
        case Debug:
            /* code */
            return "DEBUG";
        case Info:
            return "INFO";
        case Error:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

    //将String 转换为LogLevel
    LogLevel StringtoLogLevel(std::string &log_level)
    {
        if(log_level == "DEBUG")
        {
            return Debug;
        }else if(log_level == "INFO")
        {
            return Info;
        }else if(log_level == "ERROR")
        {
            return Error;
        }else{
            return Unknown;
        }
    }

    std::string LogEvent::toString()
    {
        //获得当前打印到日志的日期和时间
        struct timeval now_time;

        gettimeofday(&now_time,nullptr);

        struct tm now_time_t;

        localtime_r(&(now_time.tv_sec), &now_time_t);

        char buf[128];
        //以年月日 时分秒 格式将时间写入
        strftime(&buf[0],128,"%y-%m-%d %H:%M:%S", &now_time_t);

        std::string time_str(buf);
        //获取当前的毫秒数    
        int ms = now_time.tv_usec / 1000;
        time_str = time_str + "." +std::to_string(ms);

        //获取进程号 线程号
        m_pid = getPid();
        m_thread_id = getThreadId();

        std::stringstream ss;

        ss<< "[" << LogLeveltoString(m_level) << "]\t"
        << "[" << time_str << "]\t"
        << "[" << m_pid << ":" << m_thread_id << "]\t" ;

        return ss.str();
    }

    //
    void Logger::pushLog(const std::string& msg)
    {
        ScopeMutext<Mutex> lock(m_mutex);
        m_buffer.push(msg);
        lock.unlock();
    }

    void Logger::log()
    {
        ScopeMutext<Mutex> lock(m_mutex);
        std::queue<std::string> temp;
        m_buffer.swap(temp);
        lock.unlock();

        while(!temp.empty())
        {
            std::string msg = temp.front();
            temp.pop();
            printf(msg.c_str()); //打印到控制台
        }
    }

}