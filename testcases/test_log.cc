#include "rocket/common/log.h"
#include <pthread.h>
#include <unistd.h>
#include "rocket/common/config.h"

void * fun(void *)
{
    int i = 20;
    while(i --)
    {
        DEBUGLOG("debug this is thread in %s","fun");
        INFOLOG("info this is thread in %s","fun");
    }

    return NULL;
}
int main()
{

    //初始化读取配置的类
    rocket::Config::setGlobalConfig("../conf/rocket.xml");
    //初始化Logger类
    rocket::Logger::setGloballLogger();

    pthread_t thread;
    pthread_create(&thread,NULL,&fun,NULL);

    int i = 20;
    while(i--)
    {
        DEBUGLOG("debug test Log %s","11");
        INFOLOG("info test Log %s","11");
    }
    
    // sleep(1);
    pthread_join(thread,NULL);
    return 0;
}