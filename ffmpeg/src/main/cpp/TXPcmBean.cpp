//
// Created by yin13 on 2021/5/26.
//

#include "TXPcmBean.h"

TXPcmBean::TXPcmBean(int buffersize, SAMPLETYPE *buffer) {
    this->buffer = (char *) malloc(buffersize);
    this->buffersize = buffersize;
    // 复制 buffer
    memcpy(this->buffer, buffer, buffersize);
}

TXPcmBean::~TXPcmBean() {
    free(this->buffer);
    this->buffer = NULL;
}
