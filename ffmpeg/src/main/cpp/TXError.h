//
// Created by yin13 on 2021/5/18.
//

#ifndef APPLE_TXERROR_H
#define APPLE_TXERROR_H
#define ERROR_CODE1 101
#define ERROR_CODE2 102
#define ERROR_CODE3 103
#define ERROR_CODE4 104
#define ERROR_MSG1 "avformat_open_input error code"
#define ERROR_MSG2 "avformat_find_stream_info error"
#define ERROR_MSG3 "avcodec_find_decoder error"
#define ERROR_MSG4 "avcodec_open2 error"

class TXError {
public:

public:
    TXError();

    ~TXError();
};


#endif //APPLE_TXERROR_H
