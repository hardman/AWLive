/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://blog.csdn.net/hard_man/
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

#endif /* aw_utils_h */
