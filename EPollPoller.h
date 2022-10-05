//
// Created by cleon on 22-9-11.
//

#ifndef LMUDUO_EPOLLPOLLER_H
#define LMUDUO_EPOLLPOLLER_H

#include "Poller.h"
#include <vector>
#include <sys/epoll.h>

class Channel;

class EPollPoller : public Poller {
public:
    EPollPoller(EventLoop* loop);
    ~EPollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList* activateChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;

    void fillActivateChannels(int numEvents, ChannelList* activateChannels) const;
    void update(int operation, Channel* channel);

    using EventList = std::vector<epoll_event>;
    int epollfd_;
    EventList events_;
};

#endif //LMUDUO_EPOLLPOLLER_H
