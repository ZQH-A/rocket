#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include <sstream>
#include <sys/time.h>
#include <stdio.h>
#include "rocket/net/eventloop.h"
#include "rocket/common/run_time.h"




namespace rocket{

    //单例模式
    static Logger * g_logger = nullptr;

    Logger* Logger::GetGloballLogger()
    {
            return g_logger;
    }

    Logger::Logger(LogLevel level, int type /*= 1*/) : m_set_level(level), m_type(type)
    {

    }


    void Logger::init()
    {
        if(m_type == 0)  
        {
            return;
        }
        m_asnyc_logger = std::make_shared<AsyncLogger>(
            Config::GetGlobalConfig()->get_file_name() + "_rpc",
            Config::GetGlobalConfig()->get_file_path(),
            Config::GetGlobalConfig()->get_file_max_size());
        
        m_asnyc_app_logger = std::make_shared<AsyncLogger>(
            Config::GetGlobalConfig()->get_file_name() + "_app",
            Config::GetGlobalConfig()->get_file_path(),
            Config::GetGlobalConfig()->get_file_max_size());

        m_timer_event = std::make_shared<TimerEvent>(Config::GetGlobalConfig()->get_sync_interval(),true,
        std::bind(&Logger::syncLoop,this));  //bind后面做个笔记
        EventLoop::GetGurrentEventLoop()->addTimerEvent(m_timer_event);
    }
    void Logger::setGloballLogger(int type /*=1*/)
    {
        //读取出来的配置 转维 LogLevel行，并赋值给Logger
        std::string global_log_level = Config::GetGlobalConfig()->get_level();
        printf("Init log level [%s]\n", global_log_level.c_str());
        LogLevel global_level = StringtoLogLevel(global_log_level);
        g_logger = new Logger(global_level,type);

        g_logger->init();
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

        //获取当前线程处理的请求的msgid
        std::string msgid = RunTime::GetRunTime()->m_msgid;
        std::string method_name = RunTime::GetRunTime()->m_method_name;

        if(!msgid.empty())
        {
            ss << "[" << msgid << "]\t";
        }
        if(!method_name.empty())
        {
            ss << "[" << method_name << "]\t";
        }
        return ss.str();
    }

    //
    void Logger::pushLog(const std::string& msg)
    {
        if(m_type == 0)  //同步日志
        {
            printf((msg+"\n").c_str());
            return;
        }
        ScopeMutext<Mutex> lock(m_mutex);
        m_buffer.push_back(msg);
        lock.unlock();
    }

    void Logger::pushAppLog(const std::string& msg)
    {
        ScopeMutext<Mutex> lock(m_app_mutex);
        m_app_buffer.push_back(msg);
        lock.unlock();
    }

    void Logger::log()
    {
        // ScopeMutext<Mutex> lock(m_mutex);
        // std::queue<std::string> temp;
        // m_buffer.swap(temp);
        // lock.unlock();

        // while(!temp.empty())
        // {
        //     std::string msg = temp.front();
        //     temp.pop();
        //     printf(msg.c_str()); //打印到控制台
        // }
    }

    void Logger::syncLoop()
    {
        //同步buffer到 async_logger的buffer队尾
        ScopeMutext<Mutex> lock(m_mutex);
        std::vector<std::string> tmp;
        m_buffer.swap(tmp);
        lock.unlock();

        if(!tmp.empty())
        {
            m_asnyc_logger->pushLogBuffer(tmp);
        }

        //同步app_buffer 到 async_app_logger的buffer队尾
        ScopeMutext<Mutex> lock_app(m_app_mutex);
        std::vector<std::string> tmp_app;
        m_app_buffer.swap(tmp_app);
        lock_app.unlock();

        if(!tmp_app.empty())
        {
            m_asnyc_app_logger->pushLogBuffer(tmp_app);
        }
        
    }

    AsyncLogger::AsyncLogger(const std::string& file_name, const std::string& file_path, int max_size) 
    : m_file_name(file_name), m_file_path(file_path),m_max_file_size(max_size)
    {
        sem_init(&m_sempahore,0,0);

        //创建异步线程
        pthread_create(&m_thread,NULL,&AsyncLogger::Loop,this);

        

        sem_wait(&m_sempahore);
    }
    //核心函数
    void* AsyncLogger::Loop(void* arg)
    { //将buffer里面的全部数据打印到文件中，然后线程睡眠，知道有新的数据在重复这个过程

        AsyncLogger* logger = reinterpret_cast<AsyncLogger*>(arg);
        sem_post(&logger->m_sempahore);

        //初始化条件变量
        pthread_cond_init(&(logger->m_condtion),NULL);

        while(1)
        {
            ScopeMutext<Mutex> lock(logger->m_mutex);
            while ((logger->m_buffer.empty())) //为什么要用while，去搜一哈
            {
                /* code */
                pthread_cond_wait(&(logger->m_condtion),logger->m_mutex.getMutex());
            }
            //取m_buffer中的第一个
            std::vector<std::string> tmp;
            tmp.swap(logger->m_buffer.front());
            logger->m_buffer.pop();

            lock.unlock(); //解锁

            timeval now;
            gettimeofday(&now,NULL);

            struct tm now_time;
            localtime_r(&(now.tv_sec),&now_time);


            const char* format = "%Y%m%d";
            char date[32];
            strftime(date,sizeof(date),format,&now_time);

            if(std::string(date) != logger->m_date) //当前日期已经是下一天，需要换一个日志文件
            {
                logger->m_no = 0;
                logger->m_reopen_flag = true;
                logger->m_date = std::string(date);
            }

            if(logger->m_file_hanlder == NULL)  //首次打开
            {
                logger->m_reopen_flag = true;
            }        

            std::stringstream ss;  //文件名
            ss << logger->m_file_path << logger->m_file_name << "_"
            << std::string(date) << "_log.";
            std::string log_file_name = ss.str() + std::to_string(logger->m_no);

            if(logger->m_reopen_flag) //需要重新打开一个新的文件
            {
                if(logger->m_file_hanlder)  //如果句柄已经打开一个文件，关闭，然后重新打开
                {
                    fclose(logger->m_file_hanlder);
                }
                logger->m_file_hanlder = fopen(log_file_name.c_str(),"a");
                logger->m_reopen_flag = false;
            }

            if(ftell(logger->m_file_hanlder) > logger->m_max_file_size)
            { //打开文件的字节数大于最大字节 关闭后打开一个新的文件
                fclose(logger->m_file_hanlder);

                log_file_name = ss.str() + std::to_string(++(logger->m_no));
                logger->m_file_hanlder = fopen(log_file_name.c_str(),"a");
                logger->m_reopen_flag = false;
            }

            //将日志写入到文件中
            for(auto  & i : tmp)
            {
                if(!i.empty())
                {
                    fwrite(i.c_str(),1,i.length(),logger->m_file_hanlder);
                }
            }

            //刷新
            fflush(logger->m_file_hanlder);

            if(logger->m_stop_flag)
            {
                return NULL;
            }
        }

        return NULL;    
    }

    void AsyncLogger::stop() //退出循环的函数
    {
        m_stop_flag = true;
    }

    void AsyncLogger::flush() //刷新磁盘
    {
        if(m_file_hanlder)
        {
            fflush(m_file_hanlder);
        }
    }

     void AsyncLogger::pushLogBuffer(std::vector<std::string> &vec)
     {
        ScopeMutext<Mutex> lock(m_mutex);
        m_buffer.push(vec);
        lock.unlock();


        //有数据后需要唤醒异步日志线程
        pthread_cond_signal(&m_condtion);
     }
}