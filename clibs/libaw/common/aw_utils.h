/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

/*
 utils log等便利函数
 */

#ifndef aw_utils_h
#define aw_utils_h

#include <stdio.h>
#include <string.h>
#include "aw_alloc.h"

#define AWLog(...)  \
do{ \
printf(__VA_ARGS__); \
printf("\n");\
}while(0)

#define aw_log(...) AWLog(__VA_ARGS__)

//视频编码加速，stride须设置为16的倍数
#define aw_stride(wid) ((wid % 16 != 0) ? ((wid) + 16 - (wid) % 16): (wid))

#endif /* aw_utils_h */
