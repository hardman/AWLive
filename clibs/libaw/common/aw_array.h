/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#ifndef aw_array_h
#define aw_array_h

/*
 任意类型数组，可以存储任意类型的数据。包含了对数组数据的增删改查及排序
 */

#include <stdio.h>

typedef enum aw_array_element_type{
    AW_ARRAY_ELEMENT_TYPE_INT,
    AW_ARRAY_ELEMENT_TYPE_DOUBLE,
    AW_ARRAY_ELEMENT_TYPE_STRING,
    AW_ARRAY_ELEMENT_TYPE_POINTER,
    AW_ARRAY_ELEMENT_TYPE_RELEASE_POINTER,
} aw_array_element_type;

typedef struct aw_array_element{
    aw_array_element_type type;
    union{
        int32_t int_value;
        double double_value;
        void * pointer_value;
        char * string_value;
    };
    void (*free_pointer_func)(void *, int);
    int free_extra;
}aw_array_element;

extern aw_array_element *copy_aw_array_element(aw_array_element *);
extern aw_array_element *alloc_aw_array_element();
extern void free_aw_array_element(aw_array_element **);

typedef struct aw_array{
    aw_array_element **array_indexes;
    size_t alloc_size;
    size_t count;
}aw_array;

extern aw_array *alloc_aw_array(size_t);
extern void free_aw_array(aw_array **);
extern aw_array *move_aw_array(aw_array *array);

//基础功能
//插入元素
extern void aw_array_insert_element(aw_array **array, aw_array_element *ele, int index);
//尾部插入元素
extern void aw_array_add_element(aw_array **array, aw_array_element *ele);
//查找元素
extern aw_array_element *aw_array_element_at_index(aw_array *array, int index);
//移除元素
extern void aw_array_remove_element(aw_array *array, aw_array_element *element);
//根据下标移除元素
extern void aw_array_remove_element_at_index(aw_array *array, int index);
//获取指定元素的下标
extern int32_t aw_array_index_of_element(aw_array *array, aw_array_element *ele);
//元素交换
extern void aw_array_swap_element(aw_array *, int, int);

//具体功能函数
//加入一个int数据
extern aw_array_element *aw_array_add_int(aw_array **, int32_t);
//加入一个double数据
extern aw_array_element *aw_array_add_double(aw_array **, double);
//加入一个无需array管理释放的指针数据
extern aw_array_element *aw_array_add_pointer(aw_array **, void *);
//加入一个字符串数据，会copy一份新的
extern aw_array_element *aw_array_add_string(aw_array **, const char *);
//加入一个需要array管理释放的指针，但是要传入指定类型的释放该指针的方法，如果不传入释放方法，默认使用aw_free释放
extern aw_array_element *aw_array_add_release_pointer(aw_array **, void *, void (*)(void *, int), int);

//数组排序，实现了冒泡和快速排序
//比较函数结果
typedef enum aw_array_sort_compare_result {
    aw_array_sort_compare_result_great = -1,
    aw_array_sort_compare_result_less,
    aw_array_sort_compare_result_equal,
} aw_array_sort_compare_result;

//升序or降序排列
typedef enum aw_array_sort_policy{
    aw_array_sort_policy_ascending,
    aw_array_sort_policy_descending
}aw_array_sort_policy;

//排序数组元素比较函数
typedef aw_array_sort_compare_result(*aw_array_sort_compare_func) (aw_array_element* obj1, aw_array_element *obj2);

//排序函数指针
typedef void(*aw_array_sort_func)(aw_array *, aw_array_sort_policy, aw_array_sort_compare_func);

//冒泡排序
extern void aw_array_sort_bubble(aw_array *, aw_array_sort_policy, aw_array_sort_compare_func);

//快速排序
extern void aw_array_sort_quick(aw_array *, aw_array_sort_policy, aw_array_sort_compare_func);

//测试
extern void test_aw_array();

#endif /* aw_array_h */
