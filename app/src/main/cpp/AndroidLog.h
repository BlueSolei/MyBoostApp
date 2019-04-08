//
// Created by sfridman on 4/8/2019.
//

#ifndef MYBOOSTAPP_ANDROIDLOG_H
#define MYBOOSTAPP_ANDROIDLOG_H

#include <android/log.h>

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "SharedObject", __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "SharedObject", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "SharedObject", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "SharedObject", __VA_ARGS__))

#endif //MYBOOSTAPP_ANDROIDLOG_H
