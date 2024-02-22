#include "rocket/common/config.h"
#include <tinyxml/tinyxml.h>

//读取root节点和log节点的宏
#define READ_XML_NODE(name, parent) \
TiXmlElement * name##_node = parent->FirstChildElement(#name); \
if(!name##_node) {  \
    printf("Start rocket server error, failed to read node %s\n",#name); \
    exit(0); \
} \
//读取配置的宏   并获取配置中的文字
#define READ_STR_FROM_XML_NODE(name, parent) \
    TiXmlElement* name##_node = parent->FirstChildElement(#name); \
    if(!name##_node || !name##_node->GetText()) \
    {  \
        printf("Start rocket server error, failed to read node %s\n",#name); \
        exit(0); \
    } \
    std::string name##_str = std::string(name##_node->GetText());  \


namespace rocket{

    static Config* g_config = NULL;
    Config* Config::GetGlobalConfig()
    {
        return g_config;
    }

    void Config::setGlobalConfig(const char* xmlfile)
    {
        if(g_config == NULL)
        {
            g_config = new Config(xmlfile);
        }
    }


    Config::Config(const char* xmlfile)
    {
        TiXmlDocument* xml_document = new TiXmlDocument();

        bool rt = xml_document->LoadFile(xmlfile); //加载xml文件
        //如果读取失败 返回false
        if(! rt)
        {
            printf("Start rocket server error, failed to read config file %s, error info[%s] \n",xmlfile, xml_document->ErrorDesc());
            exit(0);
        }

        //将下面的两个用 宏READ_XML_NODE代替 
        // //读取第一个root节点
        // TiXmlElement* root_node = xml_document->FirstChildElement("root");
        // if(!root_node)  //如果第一个root节点是空，也要报错并返回
        // {
        //     printf("Start rocket server error, failed to read node %s","root");
        //     exit(0);
        // }

        // //读取log节点
        // TiXmlElement* log_node = root_node->FirstChildElement("log");
        // if(!log_node)  //如果第一个root节点是空，也要报错并返回
        // {
        //     printf("Start rocket server error, failed to read node %s","log");
        //     exit(0);
        // }

        READ_XML_NODE(root, xml_document);
        READ_XML_NODE(log, root_node);


        //同样也封装成一个宏
        // TiXmlElement* log_level_node = log_node->FirstChildElement("log_level");
        // if(!log_level_node || !log_level_node->GetText())
        // {
        //     printf("Start rocket server error, failed to read node %s","log-level");
        //     exit(0);
        // }
        // //获取log-level中的文本
        // std::string log_level = std::string(log_level->GetText());
        READ_STR_FROM_XML_NODE(log_level,log_node);

        m_log_level = log_level_str;
    }
}