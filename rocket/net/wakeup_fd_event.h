#ifndef ROCKET_NET_WAKEUP_FDEVENT_H
#define ROCKET_NET_WAKEUP_FDEVENT_H

#include "rocket/net/fd_event.h"


namespace rocket{

    class WakeUpFdEvent : public FdEvent{ //继承自Fd

        public: 
        WakeUpFdEvent(int fd);

        ~WakeUpFdEvent();

        void init();

        void wakeup();
        private:

    };

}

#endif