#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <string>
#include <queue>
#include <memory>
#include <vector>
#include "rocket/common/config.h"
#include "rocket/common/mutex.h"
#include <semaphore.h>
#include "rocket/net/timer_event.h"

namespace rocket{

//利用c++模板将将多个C风格字符串转为String     后续使用这个模板输出日志到文件中
template<typename... Args>
std::string  formatString(const char* str, Args&&... args) //第一个参数是格式 即 "%s\n" 这种，后面是要格式化的字符串
{
    int size = snprintf(nullptr,0,str,args...);  //可以测 可变参数中 字符串的长度
    std::string result;

    if(size > 0)
    {
        result.resize(size);
        snprintf(&result[0],size+1,str,args...); //将可变参数中的字符串 按照 str格式 存储在 result中，并返回
    }
    return result;
}


#define DEBUGLOG(str, ...) \
    if (rocket::Logger::GetGloballLogger()->getLoglevel() <= rocket::Debug) \
    { \
        rocket::Logger::GetGloballLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Debug).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t"+ rocket::formatString(str, ##__VA_ARGS__)+"\n"); \
    } \

#define INFOLOG(str, ...) \
    if (rocket::Logger::GetGloballLogger()->getLoglevel() <= rocket::Info) \
    { \
        rocket::Logger::GetGloballLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Info).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__)+"\n"); \
    } \

#define ERRORLOG(str, ...) \
    if (rocket::Logger::GetGloballLogger()->getLoglevel() <= rocket::Error) \
    { \
        rocket::Logger::GetGloballLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Error).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__)+"\n"); \
    } \

#define APPDEBUGLOG(str, ...) \
    if (rocket::Logger::GetGloballLogger()->getLoglevel() <= rocket::Debug) \
    { \
        rocket::Logger::GetGloballLogger()->pushAppLog(rocket::LogEvent(rocket::LogLevel::Debug).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t"+ rocket::formatString(str, ##__VA_ARGS__)+"\n"); \
    } \

#define APPINFOLOG(str, ...) \
    if (rocket::Logger::GetGloballLogger()->getLoglevel() <= rocket::Info) \
    { \
        rocket::Logger::GetGloballLogger()->pushAppLog(rocket::LogEvent(rocket::LogLevel::Info).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__)+"\n"); \
    } \

#define APPERRORLOG(str, ...) \
    if (rocket::Logger::GetGloballLogger()->getLoglevel() <= rocket::Error) \
    { \
        rocket::Logger::GetGloballLogger()->pushAppLog(rocket::LogEvent(rocket::LogLevel::Error).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__)+"\n"); \
    } \

    //日志级别
 enum LogLevel {
    Unknown = 0,
    Debug = 1,
    Info = 2,
    Error = 3
 };

//加一个定时任务，每隔一段时间，就会将Logger里面的buffer与AsyncLogger的buffer进行交换
//交换后Logger里的buffer就是空的，AsyncLogger就是满的，然后就用一个异步线程将AsyncLogger
//里的东西打印到文件    生产者{Logger} 消费者模式 {AsyncLogger} 
class AsyncLogger{  //异步日志类

public:

    typedef std::shared_ptr<AsyncLogger> s_ptr;

    AsyncLogger(const std::string& file_name, const std::string& file_path, int max_size);

    void stop(); //退出循环的函数

    void flush(); //刷新磁盘

    void pushLogBuffer(std::vector<std::string> &vec);
public:
    static void* Loop(void* arg);

private:
    //为什么要用vector嵌套在queue?  因为：为了防止异步日志在打印时，生产者Logger会向buffer里面添加日志
    //所以每次就添加一整个vector在里面，而打印queue中的一个vector
    std::queue<std::vector<std::string>> m_buffer;

    //最终的日志输出路径与文件名是在 m_file_path/m_file_name_yyyymmdd.1
    std::string m_file_name; // 日志输出文件名
    std::string m_file_path; //日志输出路径

    int m_max_file_size {0}; //单个日志文件最大大小 单位为字节

    sem_t m_sempahore; //信号量 用来唤醒线程

    pthread_t m_thread;

    pthread_cond_t m_condtion; //条件变量
    Mutex m_mutex;

    std::string m_date; //当前打印日志的文件日期
    FILE* m_file_hanlder {NULL}; //当前打开的日志文件句柄

    bool m_reopen_flag {false}; //是否要重新打开新的日志文件
    //1.日志文件已经达到了当前日志文件的最大大小 2.过了晚上12点，需要换一个日志文件

    int m_no {0}; //日志文件序号

    bool m_stop_flag {false}; //退出标志位
};


class Logger {
    public:
        typedef std::shared_ptr<Logger> s_ptr;

        Logger(LogLevel level, int type = 1);

        LogLevel getLoglevel() const 
        {
            return m_set_level;
        }
        void pushLog(const std::string& msg);

        void pushAppLog(const std::string& msg);

        //获取全局 Logger对象
        static Logger* GetGloballLogger();
        //设置 全局 Logger对象
        static void setGloballLogger(int type = 1);

        void log();

        void init();

        void syncLoop();

    private:
        LogLevel m_set_level;
        std::vector<std::string> m_buffer;

        std::vector<std::string> m_app_buffer;  //应用层的输出

        Mutex m_mutex;

        Mutex m_app_mutex;

        AsyncLogger::s_ptr m_asnyc_logger;

        AsyncLogger::s_ptr m_asnyc_app_logger;

        TimerEvent::s_ptr m_timer_event; //定时器

        int m_type {0}; //区分是否使用异步日志  客户端不使用异步日志
};



//将LogLevel 转换为String类型
std::string LogLeveltoString(LogLevel level);
//将String转维 LogLevel
LogLevel StringtoLogLevel(std::string &log_level);


class LogEvent{
    public:

        LogEvent(LogLevel level):m_level(level){}

        //返回文件名
        std::string getFileName() const{
            return m_file_name;
        }
        //返回日志级别
        LogLevel getLogLevel() const{
            return m_level;
        }

        std::string toString();
    private:
        std::string m_file_name; //文件名
        int32_t m_file_line; //行号
        int32_t m_pid; //进程号
        int32_t m_thread_id; //线程号
        LogLevel m_level; //日志级别
};


}


#endif