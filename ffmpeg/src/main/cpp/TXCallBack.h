//
// Created by yin13 on 2021/5/25.
//

#ifndef APPLE_TXCALLBACK_H
#define APPLE_TXCALLBACK_H


class TXCallBack {
public:
    int buffer_size;
    void *buffer;

public:
    TXCallBack(int buffer_size, void *buffer);

    ~TXCallBack();
};


#endif //APPLE_TXCALLBACK_H
