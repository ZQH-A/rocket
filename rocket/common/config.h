#ifndef ROCKET_COMMON_CONFIG_H
#define ROCKET_COMMON_CONFIG_H

#include <map>

//使用map传配置变量
namespace rocket
{
    class Config{
        public:
        //传入xml文件的路径  服务端使用
        Config(const char* xmlfile);

        Config();  //客户端使用

        static Config* GetGlobalConfig();

        static void setGlobalConfig(const char* xmlfile);

        std::string get_level()
        {
            return m_log_level;
        }

        std::string get_file_name()
        {
            return m_log_file_name;
        }

        std::string get_file_path()
        {
            return m_log_file_path;
        }

        int get_file_max_size()
        {
            return m_log_max_file_size;
        }

        int get_sync_interval(){
            return m_log_sync_interval;
        }

        int get_port()
        {
            return m_port;
        }

        int get_io_threads()
        {
            return m_io_threads;
        }

        private:
        std::string m_log_level;
        std::string m_log_file_name;  //日志文件名
        std::string m_log_file_path;  //日志文件路径
        int m_log_max_file_size {0};  //日志文件最大字节数
        int m_log_sync_interval {0}; //日志同步间隔  ms

        int m_port {0}; //端口号
        int m_io_threads {0}; //子线程数
    };
} // namespace rocket


#endif