//
// Created by cleon on 22-10-2.
//

#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

ssize_t Buffer::readFd(int fd, int *saveErrno) {
    char extra_buf[65536] = {0};  // 栈上
    struct iovec vec[2];
    const size_t writable = writableBytes(); // 剩余可写空间
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extra_buf;
    vec[1].iov_len = sizeof extra_buf;
    const int iov_cnt = (writable < sizeof extra_buf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iov_cnt);
    if (n < 0) {
        *saveErrno = errno;
    } else if (n <= writable) {
        writerIndex_ += n;
    } else {
        writerIndex_ = buffer_.size();
        append(extra_buf, n - writable);
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno) {
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0) {
        *saveErrno = errno;
    }
    return n;
}