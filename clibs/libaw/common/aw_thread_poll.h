/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

/*
 封装了便于使用的pthread线程池，使用时直接向线程池丢任务即可。
 */

#ifndef aw_thread_h
#define aw_thread_h

#include <stdio.h>
#include <pthread/pthread.h>

typedef struct aw_thread_poll aw_thread_poll;
typedef void (*aw_thread_func)(void *);
typedef void (*aw_thread_finish_cb)();

//创建线程池
extern aw_thread_poll *alloc_aw_thread_poll(int limit, int stored_task_list_limit);
//释放线程池
extern void free_aw_thread_poll(aw_thread_poll **poll_p, aw_thread_finish_cb finish_cb);
//添加任务
extern void aw_add_task_to_thread_poll(aw_thread_poll *poll, aw_thread_func func, void *param);

//累积的未发送的thread poll数量
extern int aw_stored_task_count_in_thread_poll(aw_thread_poll *poll);

extern void test_thread();

#endif /* aw_thread_h */
