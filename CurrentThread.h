//
// Created by cleon on 22-9-12.
//

#ifndef LMUDUO_CURRENTTHREAD_H
#define LMUDUO_CURRENTTHREAD_H

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread {
    extern __thread int t_cachedTid;

    void cachedTid();

    inline int tid() {
        if (__builtin_expect(t_cachedTid == 0, 0)) {
            cachedTid();
        }
        return t_cachedTid;
    }
}

#endif //LMUDUO_CURRENTTHREAD_H
