//
// 播放器回调方法
// Created by yin13 on 2021/4/17.
//
#include "TXCallJava.h"

TXCallJava::TXCallJava(JavaVM *vm, JNIEnv *env, jobject *obj) {
    this->javaVm = vm;
    this->jniEnv = env;
    //取指针的值
    this->job = jniEnv->NewGlobalRef(*obj);
    jclass aClass = jniEnv->GetObjectClass(job);
    if (aClass) {
        jmethodId = jniEnv->GetMethodID(aClass, JAVA_METHOD_PARPARED, "()V");
        jmethodIdCallLoad = jniEnv->GetMethodID(aClass, JAVA_METHOD_LOAD, "(Z)V");
        jmethodIdTimeInfo = jniEnv->GetMethodID(aClass, JAVA_METHOD_TIME_INFO, "(II)V");
        jmethodIdError = jniEnv->GetMethodID(aClass, JAVA_METHOD_ERROR, "(ILjava/lang/String;)V");
        jmethodIdComplete = jniEnv->GetMethodID(aClass, JAVA_METHOD_COMPLETE, "()V");
        jmethodIdValumeDB = jniEnv->GetMethodID(aClass, JAVA_METHOD_VALUME_DB, "(I)V");
        jmethodIdPcmAAc = jniEnv->GetMethodID(aClass, JAVA_METHOD_PCM_AAC, "(I[B)V");
        jmethodIdRecordTime = jniEnv->GetMethodID(aClass, JAVA_METHOD_RECORD_TIME, "(F)V");
        jmethodIdCutAudio = jniEnv->GetMethodID(aClass, JAVA_METHOD_CUT_AUDIO, "(I[B)V");
        jmethodIdRenderYUV = jniEnv->GetMethodID(aClass, JAVA_METHOD_RENDER_YUV, "(II[B[B[B)V");
        jmethodIdIsSupportMediaCodec = jniEnv->GetMethodID(aClass,
                                                           JAVA_METHOD_IS_SUPPORT_MEDIA_CODEC,
                                                           "(Ljava/lang/String;)Z");
        jmethodIdInitMediaCodecVideo = jniEnv->GetMethodID(aClass,
                                                           JAVA_METHOD_INIT_MEDIA_CODEC_VIDEO,
                                                           "(Ljava/lang/String;II[B[B)V");

        jmethodIdDecodeAvPacket = jniEnv->GetMethodID(aClass, JAVA_METHOD_DECODE_AV_PACKET,
                                                      "(I[B)V");
    }
}

TXCallJava::~TXCallJava() {
    if (jniEnv != NULL) {
        job = NULL;
        jniEnv = NULL;
    }
}

void TXCallJava::onParpared(int type) {
    SDK_LOG_D("type%d: ", type);
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(job, jmethodId);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if (javaVm->AttachCurrentThread(&env, 0) != JNI_OK) {
            return;
        }
        env->CallVoidMethod(job, jmethodId);
        javaVm->DetachCurrentThread();
    }
}

void TXCallJava::onLoad(int type, bool isLoad) {
    SDK_LOG_D("isload%d", isLoad);
    if (type == MAIN_THREAD) {
        jniEnv->CallBooleanMethod(job, jmethodIdCallLoad, isLoad);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if (javaVm->AttachCurrentThread(&env, 0) != JNI_OK) {
            return;
        }
        //TODO seek 时 javaVm->DetachCurrentThread() 会导致崩溃
//        env->CallBooleanMethod(job, jmethodIdCallLoad, isLoad);
        javaVm->DetachCurrentThread();
    }
}

void TXCallJava::onTimeInfo(int type, int currentTime, int total) {
    SDK_LOG_D("onTimeInfo,currenttime %d ,totaltime %d", currentTime, total);
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(job, jmethodIdTimeInfo, currentTime, total);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if (javaVm->AttachCurrentThread(&env, 0) != JNI_OK) {
            return;
        }
        env->CallVoidMethod(job, jmethodIdTimeInfo, currentTime, total);
        javaVm->DetachCurrentThread();
    }
}

void TXCallJava::onError(int type, int code, char *errorMsg) {
    SDK_LOG_D("onError,code %d ,errorMsg %s", code, errorMsg);
    // 无法回调回去
    if (type == MAIN_THREAD) {
        jstring msg = jniEnv->NewStringUTF(errorMsg);
        jniEnv->CallVoidMethod(job, jmethodIdError, code, msg);
        jniEnv->DeleteLocalRef(msg);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if (javaVm->AttachCurrentThread(&env, 0) != JNI_OK) {
            return;
        }
        jstring msg = env->NewStringUTF(errorMsg);
        env->CallVoidMethod(job, jmethodIdError, code, msg);
        env->DeleteLocalRef(msg);
        javaVm->DetachCurrentThread();
    }
}

void TXCallJava::onCallComplete(int type) {
    SDK_LOG_D("onCallComplete");
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(job, jmethodIdComplete);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if (javaVm->AttachCurrentThread(&env, 0) != JNI_OK) {
            return;
        }
        env->CallVoidMethod(job, jmethodIdComplete);
        javaVm->DetachCurrentThread();
    }
}

void TXCallJava::onCallValumeDB(int type, int db) {
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(job, jmethodIdValumeDB, db);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if (javaVm->AttachCurrentThread(&env, 0) != JNI_OK) {
            return;
        }
        env->CallVoidMethod(job, jmethodIdValumeDB, db);
        javaVm->DetachCurrentThread();
    }
}

void TXCallJava::onCallOnPcmTAAc(int type, int size, void *pcmBuffer) {
    if (type == MAIN_THREAD) {
        jbyteArray pArray = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(pArray, 0, size, static_cast<const jbyte *>(pcmBuffer));
        jniEnv->CallVoidMethod(job, jmethodIdPcmAAc, size, pArray);
        jniEnv->DeleteLocalRef(pArray);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if (javaVm->AttachCurrentThread(&env, 0) != JNI_OK) {
            return;
        }
        jbyteArray pArray = env->NewByteArray(size);
        env->SetByteArrayRegion(pArray, 0, size, static_cast<const jbyte *>(pcmBuffer));
        env->CallVoidMethod(job, jmethodIdPcmAAc, size, pArray);
        env->DeleteLocalRef(pArray);
        javaVm->DetachCurrentThread();
    }
}

void TXCallJava::onCallOnRecordTime(int type, float time) {
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(job, jmethodIdRecordTime, time);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if (javaVm->AttachCurrentThread(&env, 0) != JNI_OK) {
            return;
        }
        env->CallVoidMethod(job, jmethodIdRecordTime, time);
        javaVm->DetachCurrentThread();
    }
}

void TXCallJava::onCallOnCutAudio(int type, int sampleRate, int size, void *pcmBuffer) {
    if (type == MAIN_THREAD) {
        jbyteArray pArray = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(pArray, 0, size, static_cast<const jbyte *>(pcmBuffer));
        jniEnv->CallVoidMethod(job, jmethodIdCutAudio, sampleRate, pArray);
        jniEnv->DeleteLocalRef(pArray);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if (javaVm->AttachCurrentThread(&env, 0) != JNI_OK) {
            return;
        }
        jbyteArray pArray = env->NewByteArray(size);
        env->SetByteArrayRegion(pArray, 0, size, static_cast<const jbyte *>(pcmBuffer));
        env->CallVoidMethod(job, jmethodIdCutAudio, sampleRate, pArray);
        env->DeleteLocalRef(pArray);
        javaVm->DetachCurrentThread();
    }
}

void
TXCallJava::onCallOnRenderYUV(int type, int width, int height, uint8_t *y, uint8_t *u, uint8_t *v) {
    SDK_LOG_D("onCallOnRenderYUV width: %d, height: %d", width, height);
    if (type == MAIN_THREAD) {
        // YUV  4:1:1
        int size = width * height;
        jbyteArray byteArrayY = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(byteArrayY, 0, size, reinterpret_cast<const jbyte *>(y));

        jbyteArray byteArrayU = jniEnv->NewByteArray(size / 4);
        jniEnv->SetByteArrayRegion(byteArrayU, 0, size / 4, reinterpret_cast<const jbyte *>(u));

        jbyteArray byteArrayV = jniEnv->NewByteArray(size / 4);
        jniEnv->SetByteArrayRegion(byteArrayV, 0, size / 4, reinterpret_cast<const jbyte *>(v));

        jniEnv->CallVoidMethod(job, jmethodIdRenderYUV, width, height, byteArrayY, byteArrayU,
                               byteArrayV);
        jniEnv->DeleteLocalRef(byteArrayY);
        jniEnv->DeleteLocalRef(byteArrayU);
        jniEnv->DeleteLocalRef(byteArrayV);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if ((javaVm->AttachCurrentThread(&env, 0)) != JNI_OK) {
            return;
        }
        int size = width * height;
        jbyteArray byteArrayY = env->NewByteArray(size);
        env->SetByteArrayRegion(byteArrayY, 0, size, reinterpret_cast<const jbyte *>(y));

        jbyteArray byteArrayU = env->NewByteArray(size / 4);
        env->SetByteArrayRegion(byteArrayU, 0, size / 4, reinterpret_cast<const jbyte *>(u));

        jbyteArray byteArrayV = env->NewByteArray(size / 4);
        env->SetByteArrayRegion(byteArrayV, 0, size / 4, reinterpret_cast<const jbyte *>(v));

        env->CallVoidMethod(job, jmethodIdRenderYUV, width, height, byteArrayY, byteArrayU,
                            byteArrayV);
        env->DeleteLocalRef(byteArrayY);
        env->DeleteLocalRef(byteArrayU);
        env->DeleteLocalRef(byteArrayV);
        javaVm->DetachCurrentThread();
    }

}

bool TXCallJava::onCallIsSupportMediaCodec(int type, const char *s) {
    bool support = false;
    if (type == MAIN_THREAD) {
        jstring codecName = jniEnv->NewStringUTF(s);
        support = jniEnv->CallBooleanMethod(job, jmethodIdIsSupportMediaCodec, codecName);
        jniEnv->DeleteLocalRef(codecName);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if (javaVm->AttachCurrentThread(&env, 0) != JNI_OK) {
            return support;
        }
        jstring codecName = env->NewStringUTF(s);
        support = env->CallBooleanMethod(job, jmethodIdIsSupportMediaCodec, codecName);
        env->DeleteLocalRef(codecName);
        javaVm->DetachCurrentThread();
    }
    return support;
}

void
TXCallJava::onCallInitMediaCodecVideo(int type, const char *mime, int width, int height, int size,
                                      uint8_t *csd_01,
                                      int size2, uint8_t *csd_02) {
    if (type == MAIN_THREAD) {
        jstring string = jniEnv->NewStringUTF(mime);
        jbyteArray csd1Array = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(csd1Array, 0, size, reinterpret_cast<const jbyte *>(csd_01));
        jbyteArray csd2Array = jniEnv->NewByteArray(size2);
        jniEnv->SetByteArrayRegion(csd2Array, 0, size2, reinterpret_cast<const jbyte *>(csd_02));
        jniEnv->CallVoidMethod(job, jmethodIdInitMediaCodecVideo, string, width, height, csd1Array,
                               csd2Array);
        jniEnv->DeleteLocalRef(string);
        jniEnv->DeleteLocalRef(csd1Array);
        jniEnv->DeleteLocalRef(csd2Array);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if ((javaVm->AttachCurrentThread(&env, 0) != JNI_OK)) {
            return;
        }
        jstring string = env->NewStringUTF(mime);
        jbyteArray csd1Array = env->NewByteArray(size);
        env->SetByteArrayRegion(csd1Array, 0, size, reinterpret_cast<const jbyte *>(csd_01));
        jbyteArray csd2Array = env->NewByteArray(size2);
        env->SetByteArrayRegion(csd2Array, 0, size2, reinterpret_cast<const jbyte *>(csd_02));
        env->CallVoidMethod(job, jmethodIdInitMediaCodecVideo, string, width, height, csd1Array,
                            csd2Array);
        env->DeleteLocalRef(string);
        env->DeleteLocalRef(csd1Array);
        env->DeleteLocalRef(csd2Array);
        javaVm->DetachCurrentThread();
    }
}

void TXCallJava::onCallDecodeAvPacket(int type, int size, uint8_t *buffer) {
    if (type == MAIN_THREAD) {
        jbyteArray csd1Array = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(csd1Array, 0, size, reinterpret_cast<const jbyte *>(buffer));
        jniEnv->CallVoidMethod(job, jmethodIdDecodeAvPacket, size, csd1Array);
        jniEnv->DeleteLocalRef(csd1Array);
    } else if (type == CHILD_THREAD) {
        JNIEnv *env;
        if ((javaVm->AttachCurrentThread(&env, 0) != JNI_OK)) {
            return;
        }
        jbyteArray csd1Array = env->NewByteArray(size);
        env->SetByteArrayRegion(csd1Array, 0, size, reinterpret_cast<const jbyte *>(buffer));
        env->CallVoidMethod(job, jmethodIdDecodeAvPacket, size, csd1Array);
        env->DeleteLocalRef(csd1Array);
        javaVm->DetachCurrentThread();
    }

}

