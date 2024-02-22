#ifndef ROCKET_COMMON_CONFIG_H
#define ROCKET_COMMON_CONFIG_H

#include <map>

//使用map传配置变量
namespace rocket
{
    class Config{
        public:
        //传入xml文件的路径
        Config(const char* xmlfile);

        static Config* GetGlobalConfig();

        static void setGlobalConfig(const char* xmlfile);

        std::string get_level()
        {
            return m_log_level;
        }
        private:
        std::string m_log_level;

    };
} // namespace rocket


#endif