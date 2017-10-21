/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#include "aw_data.h"
#include <string.h>
#include <stdlib.h>
#include "aw_utils.h"

//record size

static int8_t aw_is_started_record_size = 0;
static uint32_t aw_recorded_size = 0;

static void aw_start_record_size(){
    if (aw_is_started_record_size) {
        AWLog("[ERROR] record size is already in use!!");
        return;
    }
    aw_is_started_record_size = 1;
    aw_recorded_size = 0;
}

static void aw_increase_record_size(size_t size){
    if (aw_is_started_record_size) {
        aw_recorded_size += size;
    }
}

static size_t aw_get_recorded_size(){
    return aw_recorded_size;
}

static void aw_end_record_size(){
    aw_is_started_record_size = 0;
    aw_recorded_size = 0;
}

#define AW_DATA_ALLOC_BLOCK 10 * 1024

void memcpy_aw_data(aw_data **tdata, const void *fdata, int size){
    if (!tdata) {
        return;
    }
    aw_data *tdatap = *tdata;
    if (!tdatap) {
        tdatap = alloc_aw_data(size);
        *tdata = tdatap;
    }
    int new_size = tdatap->alloc_size;
    while (new_size < tdatap->size + size) {
        new_size += AW_DATA_ALLOC_BLOCK;
    }
    if (new_size > tdatap->alloc_size) {
        extend_aw_data(tdata, new_size);
        tdatap = *tdata;
    }
    
    memcpy(tdatap->data + tdatap->curr_pos, fdata, size);
    tdatap->curr_pos += size;
    tdatap->size = tdatap->curr_pos;
}

aw_data * alloc_aw_data(int size){
    int awdatasize = sizeof(aw_data);
    aw_data *awdata = (aw_data *)aw_alloc(awdatasize);
    memset(awdata, 0, awdatasize);
    if (size == 0) {
        size = AW_DATA_ALLOC_BLOCK;
    }
    awdata->data = aw_alloc(size);
    awdata->alloc_size = size;
    return awdata;
}

extern aw_data * copy_aw_data(aw_data *data){
    if (!data) {
        return NULL;
    }
    aw_data *new_data = alloc_aw_data(data->alloc_size);
    memcpy_aw_data(&new_data, data->data, data->size);
    return new_data;
}

void extend_aw_data(aw_data **old_aw_data, int new_size){
    aw_data * new_aw_data = alloc_aw_data(new_size);
    memcpy_aw_data(&new_aw_data, (*old_aw_data)->data, (*old_aw_data)->size);
    free_aw_data(old_aw_data);
    *old_aw_data = new_aw_data;
}

extern void reset_aw_data(aw_data **data){
    aw_data *aw_data = *data;
    if (aw_data && aw_data->curr_pos > 0 && aw_data->size > 0) {
        aw_data->curr_pos = 0;
        memset(aw_data->data, 0, aw_data->size);
        aw_data->size = 0;
    }
}

void free_aw_data(aw_data **data){
    aw_data *aw_data = *data;
    aw_free(aw_data->data);
    aw_data->size = 0;
    aw_data->alloc_size = 0;
    aw_data->curr_pos = 0;
    aw_data->data = 0;
    aw_free(aw_data);
    *data = NULL;
}

//大小端转换
#define CONVERT_32(i, store, size) \
do{\
store = 0;\
for(int j = 0; j < size; j++){ \
store |=  (((i & (0xff << j * 8)) >> j * 8) & 0xff) << (((int)size - 1 - j) * 8); \
} \
}while(0)

//大小端不影响移位操作
#define CONVERT_64(i, store, size) \
do{ \
/*后面*/ \
uint32_t i1 = (uint32_t)i; \
uint32_t store1 = 0; \
CONVERT_32(i1, store1, 4); \
uint32_t *ipre = (uint32_t *)&store + 1; \
memcpy(ipre, &store1, 4); \
/*前面*/\
uint32_t i2 = i >> 32; \
uint32_t store2 = 0; \
CONVERT_32(i2, store2, 4); \
uint32_t *itail = (uint32_t *)&store; \
memcpy(itail, &store2, 4); \
}while(0)

#define CONVERT(i, store, size) \
do{ \
if(size < 8){ \
CONVERT_32(i, store, size); \
} else { \
CONVERT_64(i, store, size); \
} \
}while(0)

//读取数据
//读取单字节
#define READ_BYTE(type, i) \
size_t type_size = sizeof(type); \
memcpy(&i, awdata->data + awdata->curr_pos, type_size); \
awdata->curr_pos += type_size; \
aw_increase_record_size(type_size);

//读取多字节，系统字节序为小端
#define READ_MUTIBYTES_FOR_LITTLE_ENDIAN(i, type, size) \
do{ \
READ_MUTIBYTES_FOR_BIG_ENDIAN(i, type, size); \
type store = 0; \
CONVERT(i, store, size); \
i = store; \
}while(0);

//读取多字节，系统字节序为大端
#define READ_MUTIBYTES_FOR_BIG_ENDIAN(i, type, isize) \
do{\
memcpy(&i, awdata->data + awdata->curr_pos, isize); \
awdata->curr_pos += isize; \
aw_increase_record_size(isize); \
} while(0);

//读取多字节，自动判断大小端
#define READ_MUTIBYTES(i, type, size) \
do{\
if(is_little_endian()) {\
READ_MUTIBYTES_FOR_LITTLE_ENDIAN(i, type, size); \
}else{\
READ_MUTIBYTES_FOR_BIG_ENDIAN(i, type, size); \
}\
}while(0);

//读取多字节带返回值
#define RETURN_FOR_READ_MUTIBYTES(type) \
type i; \
READ_MUTIBYTES(i, type, sizeof(type)); \
return i;

static uint8_t is_little_endian(){
    union {
        int a;
        uint8_t b;
    }c;
    c.a = 1;
    return c.b == 1;
}

static void start_read(aw_data *awdata){
    awdata->curr_pos = 0;
}

static uint8_t read_uint8(aw_data *awdata){
    uint8_t i;
    READ_BYTE(uint8_t, i);
    return i;
}

static uint16_t read_uint16(aw_data *awdata){
    RETURN_FOR_READ_MUTIBYTES(uint16_t);
}

static uint32_t read_uint24(aw_data *awdata){
    uint32_t i;
    READ_MUTIBYTES(i, uint32_t, 3);
    return i;
}

static uint32_t read_uint32(aw_data *awdata){
    RETURN_FOR_READ_MUTIBYTES(uint32_t);
}

static uint64_t read_uint64(aw_data *awdata){
    RETURN_FOR_READ_MUTIBYTES(uint64_t);
}

static double read_double(aw_data *awdata){
    uint64_t i;
    READ_MUTIBYTES(i, uint64_t, sizeof(i));
    double d = 0;
    memcpy(&d, &i, sizeof(i));
    return d;
}

static void read_string(aw_data* awdata, char **string, int len){
    *string = aw_alloc(len);
    memset(*string, 0, len + 1);
    memcpy(*string, awdata->data + awdata->curr_pos, len);
    awdata->curr_pos += len;
    aw_increase_record_size(len);
}

static void read_bytes(aw_data * awdata, char **bytes, int len){
    *bytes = aw_alloc(len);
    memset(*bytes, 0, len);
    memcpy(*bytes, awdata->data + awdata->curr_pos, len);
    awdata->curr_pos += len;
    aw_increase_record_size(len);
}

static void skip_bytes(aw_data* awdata, int count){
    awdata->curr_pos += count;
    aw_increase_record_size(count);
}

static int remain_count(aw_data *awdata){
    return awdata->size - awdata->curr_pos;
}

//写入
static void write_uint8(aw_data **awdata, uint8_t v){
    memcpy_aw_data(awdata, &v, 1);
    aw_increase_record_size(1);
}

static void write_uint16(aw_data **awdata, uint16_t v){
    uint16_t big = v;
    if (is_little_endian()) {
        CONVERT(v, big, sizeof(v));
    }
    memcpy_aw_data(awdata, &big, sizeof(uint16_t));
    aw_increase_record_size(2);
}

static void write_uint24(aw_data **awdata, uint32_t v){
    uint32_t big = v;
    if (is_little_endian()) {
        CONVERT(v, big, 3);
    }
    memcpy_aw_data(awdata, &big, sizeof(uint8_t) * 3);
    aw_increase_record_size(3);
}

static void write_uint32(aw_data **awdata, uint32_t v){
    uint32_t big = v;
    if (is_little_endian()) {
        CONVERT(v, big, sizeof(v));
    }
    memcpy_aw_data(awdata, &big, sizeof(uint32_t));
    aw_increase_record_size(4);
}

static void write_uint64(aw_data **awdata, uint64_t v){
    uint64_t big = v;
    if (is_little_endian()) {
        CONVERT(v, big, sizeof(v));
    }
    memcpy_aw_data(awdata, &big, sizeof(uint64_t));
    aw_increase_record_size(8);
}

static void write_double(aw_data **awdata, double v){
    uint64_t newV = 0;
    memcpy(&newV, &v, sizeof(uint64_t));
    write_uint64(awdata, newV);
}

static void write_bytes(aw_data **awdata, const uint8_t *bytes, uint32_t count){
    memcpy_aw_data(awdata, bytes, (int)count);
    aw_increase_record_size(count);
}

static void write_string(aw_data **awdata, const char *str, uint32_t len_bytes_count){
    size_t str_len = strlen((const char *)str);
    switch (len_bytes_count) {
        case 1:
            write_uint8(awdata, (uint8_t) str_len);
            break;
        case 2:
            write_uint16(awdata, (uint16_t) str_len);
            break;
        case 4:
            write_uint32(awdata, (uint32_t) str_len);
            break;
        case 8:
            write_uint64(awdata, (uint64_t) str_len);
            break;
        default:
            break;
    }
    
    write_bytes(awdata, (const uint8_t *)str, (int)str_len);
}

static void write_empty_bytes(aw_data **awdata, uint32_t count){
    uint8_t *empty_bytes = (uint8_t *)aw_alloc(count);
    memset(empty_bytes, 0, count);
    write_bytes(awdata, empty_bytes, count);
}

//初始化 awdata operation
aw_data_reader data_reader = {
    .start_read = start_read,
    .read_uint8 = read_uint8,
    .read_uint16 = read_uint16,
    .read_uint24 = read_uint24,
    .read_uint32 = read_uint32,
    .read_uint64 = read_uint64,
    .read_double = read_double,
    .read_string = read_string,
    .read_bytes = read_bytes,
    .skip_bytes = skip_bytes,
    .remain_count = remain_count,
    //debug
    .start_record_size = aw_start_record_size,
    .record_size = aw_get_recorded_size,
    .end_record_size = aw_end_record_size,
};

aw_data_writer data_writer = {
    .write_uint8 = write_uint8,
    .write_uint16 = write_uint16,
    .write_uint24 = write_uint24,
    .write_uint32 = write_uint32,
    .write_uint64 = write_uint64,
    .write_double = write_double,
    .write_string = write_string,
    .write_bytes = write_bytes,
    .write_empty_bytes = write_empty_bytes,
    //debug
    .start_record_size = aw_start_record_size,
    .record_size = aw_get_recorded_size,
    .end_record_size = aw_end_record_size,
};

#define StringMark1(mark) #mark
#define StringMark(mark) StrintMark1(mark)

#define TWO 2

#define PRINT(n) AWLog("%s", StringMark(n))

#define TEST(type, funcname, value, printMark) \
wdata = alloc_aw_data(0); \
type funcname##1 = value; \
data_writer.write_##funcname(&wdata, funcname##1); \
data_reader.start_read(wdata); \
type funcname##2 = data_reader.read_##funcname(wdata); \
AWLog(#type".1 = %"printMark", u"#type".2 = %"printMark, funcname##1, funcname##2); \
free_aw_data(&wdata);

static void aw_data_test_convert(){
    uint8_t u8 = 0x12;
    uint8_t u81 = 0;
    CONVERT(u8, u81, 1);
    AWLog("u8=%x, u81=%x", u8, u81);
    
    uint16_t u16 = 0x1234;
    uint16_t u161 = 0;
    CONVERT_32(u16, u161, 2);
    AWLog("u16=%x, u161=%x", u16, u161);
    
    uint32_t u24 = 0x123456;
    uint32_t u241 = 0;
    CONVERT_32(u24, u241, 3);
    AWLog("u24=%x, u241=%x", u24, u241);
    
    uint32_t u32 = 0x12345678;
    uint32_t u321 = 0;
    CONVERT_32(u32, u321, 4);
    AWLog("u32=%x, u321=%x", u32, u321);
    
    uint64_t u64 = 0x12345678aabbccdd;
    uint64_t u641 = 0;
    CONVERT_64(u64, u641, 8);
    AWLog("u64=%llx, u641=%llx", u64, u641);
}

extern void aw_data_test(){
    AWLog("小端 ＝ %d", is_little_endian());
    
    aw_data_test_convert();
    aw_data *
    //    8字节
    TEST(uint8_t, uint8, 0xaa, "x");
    TEST(uint16_t, uint16, 0xaabb, "x");
    TEST(uint32_t, uint24, 0xabcdef, "x");
    TEST(uint32_t, uint32, 0x12345678, "x");
    TEST(uint64_t, uint64, 0x12345678aabbccdd, "llx");
    
    
    double d1 = 0.12345678;
    uint64_t di1 = 0;
    memcpy(&di1, &d1, sizeof(uint64_t));
    wdata = alloc_aw_data(0);
    data_writer.write_double(&wdata, d1);
    data_reader.start_read(wdata);
    double d2 = data_reader.read_double(wdata);
    uint64_t di2 = 0;
    memcpy(&di2, &d2, sizeof(uint64_t));
    AWLog("d.1 = %llx, d.2 = %llx \n", di1, di2);
    free_aw_data(&wdata);
}
