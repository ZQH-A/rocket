#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <string>
#include <queue>
#include <memory>
#include "rocket/common/config.h"
#include "rocket/common/mutex.h"

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
        rocket::Logger::GetGloballLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Debug))->toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t"+ rocket::formatString(str, ##__VA_ARGS__)+"\n"); \
        rocket::Logger::GetGloballLogger()->log(); \
    } \

#define INFOLOG(str, ...) \
    if (rocket::Logger::GetGloballLogger()->getLoglevel() <= rocket::Info) \
    { \
        rocket::Logger::GetGloballLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Info))->toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__)+"\n"); \
        rocket::Logger::GetGloballLogger()->log(); \
    } \

#define ERRORLOG(str, ...) \
    if (rocket::Logger::GetGloballLogger()->getLoglevel() <= rocket::Error) \
    { \
        rocket::Logger::GetGloballLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Error))->toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__)+"\n"); \
        rocket::Logger::GetGloballLogger()->log(); \
    } \


    //日志级别
 enum LogLevel {
    Unknown = 0,
    Debug = 1,
    Info = 2,
    Error = 3
 };

class Logger {
    public:
        typedef std::shared_ptr<Logger> s_ptr;

        Logger(LogLevel level) : m_set_level(level) {}

        LogLevel getLoglevel() const 
        {
            return m_set_level;
        }
        void pushLog(const std::string& msg);
        //获取全局 Logger对象
        static Logger* GetGloballLogger();
        //设置 全局 Logger对象
        static void setGloballLogger();

        void log();
    private:
        LogLevel m_set_level;
        std::queue<std::string> m_buffer;
        Mutex m_mutex;
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