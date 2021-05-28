//
// Created by yin13 on 2021/5/26.
//

#ifndef APPLE_TXPCMBEAN_H
#define APPLE_TXPCMBEAN_H

#include "soundtouch/source/SoundTouch.h"

using namespace soundtouch;

class TXPcmBean {
public:
    int buffersize;
    char *buffer;
public :
    TXPcmBean(int buffersize, SAMPLETYPE *buffer);

    ~TXPcmBean();
};


#endif //APPLE_TXPCMBEAN_H
