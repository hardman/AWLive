/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#include "aw_sw_x264_encoder.h"
#include "aw_all.h"

static aw_x264_context *s_x264_ctx = NULL;
static aw_x264_config *s_x264_config = NULL;

//debug使用
static int32_t video_count = 0;

//创建flv video tag
static aw_flv_video_tag *aw_sw_encoder_create_flv_video_tag(){
    aw_flv_video_tag *video_tag = alloc_aw_flv_video_tag();
    video_tag->codec_id = aw_flv_v_codec_id_H264;
    video_tag->common_tag.header_size = 5;
    return video_tag;
}

//将采集到的video yuv数据，编码为flv video tag
extern aw_flv_video_tag * aw_sw_encoder_encode_x264_data(int8_t *yuv_data, long len, uint32_t timeStamp){
    if (!aw_sw_x264_encoder_is_valid()) {
        aw_log("[E] aw_sw_encoder_encode_video_data when video encoder is not inited");
        return NULL;
    }
    
    aw_encode_yuv_frame_2_x264(s_x264_ctx, yuv_data, (int32_t)len);
    
    if (s_x264_ctx->encoded_h264_data->size <= 0) {
        return NULL;
    }
    
    x264_picture_t *pic_out = s_x264_ctx->pic_out;
    
    aw_flv_video_tag *video_tag = aw_encoder_create_video_tag((int8_t *)s_x264_ctx->encoded_h264_data->data, s_x264_ctx->encoded_h264_data->size, timeStamp, (uint32_t)((pic_out->i_pts - pic_out->i_dts) * 1000.0 / 90000), pic_out->b_keyframe);
    
    //    aw_log("----------------timestamp=%d, composition_time=%d, s_video_time_stamp=%d", video_tag->common_tag.timestamp, video_tag->h264_composition_time, s_video_time_stamp);
    
    video_count++;
    
    return video_tag;
}

//根据flv/h264/aac协议创建video/audio首帧tag
extern aw_flv_video_tag *aw_sw_encoder_create_x264_sps_pps_tag(){
    if(!aw_sw_x264_encoder_is_valid()){
        aw_log("[E] aw_sw_encoder_create_video_sps_pps_tag when video encoder is not inited");
        return NULL;
    }
    
    //创建 sps pps
    aw_flv_video_tag *sps_pps_tag = aw_sw_encoder_create_flv_video_tag();
    sps_pps_tag->frame_type = aw_flv_v_frame_type_key;
    sps_pps_tag->h264_package_type = aw_flv_v_h264_packet_type_seq_header;
    sps_pps_tag->h264_composition_time = 0;
    sps_pps_tag->config_record_data = copy_aw_data(s_x264_ctx->sps_pps_data);
    sps_pps_tag->common_tag.timestamp = 0;
    sps_pps_tag->common_tag.data_size = s_x264_ctx->sps_pps_data->size + 11 + sps_pps_tag->common_tag.header_size;
    return sps_pps_tag;
}

//打开编码器
extern void aw_sw_encoder_open_x264_encoder(aw_x264_config *x264_config){
    if (aw_sw_x264_encoder_is_valid()) {
        aw_log("[E] aw_sw_encoder_open_video_encoder when video encoder is not inited");
        return;
    }
    
    int32_t x264_cfg_len = sizeof(aw_x264_config);
    if (!s_x264_config) {
        s_x264_config = aw_alloc(x264_cfg_len);
    }
    memcpy(s_x264_config, x264_config, x264_cfg_len);
    
    s_x264_ctx = alloc_aw_x264_context(*x264_config);
}

//关闭编码器
extern void aw_sw_encoder_close_x264_encoder(){
    if (!aw_sw_x264_encoder_is_valid()) {
        aw_log("[E] aw_sw_encoder_close_video_encoder s_faac_ctx is NULL");
        return;
    }
    
    if (s_x264_config) {
        aw_free(s_x264_config);
        s_x264_config = NULL;
    }
    
    free_aw_x264_context(&s_x264_ctx);
}

//是否video encoder 已经创建
extern int8_t aw_sw_x264_encoder_is_valid(){
    return s_x264_ctx != NULL;
}

//创建 flv tag 跟编码无关。
//将h264数据转为flv_video_tag
extern aw_flv_video_tag *aw_encoder_create_video_tag(int8_t *h264_data, long len, uint32_t timeStamp, long composition_time, int8_t is_key_frame){
    aw_flv_video_tag *video_tag = aw_sw_encoder_create_flv_video_tag();
    video_tag->frame_type = is_key_frame;
    video_tag->h264_package_type = aw_flv_v_h264_packet_type_nalu;
    
    video_tag->h264_composition_time = (uint32_t)(composition_time);
    video_tag->common_tag.timestamp = timeStamp;
    aw_data *frame_data = alloc_aw_data((uint32_t)len);
    memcpy(frame_data->data, h264_data, len);
    frame_data->size = (uint32_t)len;
    video_tag->frame_data = frame_data;
    video_tag->common_tag.data_size = video_tag->frame_data->size + 11 + video_tag->common_tag.header_size;
    return video_tag;
}

//创建sps_pps_tag
extern aw_flv_video_tag *aw_encoder_create_sps_pps_tag(aw_data *sps_pps_data){
    //创建 sps pps
    aw_flv_video_tag *sps_pps_tag = aw_sw_encoder_create_flv_video_tag();
    sps_pps_tag->frame_type = aw_flv_v_frame_type_key;
    sps_pps_tag->h264_package_type = aw_flv_v_h264_packet_type_seq_header;
    sps_pps_tag->h264_composition_time = 0;
    sps_pps_tag->config_record_data = copy_aw_data(sps_pps_data);
    sps_pps_tag->common_tag.timestamp = 0;
    sps_pps_tag->common_tag.data_size = sps_pps_data->size + 11 + sps_pps_tag->common_tag.header_size;
    return sps_pps_tag;
}
