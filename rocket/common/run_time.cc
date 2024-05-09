#include "rocket/common/run_time.h"


namespace rocket
{
    thread_local RunTime* local_run_time = NULL;

    RunTime* RunTime::GetRunTime()
    {
        if(local_run_time != nullptr)
        {
            return local_run_time;
        }
        local_run_time = new RunTime();
        return local_run_time;
    }
} // namespace rocket
