//
// Created by cleon on 22-9-11.
//

#include "Poller.h"
// #include "PollPoller.h"
#include "EPollPoller.h"

#include <stdlib.h>

Poller* Poller::newDefaultPoller(EventLoop *loop) {
    if (::getenv("MUDUO_USE_POLL")) {
        return nullptr;
    } else {
        return new EPollPoller(loop);
    }
}