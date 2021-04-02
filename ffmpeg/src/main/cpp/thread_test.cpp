//
// Created by yin13 on 2021/3/31.
//

#include "thread_test.h"

void *callBack(void *data) {
    SDK_LOG_D("c++ callback ");
    pthread_exit(data);
}

ThreadTest::~ThreadTest() {
    if (pt != nullptr) {
        pthread_exit(pt);
    }
}

ThreadTest::ThreadTest(pthread_t *__pt, char *__name) {
    this->pt = __pt;
    this->name = __name;
    SDK_LOG_D("c++ callback %s ", name);
    pthread_create(pt, NULL, callBack, pt);

}

