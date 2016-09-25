#ifndef DEBUG_H_
#define DEBUG_H_

#if 1
#include <stdio.h>
#define NET_DBG(fmt, args...)	printf("%s: "fmt,__func__, ## args)

#else
//ss
#include <android/log.h>
#define LOG_TAG    "JNI LOG"
#define LOGE(...)  		__android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define NET_DBG(...) 	__android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

#endif
