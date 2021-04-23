//
// Created by yin13 on 2021/3/31.
//

#ifndef APPLE_THREAD_TEST_H
#define APPLE_THREAD_TEST_H

#include <jni.h>
#include "pthread.h"
#include "../android_log.h"

class ThreadTest {
public:
    char *name;
    pthread_t *pt;

public:
    ThreadTest();

    ThreadTest(pthread_t *pt, char *name);

    ~ThreadTest();


};


#endif //APPLE_THREAD_TEST_H
