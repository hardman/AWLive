/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#include "aw_array.h"
#include "aw_alloc.h"
#include <string.h>
#include "aw_utils.h"

#define AW_ARRAY_ALLOC_SIZE 100

extern aw_array_element *copy_aw_array_element(aw_array_element * ele){
    aw_array_element *new_ele = alloc_aw_array_element();
    memcpy(new_ele, ele, sizeof(aw_array_element));
    return new_ele;
}

extern aw_array_element *alloc_aw_array_element(){
    aw_array_element *array_ele = aw_alloc(sizeof(aw_array_element));
    memset(array_ele, 0, sizeof(aw_array_element));
    return array_ele;
}

extern void free_aw_array_element(aw_array_element **array_element){
    aw_array_element *ele = *array_element;
    switch (ele->type) {
        case AW_ARRAY_ELEMENT_TYPE_STRING:{
            if (ele->string_value) {
                aw_free(ele->string_value);
            }
            break;
        }
        case AW_ARRAY_ELEMENT_TYPE_RELEASE_POINTER: {
            if (ele->free_pointer_func) {
                ele->free_pointer_func(ele->pointer_value, ele->free_extra);
            }else{
                aw_free(ele->pointer_value);
            }
            break;
        }
        default:
            break;
    }
    
    aw_free(ele);
    *array_element = NULL;
}

extern aw_array *alloc_aw_array(size_t alloc_size){
    aw_array *array = aw_alloc(sizeof(aw_array));
    memset(array, 0, sizeof(aw_array));
    
    if (!alloc_size) {
        alloc_size = AW_ARRAY_ALLOC_SIZE;
    }
    
    array->array_indexes = aw_alloc(sizeof(aw_array_element *) * alloc_size);
    memset(array->array_indexes, 0, sizeof(aw_array_element *) * alloc_size);
    array->alloc_size = alloc_size;
    array->count = 0;
    
    return array;
}

extern void extend_aw_array(aw_array **array, size_t new_size){
    aw_array *now_array = *array;
    size_t now_size = now_array->alloc_size;
    if (!new_size) {
        now_size += AW_ARRAY_ALLOC_SIZE;
    }else{
        while (new_size >= now_size) {
            now_size += AW_ARRAY_ALLOC_SIZE;
        }
    }
    aw_array *new_array = alloc_aw_array(now_size);
    memcpy(new_array->array_indexes, now_array->array_indexes, now_array->alloc_size * sizeof(aw_array_element *));
    new_array->count = now_array->count;
    
    *array = new_array;
    
    //此时now_array的数组是浅copy到new_array中，所以释放now_array是不允许释放elements
    now_array->count = 0;
    free_aw_array(&now_array);
}

extern int8_t need_extend_aw_array(aw_array *array, int add_count){
    return array->count + add_count > array->alloc_size;
}

extern void free_aw_array(aw_array **array){
    aw_array *aw_array = *array;
    aw_array_element **elements = aw_array->array_indexes;
    size_t i = 0;
    for (; i < aw_array->count; i++) {
        free_aw_array_element(&elements[i]);
    }
    aw_free(aw_array->array_indexes);
    aw_free(aw_array);
    *array = NULL;
}

extern aw_array *move_aw_array(aw_array *array){
    aw_array *new_arr = aw_alloc(sizeof(aw_array));
    memcpy(new_arr, array, sizeof(aw_array));
    
    array->array_indexes = aw_alloc(sizeof(aw_array_element *) * array->alloc_size);
    memset(array->array_indexes, 0, sizeof(aw_array_element *) * array->alloc_size);
    array->count = 0;
    
    return new_arr;
}

extern void aw_array_insert_element(aw_array **array, aw_array_element *ele, int index){
    if (!array || !*array || !ele) {
        AWLog("[error] in aw_array_insert_element array or ele is NULL");
        return;
    }
    aw_array *aw_array = *array;
    if (need_extend_aw_array(aw_array, 1)) {
        extend_aw_array(array, aw_array->alloc_size + 1);
        aw_array = *array;
    }
    if (index >= aw_array->count) {
        aw_array->array_indexes[aw_array->count] = ele;
    }else {
        if(index < 0){
            index = 0;
        }
        memmove(aw_array->array_indexes + index + 1, aw_array->array_indexes + index, sizeof(aw_array_element *) * (aw_array->count - index));
        aw_array->array_indexes[index] = ele;
    }
    aw_array->count++;
}

extern void aw_array_add_element(aw_array **array, aw_array_element *ele){
    if (!array || !*array || !ele) {
        AWLog("[error] in aw_array_add_element array or ele is NULL");
        return;
    }
    aw_array_insert_element(array, ele, (int32_t)(*array)->count);
}

extern aw_array_element *aw_array_element_at_index(aw_array *array, int index){
    if (!array || index < 0 || index >= array->count) {
        AWLog("[error] in aw_array_element_at_index array is NULL or index out of range");
        return NULL;
    }
    return array->array_indexes[index];
}

extern void aw_array_remove_element(aw_array *array, aw_array_element *element){
    aw_array_element_at_index(array, aw_array_index_of_element(array, element));
}

extern void aw_array_remove_element_at_index(aw_array *array, int index){
    if (!array || index < 0 || index >= array->count) {
        AWLog("[error] in aw_array_remove_element_at_index array is NULL or index out of range");
        return;
    }
    aw_array_element *will_removed_ele = array->array_indexes[index];
    array->array_indexes[index] = NULL;
    if (index < array->count - 1) {
        memmove(array->array_indexes + index, array->array_indexes + index + 1, (array->count - 1 - index) * sizeof(aw_array_element *));
    }
    
    array->count--;
    
    free_aw_array_element(&will_removed_ele);
}

extern int32_t aw_array_index_of_element(aw_array *array, aw_array_element *ele){
    if (!array || !ele) {
        AWLog("[error] in aw_array_index_of_element array or element is NULL");
        return -1;
    }
    int i = 0;
    for (; i < (int)array->array_indexes; i++) {
        if (ele == array->array_indexes[i]) {
            return i;
        }
    }
    return -1;
}

extern aw_array_element *aw_array_add_int(aw_array **array, int32_t i){
    if (!array) {
        return NULL;
    }
    aw_array_element * ele = alloc_aw_array_element();
    ele->type = AW_ARRAY_ELEMENT_TYPE_INT;
    ele->int_value = i;
    
    aw_array_add_element(array, ele);
    
    return ele;
}

extern aw_array_element *aw_array_add_double(aw_array **array, double d){
    if (!array) {
        return NULL;
    }
    aw_array_element * ele = alloc_aw_array_element();
    ele->type = AW_ARRAY_ELEMENT_TYPE_DOUBLE;
    ele->double_value = d;
    
    aw_array_add_element(array, ele);
    
    return ele;
}

extern aw_array_element *aw_array_add_pointer(aw_array **array, void *pointer){
    if (!array) {
        return NULL;
    }
    aw_array_element * ele = alloc_aw_array_element();
    ele->type = AW_ARRAY_ELEMENT_TYPE_POINTER;
    ele->pointer_value = pointer;
    
    aw_array_add_element(array, ele);
    
    return ele;
}

extern aw_array_element *aw_array_add_string(aw_array **array, const char *str){
    if (!array) {
        return NULL;
    }
    aw_array_element * ele = alloc_aw_array_element();
    ele->type = AW_ARRAY_ELEMENT_TYPE_STRING;
    
    int32_t str_len = (int32_t)strlen(str);
    char *real_str = aw_alloc(str_len + 1);
    memset(real_str, 0, str_len + 1);
    memcpy(real_str, str, str_len);
    
    ele->string_value = real_str;
    
    aw_array_add_element(array, ele);
    
    return ele;
}

extern aw_array_element *aw_array_add_release_pointer(aw_array **array, void *pointer, void (*free_func)(void *, int), int free_extra){
    if (!array) {
        return NULL;
    }
    aw_array_element * ele = alloc_aw_array_element();
    ele->type = AW_ARRAY_ELEMENT_TYPE_RELEASE_POINTER;
    ele->pointer_value = pointer;
    ele->free_extra = free_extra;
    ele->free_pointer_func = free_func;
    
    aw_array_add_element(array, ele);
    
    return ele;
}

extern void aw_array_swap_element(aw_array *array, int index1, int index2){
    aw_array_element *ele1 = aw_array_element_at_index(array, index1);
    if (ele1) {
        aw_array_element *ele2 = aw_array_element_at_index(array, index2);
        if (ele2) {
            aw_array_element *tmp = ele1;
            array->array_indexes[index1] = ele2;
            array->array_indexes[index2] = tmp;
        }
    }
}

extern void aw_array_sort_bubble(aw_array *array, aw_array_sort_policy sort_policy, aw_array_sort_compare_func compare_func){
    int i = 0;
    for (; i < array->count; i++) {
        int j = i + 1;
        for (; j < array->count; j++) {
            switch(compare_func(aw_array_element_at_index(array, i), aw_array_element_at_index(array, j))) {
                case aw_array_sort_compare_result_great: {
                    if (sort_policy == aw_array_sort_policy_ascending) {
                        aw_array_swap_element(array, i, j);
                    }
                    break;
                }
                case aw_array_sort_compare_result_less: {
                    if (sort_policy == aw_array_sort_policy_descending) {
                        aw_array_swap_element(array, i, j);
                    }
                    break;
                }
                case aw_array_sort_compare_result_equal: {
                    // do nothing
                    break;
                }
            }
        }
    }
}

static void aw_array_sort_quick_inner(aw_array *array, int start, int end, aw_array_sort_policy sort_policy, aw_array_sort_compare_func compare_func){
    int len = end - start + 1;
    if (len <= 0) {
        return;
    }
    aw_array_element *key = aw_array_element_at_index(array, start);
    
    int i = start, j = end;
    while (j > i) {
        do{
            aw_array_element *end_ele = aw_array_element_at_index(array, j);
            aw_array_sort_compare_result compare_result = compare_func(key, end_ele);
            if(compare_result == aw_array_sort_compare_result_great && sort_policy == aw_array_sort_policy_ascending) {
                aw_array_swap_element(array, i, j);
                ++i;
                break;
            }else if(compare_result == aw_array_sort_compare_result_less && sort_policy == aw_array_sort_policy_descending) {
                aw_array_swap_element(array, i, j);
                ++i;
                break;
            }
        }while(--j > i);
        
        if (j <= i) {
            break;
        }
        
        do{
            aw_array_element *start_ele = aw_array_element_at_index(array, i);
            aw_array_sort_compare_result compare_result = compare_func(start_ele, key);
            if(compare_result == aw_array_sort_compare_result_great && sort_policy == aw_array_sort_policy_ascending) {
                aw_array_swap_element(array, i, j);
                --j;
                break;
            }else if(compare_result == aw_array_sort_compare_result_less&&sort_policy == aw_array_sort_policy_descending) {
                aw_array_swap_element(array, i, j);
                --j;
                break;
            }
        }while (++i < j);
    }
    
    aw_array_sort_quick_inner(array, start, i - 1, sort_policy, compare_func);
    aw_array_sort_quick_inner(array, i + 1, end, sort_policy, compare_func);
}

extern void aw_array_sort_quick(aw_array *array, aw_array_sort_policy sort_policy, aw_array_sort_compare_func compare_func){
    aw_array_sort_quick_inner(array, 0, (int32_t)array->count - 1, sort_policy, compare_func);
}

#include "aw_dict.h"

static aw_array_sort_compare_result test_aw_array_sort_compare_func(aw_array_element *element1, aw_array_element *element2){
    if (element1->int_value > element2->int_value) {
        return aw_array_sort_compare_result_great;
    }else if(element1->int_value == element2->int_value){
        return aw_array_sort_compare_result_equal;
    }else{
        return aw_array_sort_compare_result_less;
    }
}

static void test_aw_array_sort(aw_array_sort_func sort_func){
    aw_array *array = alloc_aw_array(0);
    int i = 0;
    int int_arr[10] = {5,6,2,7,8,3,4,1,9,0};
    for (; i < 10; i++) {
        int value = int_arr[i];
        aw_array_add_int(&array, value);
    }
    
    AWLog("排序前");
    for (i = 0; i < 10; i++) {
        AWLog("%d", aw_array_element_at_index(array, i)->int_value);
    }
    sort_func(array, aw_array_sort_policy_ascending, test_aw_array_sort_compare_func);
    
    AWLog("排序后 升序");
    for (i = 0; i < 10; i++) {
        AWLog("%d", aw_array_element_at_index(array, i)->int_value);
    }
    sort_func(array, aw_array_sort_policy_descending, test_aw_array_sort_compare_func);
    
    AWLog("排序后 降序");
    for (i = 0; i < 10; i++) {
        AWLog("%d", aw_array_element_at_index(array, i)->int_value);
    }
    
    free_aw_array(&array);
}

static void test_release_aw_dict(aw_dict *dict, int extra){
    free_aw_dict(&dict);
}

static void test_move_array(){
    aw_array *array = alloc_aw_array(1);
    const char *s = "test string";
    aw_array_add_string(&array, s);
    
    //test release pointer
    aw_dict *dict = alloc_aw_dict();
    int i = 0;
    for (; i < 10; i++) {
        aw_dict_set_int(dict, "x", i, 1);
    }
    aw_array_add_release_pointer(&array, dict, (void (*)(void *, int))test_release_aw_dict, 0);
    
    aw_array *new_arr = move_aw_array(array);
    
    free_aw_array(&array);
    free_aw_array(&new_arr);
}

extern void test_aw_array(){
    aw_uninit_debug_alloc();
    aw_init_debug_alloc();
    // test insert
    aw_array *array = alloc_aw_array(1);
    
    int i = 0;
    for (; i < 10; i++) {
        aw_array_add_int(&array, i);
    }
    
    i = 0;
    for (; i < array->count; i++) {
        AWLog(" - %d", aw_array_element_at_index(array, i)->int_value);
    }
    
    i = 0;
    for (; i < 5; i++) {
        aw_array_remove_element_at_index(array, i);
    }
    
    i = 0;
    for (; i < array->count; i++) {
        AWLog(" -- %d", aw_array_element_at_index(array, i)->int_value);
    }
    
    AWLog("test add and remove end");
    
    //test string
    const char *s = "test string";
    aw_array_add_string(&array, s);
    
    //test release pointer
    aw_dict *dict = alloc_aw_dict();
    i = 0;
    for (; i < 10; i++) {
        aw_dict_set_int(dict, "x", i, 1);
    }
    aw_array_add_release_pointer(&array, dict, (void (*)(void *, int))test_release_aw_dict, 0);
    
    aw_array_add_double(&array, 1.236);
    
    AWLog(" test 5(%s)", aw_array_element_at_index(array, 5)->string_value);
    
    aw_dict *get_dict = (aw_dict *)aw_array_element_at_index(array, 6)->pointer_value;
    i = 0;
    for (; i < 10; i++) {
        char xx[5];
        memset(xx, 0, 5);
        sprintf(xx, "x.%d", i);
        AWLog(" test 6-%d(%d)", i, aw_dict_get_int(get_dict, xx));
    }
    AWLog(" test 7(%f)", aw_array_element_at_index(array, 7)->double_value);
    
    free_aw_array(&array);
    
    //    test_aw_array_sort(aw_array_sort_bubble);
    test_aw_array_sort(aw_array_sort_quick);
    
    test_move_array();
    
    aw_print_alloc_description();
}
