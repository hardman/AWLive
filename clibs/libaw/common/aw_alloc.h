/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

/*
 对 malloc 和 free 进行封装，可以跟踪内存分配情况，方便调试
 */

#ifndef aw_alloc_h
#define aw_alloc_h

#include <stdio.h>

#if defined(__FILE__) && defined(__LINE__)
#define aw_alloc(size) aw_alloc_detail((size), __FILE__, __LINE__)
#else
#define aw_alloc(size) aw_alloc_detail((size), "", 0)
#endif

//可以监视内存的分配和释放，便于调试内存泄漏

//自定义 alloc
extern void * aw_alloc_detail(size_t size, const char *file_name, uint32_t line);

//自定义 free
extern void aw_free(void *);

//开始debug alloc，调用此函数开始才记录分配和释放数量
extern void aw_init_debug_alloc();

//停止debug alloc，调用此函数后，不再记录分配释放。所有记录清0
extern void aw_uninit_debug_alloc();

//返回总共alloc的size
extern size_t aw_total_alloc_size();

//返回总共free的size
extern size_t aw_total_free_size();

//打印内存alloc/free/leak状况
extern void aw_print_alloc_description();

#endif /* aw_alloc_h */
