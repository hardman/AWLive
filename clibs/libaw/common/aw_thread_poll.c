/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#include "aw_thread_poll.h"
#include "aw_alloc.h"
#include "aw_utils.h"
#include "aw_array.h"
#include <unistd.h>

typedef struct aw_thread_poll_task{
    aw_thread_func func;
    void *param;
}aw_thread_poll_task;

typedef struct aw_thread_poll_element{
    int index;
    pthread_t *thread;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    aw_thread_poll *poll;
    
    int8_t is_finish;
    
    //传入
    aw_thread_poll_task task;
} aw_thread_poll_element;

static aw_thread_poll_element * alloc_aw_thread_poll_element(aw_thread_poll *poll, int index){
    aw_thread_poll_element *ele = aw_alloc(sizeof(aw_thread_poll_element));
    memset(ele, 0, sizeof(aw_thread_poll_element));
    
    ele->thread = aw_alloc(sizeof(pthread_t));
    
    ele->mutex = aw_alloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(ele->mutex, NULL);
    
    ele->cond = aw_alloc(sizeof(pthread_cond_t));
    pthread_cond_init(ele->cond, NULL);
    
    ele->poll = poll;
    
    ele->index = index;
    
    return ele;
}

static void free_aw_thread_poll_element(aw_thread_poll_element ** ele_p){
    aw_thread_poll_element *ele = *ele_p;
    if (ele) {
        if (ele->thread) {
            aw_free(ele->thread);
        }
        
        if (ele->mutex) {
            aw_free(ele->mutex);
        }
        
        if (ele->cond) {
            aw_free(ele->cond);
        }
        
        aw_free(ele);
    }
    
    *ele_p = NULL;
}

//线程池
typedef struct aw_thread_poll {
    uint32_t limit;//几个线程
    aw_thread_poll_element **elements;
    uint8_t is_running;
    
    //任务列表
    aw_array *task_list;
    uint32_t task_list_limit;
    
    pthread_mutex_t *task_list_mutex;
    
    //结束回调
    aw_thread_finish_cb finish_cb;
}aw_thread_poll;

static void s_inner_thread_poll_running_function(aw_thread_poll_element *th_ele){
    //    aw_log("[d] thread %d is start..\n", th_ele->index);
    while (th_ele && th_ele->poll && th_ele->poll->is_running) {
        pthread_mutex_lock(th_ele->mutex);
        if (th_ele->task.func) {
            //            aw_log("[d] thread %d will run task..\n", th_ele->index);
            th_ele->task.func(th_ele->task.param);
            th_ele->task.func = NULL;
            th_ele->task.param = NULL;
            //            aw_log("[d] thread %d finish run task..\n", th_ele->index);
        }else{
            pthread_mutex_lock(th_ele->poll->task_list_mutex);
            aw_array *task_list = th_ele->poll->task_list;
            if (task_list->count > 0) {
                aw_thread_poll_task *task = aw_array_element_at_index(task_list, 0)->pointer_value;
                aw_array_remove_element_at_index(task_list, 0);
                th_ele->task.func = task->func;
                th_ele->task.param = task->param;
                aw_free(task);
                //                aw_log("[d]continue-- %d task list count=%ld", th_ele->index, task_list->count);
                pthread_mutex_unlock(th_ele->poll->task_list_mutex);
                pthread_mutex_unlock(th_ele->mutex);
                continue;
            }
            pthread_mutex_unlock(th_ele->poll->task_list_mutex);
            
            //            aw_log("[d] thread %d will waiting..\n", th_ele->index);
            pthread_cond_wait(th_ele->cond, th_ele->mutex);
            //            aw_log("[d] thread %d is waked up..\n", th_ele->index);
        }
        pthread_mutex_unlock(th_ele->mutex);
    }
    
    pthread_mutex_lock(th_ele->mutex);
    th_ele->is_finish = 1;
    pthread_mutex_unlock(th_ele->mutex);
    
    //    aw_log("[d] thread %d is finished..\n", th_ele->index);
}

extern aw_thread_poll *alloc_aw_thread_poll(int limit, int stored_task_list_limit){
    aw_thread_poll *th_poll = aw_alloc(sizeof(aw_thread_poll));
    memset(th_poll, 0, sizeof(aw_thread_poll));
    
    th_poll->limit = limit;
    
    th_poll->task_list_limit = stored_task_list_limit;
    
    if (!th_poll->task_list_limit) {
        th_poll->task_list_limit = 100;
    }
    
    //下标
    th_poll->elements = aw_alloc(sizeof(aw_thread_poll_element*) * limit);
    
    //任务列表
    th_poll->task_list = alloc_aw_array(0);
    //列表锁
    th_poll->task_list_mutex = aw_alloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(th_poll->task_list_mutex, NULL);
    
    th_poll->is_running = 1;
    
    for (int i = 0; i < limit; i++) {
        aw_thread_poll_element *element = alloc_aw_thread_poll_element(th_poll, i);
        th_poll->elements[i] = element;
        //线程
        pthread_t *pthread = element->thread;
        //调度策略
        pthread_attr_t pth_attr;
        pthread_attr_init(&pth_attr);
        pthread_attr_setschedpolicy(&pth_attr, SCHED_OTHER);
        pthread_create(pthread, &pth_attr, (void *)s_inner_thread_poll_running_function, element);
    }
    
    return th_poll;
}

static void aw_free_thread_poll_inner(aw_thread_poll *poll){
    if (poll) {
        if (poll->elements) {
            int i = 0;
            for (; i < poll->limit; i++) {
                aw_thread_poll_element *ele = poll->elements[i];
                if (!ele->is_finish) {
                    pthread_cond_signal(ele->cond);
                    pthread_join(*ele->thread, NULL);
                }
                free_aw_thread_poll_element(&ele);
            }
            
            aw_free(poll->elements);
            poll->elements = NULL;
        }
        
        if (poll->task_list) {
            free_aw_array(&poll->task_list);
        }
        
        if (poll->task_list_mutex) {
            aw_free(poll->task_list_mutex);
            poll->task_list_mutex = NULL;
        }
        
        aw_thread_finish_cb finish_cb = poll->finish_cb;
        
        aw_free(poll);
        
        if (finish_cb) {
            finish_cb();
        }
    }
    
    //    aw_log("[d]thread_poll is finished step 2\n");
}

extern void free_aw_thread_poll(aw_thread_poll **poll_p, aw_thread_finish_cb finish_cb){
    aw_thread_poll *poll = *poll_p;
    if (poll) {
        poll->is_running = 0;
        poll->finish_cb = finish_cb;
        pthread_t finish_th;
        pthread_create(&finish_th, NULL, (void *)aw_free_thread_poll_inner, poll);
    }else{
        if (finish_cb) {
            finish_cb();
        }
    }
    //    aw_log("[d]thread_poll is finished step 1\n");
}

extern void aw_add_task_to_thread_poll(aw_thread_poll *poll, aw_thread_func func, void *param){
    if (poll->is_running) {
        aw_thread_poll_element *avaliable_th = NULL;
        int i = 0;
        for (; i < poll->limit; i++) {
            aw_thread_poll_element *th_ele = poll->elements[i];
            if (!th_ele->task.func) {
                avaliable_th = th_ele;
                break;
            }
        }
        if (avaliable_th) {
            avaliable_th->task.func = func;
            avaliable_th->task.param = param;
            pthread_cond_signal(avaliable_th->cond);
        }else{
            aw_thread_poll_task *task = aw_alloc(sizeof(aw_thread_poll_task));
            task->func = func;
            task->param = param;
            pthread_mutex_lock(poll->task_list_mutex);
            if (poll->task_list->count >= poll->task_list_limit) {
                aw_thread_poll_task *remove_task = aw_array_element_at_index(poll->task_list, 0)->pointer_value;
                aw_array_remove_element_at_index(poll->task_list, 0);
                aw_free(remove_task);
                aw_log("[w] ---- thread poll stored tasks too much, so remove the first one task");
                aw_log("[w] ---- you can set task_list_limit with a larger value, current task_list_limit=%d\n", poll->task_list_limit);
            }
            aw_array_add_pointer(&poll->task_list, task);
            pthread_mutex_unlock(poll->task_list_mutex);
        }
    }
}

extern int aw_stored_task_count_in_thread_poll(aw_thread_poll *poll){
    if (poll && poll->task_list) {
        return (int)poll->task_list->count;
    }
    return 0;
}

static void test_thread_func(int *x){
    aw_log("[d] test_thread_func p = %p, x = %d\n", x, *x);
}

static void test_thread_finish_cb(){
    aw_log("[d] thread is finished !!!!\n");
    
    //    aw_print_alloc_description();
}

extern void test_thread(){
    //    aw_uninit_debug_alloc();
    //    aw_init_debug_alloc();
    
    aw_thread_poll *th_poll = alloc_aw_thread_poll(5, 0);
    
    usleep(3*1000000);
    
    int idxes[] = {1,2,3,4,5,6,7,8,9,10};
    
    int i = 0;
    for (; i < 10; i++) {
        aw_add_task_to_thread_poll(th_poll, (aw_thread_func)test_thread_func, idxes + i);
    }
    
    aw_log("[d] start sleep 10 sec...\n");
    usleep(10 * 1000000);
    aw_log("[d] finish sleep 10 sec...\n");
    free_aw_thread_poll(&th_poll, test_thread_finish_cb);
    aw_log("[d] main thread finished\n");
}


