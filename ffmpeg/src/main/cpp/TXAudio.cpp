//
// Created by yin13 on 2021/4/17.
//

#include "TXAudio.h"

TXAudio::TXAudio(TXPlayStatus *txPlayStatus) {
    this->playStatus = txPlayStatus;
    queue = new TXQueue(playStatus);
}
