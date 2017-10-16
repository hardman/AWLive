/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#ifndef aw_sw_faac_encoder_h
#define aw_sw_faac_encoder_h

/*
 使用faac进行软编码：aac软编码器。
 */

#include "aw_faac.h"
#include "aw_encode_flv.h"

//编码器开关
extern void aw_sw_encoder_open_faac_encoder(aw_faac_config *faac_config);
extern void aw_sw_encoder_close_faac_encoder();

//对pcm数据进行faac软编码，并转成flv_audio_tag
extern aw_flv_audio_tag *aw_sw_encoder_encode_faac_data(int8_t *pcm_data, long len, uint32_t timestamp);
//根据faac_config 创建包含audio specific config 的flv tag
extern aw_flv_audio_tag *aw_sw_encoder_create_faac_specific_config_tag();

//获取每帧输入样本数量 用来计算时间戳，除以样本率就是一帧的duration。
extern uint32_t aw_sw_faac_encoder_max_input_sample_count();

//编码器是否合法
extern int8_t aw_sw_faac_encoder_is_valid();

//下面2个函数所有编码器都可以用
//将aac数据转为flv_audio_tag
extern aw_flv_audio_tag *aw_encoder_create_audio_tag(int8_t *aac_data, long len, uint32_t timeStamp, aw_faac_config *faac_cfg);
//创建audio_specific_config_tag
extern aw_flv_audio_tag *aw_encoder_create_audio_specific_config_tag(aw_data *audio_specific_config_data, aw_faac_config *faac_config);


#endif /* aw_sw_audio_encoder_h */
