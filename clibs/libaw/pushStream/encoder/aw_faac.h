/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

/*
 aac软编码器：faac封装。
 */

#ifndef aw_faac_h
#define aw_faac_h

#include <stdio.h>
#include "faac.h"
#include "faaccfg.h"
#include "aw_data.h"

typedef struct aw_faac_config {
    //采样率
    int sample_rate;
    
    //单个样本大小
    int sample_size;
    
    //比特率
    int bitrate;
    
    //声道
    int channel_count;
} aw_faac_config;

extern aw_faac_config *alloc_aw_faac_config();
extern void free_aw_faac_config(aw_faac_config **);

typedef struct aw_faac_context {
    aw_faac_config config;
    
    //编码器句柄
    faacEncHandle *faac_handler;
    
    //最大输入样本数
    unsigned long max_input_sample_count;
    unsigned long max_input_byte_count;
    
    //最大输出字节数
    unsigned long max_output_byte_count;
    
    //缓冲区
    int8_t *aac_buffer;
    
    aw_data *audio_specific_config_data;
    
    //保存的每一帧的数据
    aw_data *encoded_aac_data;
} aw_faac_context;

extern aw_faac_context * alloc_aw_faac_context(aw_faac_config);
extern void free_aw_faac_context(aw_faac_context **);

extern void aw_encode_pcm_frame_2_aac(aw_faac_context *ctx, int8_t *pcm_data, long len);

#endif /* aw_pcm2aac_h */
