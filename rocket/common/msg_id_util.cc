#include "rocket/common/msg_id_util.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "rocket/common/log.h"

namespace rocket
{
    static int g_msg_id_length = 20; //字符序列长度
    static int g_random_fd = -1; //判断是否打开了一个随机文件

    static thread_local std::string t_msg_id_no; //每个线程都有一个msg_id
    static thread_local std::string t_max_mag_id_no; //有一个最大限制的msg_id

        //获取随机的一个字符序列
    std::string MsgIDUtil::GenMsgID()
    {
        if(t_msg_id_no.empty() || t_msg_id_no == t_max_mag_id_no) //msg_id为空或者为最大值了就要重新生成
        {
            if(g_random_fd == -1)
            {
                g_random_fd = open("/dev/urandom",O_RDONLY);
            }

            std::string res(g_msg_id_length,0);
            if((read(g_random_fd,&res[0],g_msg_id_length)) != g_msg_id_length)
            { //没有从文件中读取到g_msg_id_length的字符
                ERRORLOG("read from /dev/urandom error");
                return "";
            }

            for(int i = 0; i < g_msg_id_length; ++i)
            {
                uint8_t x = ((uint8_t)(res[i])) % 10;
                res[i] = x + '0';
                t_max_mag_id_no += "9";
            }
            t_msg_id_no = res;
        }else{ //已经存在msg_id,下一个id则是在前一个id上加1
            int i = t_msg_id_no.length() - 1;
            while (t_msg_id_no[i] == '9' && i >= 0)
            {
                i--;
            }
            if(i >= 0)
            {
                t_msg_id_no[i] += 1;
                for(size_t j = i + 1; j < t_msg_id_no.length(); ++j)
                {
                    t_msg_id_no[j] = '0';
                }
            }
            
        }
        return t_msg_id_no;
    }
} // namespace rocket
