//
// Created by cleon on 22-9-11.
//

#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"

#include <errno.h>
#include <unistd.h>
#include <string.h>

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOG_FATAL("epoll create error: :%d\n", errno);
    }
}

EPollPoller::~EPollPoller() noexcept {
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activateChannels) {
    LOG_INFO("func:%s fd total count: %lu", __FILE__, channels_.size());
    int num_events = ::epoll_wait(epollfd_, &*events_.begin(),
                                    static_cast<int>(events_.size()),
                                    timeoutMs);
    int saved_errno = errno;
    Timestamp now(Timestamp::now());
    if (num_events > 0) {
        LOG_INFO("%d events happened\n", num_events);
        fillActivateChannels(num_events, activateChannels);
        if (num_events == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (num_events == 0) {
        LOG_INFO("%s timeout!\n", __FILE__);
    } else {
        if (saved_errno != EINTR) {
            errno = saved_errno;
            LOG_ERROR("EPollPoller::poll() err!\n");
        }
    }

    return now;
}

void EPollPoller::updateChannel(Channel* channel) {
    const int index = channel->index();
    LOG_INFO("func: %s fd=%d events=%d index=%d\n", __FUNCTION__, channel->fd(),
             channel->events(), channel->index());
    if (index == kNew || index == kDeleted) {
        if (index == kNew) {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        int fd = channel->fd();
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}
void EPollPoller::removeChannel(Channel* channel) {
    int fd = channel->fd();
    channels_.erase(fd);

    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, fd);
    int index = channel->index();
    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EPollPoller::fillActivateChannels(int numEvents, ChannelList* activateChannels) const {
    for (int i = 0; i < numEvents; i++) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activateChannels->push_back(channel);
    }
}

void EPollPoller::update(int operation, Channel* channel) {
    epoll_event event;
    bzero(&event, sizeof event);
    int fd = channel->fd();

    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR("epoll_ctl del error: %d\n", errno);
        } else {
            LOG_FATAL("epoll_ctl add/mod error: %d\n", errno);
        }
    }
}
