//
// Created by cleon on 22-9-8.
//

#ifndef LMUDUO_NONCOPYABLE_H
#define LMUDUO_NONCOPYABLE_H

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif //LMUDUO_NONCOPYABLE_H
