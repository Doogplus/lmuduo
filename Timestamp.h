//
// Created by cleon on 22-9-10.
//

#ifndef LMUDUO_TIMESTAMP_H
#define LMUDUO_TIMESTAMP_H

#include <iostream>
#include <string>

class Timestamp {
public:
    Timestamp();

    explicit Timestamp(int64_t microSecondsSinceEpoch);

    static Timestamp now();

    std::string toString() const;

private:
    int64_t microSecondsSinceEpoch_;
};

#endif //LMUDUO_TIMESTAMP_H
