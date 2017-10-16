/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#include "aw_streamer.h"
#include "aw_utils.h"
#include "aw_rtmp.h"
#include "aw_encode_flv.h"
#include <unistd.h>
#include <inttypes.h>
#include "aw_thread_poll.h"
#include "aw_array.h"

//open stream 使用的变量
static aw_rtmp_context *s_rtmp_ctx = NULL;
static aw_data *s_output_buf = NULL;
static aw_rtmp_state_changed_cb s_state_changed_cb = NULL;
static char *s_rtmp_url = NULL;

//rtmp
static int8_t aw_streamer_is_rtmp_valid();
static void aw_streamer_send_flv_tag_to_rtmp(aw_flv_common_tag *common_tag);
static void aw_streamer_rtmp_state_changed_callback(aw_rtmp_state old_state, aw_rtmp_state new_state);

//rtmp
static int8_t aw_steamer_open_rtmp_context();
static void aw_streamer_close_rtmp_context();

//video-----

extern void aw_streamer_send_video_data(aw_flv_video_tag *video_tag){
    if (!aw_streamer_is_rtmp_valid()) {
        aw_log("[E] aw_streamer_send_video_data s_rtmp_ctx is NULL");
        free_aw_flv_video_tag(&video_tag);
        return;
    }
    
    //    aw_log("[D] aw_streamer_send_video_data timestamp=%u, compTime=%u", video_tag->common_tag.timestamp, video_tag->h264_composition_time);
    
    aw_streamer_send_flv_tag_to_rtmp(&video_tag->common_tag);
}

extern void aw_streamer_send_video_sps_pps_tag(aw_flv_video_tag *sps_pps_tag){
    if (!aw_streamer_is_rtmp_valid()) {
        aw_log("[E] aw_streamer_send_video_sps_pps_tag when rtmp is not valid");
        free_aw_flv_video_tag(&sps_pps_tag);
        return;
    }
    
    if (s_rtmp_ctx->is_header_sent) {
        free_aw_flv_video_tag(&sps_pps_tag);
        return;
    }
    
    //发送 video sps pps
    aw_streamer_send_flv_tag_to_rtmp(&sps_pps_tag->common_tag);
}

//audio------

extern void aw_streamer_send_audio_specific_config_tag(aw_flv_audio_tag *asc_tag){
    if (!aw_streamer_is_rtmp_valid()) {
        aw_log("[E] aw_streamer_send_audio_specific_config_tag when rtmp is not valid");
        free_aw_flv_audio_tag(&asc_tag);
        return;
    }
    
    if (s_rtmp_ctx->is_header_sent) {
        free_aw_flv_audio_tag(&asc_tag);
        return;
    }
    
    //发送 audio specific config
    aw_streamer_send_flv_tag_to_rtmp(&asc_tag->common_tag);
}

extern void aw_streamer_send_audio_data(aw_flv_audio_tag *audio_tag){
    if (!aw_streamer_is_rtmp_valid()) {
        aw_log("[E] aw_streamer_send_audio_specific_config_tag when rtmp is not valid");
        free_aw_flv_audio_tag(&audio_tag);
        return;
    }
    
    aw_streamer_send_flv_tag_to_rtmp(&audio_tag->common_tag);
}

//rtmp------

static void aw_streamer_send_flv_tag_to_rtmp(aw_flv_common_tag *common_tag){
    if (common_tag) {
        aw_write_flv_tag(&s_output_buf, common_tag);
        switch (common_tag->tag_type) {
            case aw_flv_tag_type_audio: {
                free_aw_flv_audio_tag(&common_tag->audio_tag);
                break;
            }
            case aw_flv_tag_type_video: {
                free_aw_flv_video_tag(&common_tag->video_tag);
                break;
            }
            case aw_flv_tag_type_script: {
                free_aw_flv_script_tag(&common_tag->script_tag);
                break;
            }
        }
    }
    
    if (!aw_streamer_is_streaming()) {
        return;
    }
    
    if (s_output_buf->size <= 0) {
        return;
    }
    
    //    int nRet =
    aw_rtmp_write(s_rtmp_ctx, (const char *)s_output_buf->data, s_output_buf->size);
    
    //    aw_log("[d] send flv tag size=%d sended_size=%d", s_output_buf->size, nRet);
    
    reset_aw_data(&s_output_buf);
}

static int8_t aw_streamer_is_rtmp_valid(){
    return s_rtmp_ctx != NULL;
}

extern int8_t aw_streamer_is_streaming(){
    //    return aw_streamer_is_rtmp_valid() && aw_streamer_is_video_valid() && aw_streamer_is_audio_valid() && aw_is_rtmp_opened(s_rtmp_ctx);
    return aw_streamer_is_rtmp_valid() && aw_is_rtmp_opened(s_rtmp_ctx);
}

static void aw_streamer_rtmp_state_changed_callback(aw_rtmp_state old_state, aw_rtmp_state new_state){
    if(new_state == aw_rtmp_state_connected){
        //打开rtmp 先发送 配置tag
        //        aw_streamer_send_video_sps_pps_tag();
        //        aw_streamer_send_audio_specific_config_tag();
    }else if(new_state == aw_rtmp_state_error_open){
        aw_streamer_close_rtmp_context();
    }
    
    if (s_state_changed_cb) {
        s_state_changed_cb(old_state, new_state);
    }
}

static int8_t aw_steamer_open_rtmp_context(){
    if (!s_rtmp_ctx) {
        s_rtmp_ctx = alloc_aw_rtmp_context(s_rtmp_url, aw_streamer_rtmp_state_changed_callback);
    }
    return aw_rtmp_open(s_rtmp_ctx);
}

static void aw_streamer_close_rtmp_context(){
    if (s_rtmp_ctx) {
        aw_rtmp_close(s_rtmp_ctx);
    }
    aw_log("[d] closed rtmp context");
}

//创建 outbuf
static void aw_streamer_create_out_buf(){
    if (s_output_buf) {
        aw_log("[E] aw_streamer_open_encoder s_out_buf is already exist");
        return;
    }
    s_output_buf = alloc_aw_data(0);
}

//释放 outbuf
static void aw_streamer_release_out_buf(){
    if (!s_output_buf) {
        aw_log("[E] aw_streamer_open_encoder s_out_buf is already free");
        return;
    }
    free_aw_data(&s_output_buf);
}

extern int8_t aw_streamer_open(const char *rtmp_url, aw_rtmp_state_changed_cb state_changed_cb){
    if (aw_streamer_is_rtmp_valid()) {
        aw_log("[E] aw_streamer_open_rtmp s_rtmp_ctx is already open");
        return -1;
    }
    
    // 调试 mem leak
    //    aw_uninit_debug_alloc();
    //    aw_init_debug_alloc();
    
    //创建outbuf
    aw_streamer_create_out_buf();
    
    s_state_changed_cb = state_changed_cb;
    
    int32_t rtmp_url_len = (int32_t)strlen(rtmp_url);
    if (!s_rtmp_url) {
        s_rtmp_url = aw_alloc(rtmp_url_len + 1);
    }
    memcpy(s_rtmp_url, rtmp_url, rtmp_url_len);
    
    return aw_steamer_open_rtmp_context();
}

extern void aw_streamer_close(){
    if (!aw_streamer_is_rtmp_valid()) {
        aw_log("[E] aw_streamer_close s_rtmp_ctx is NULL");
        return;
    }
    
    //关闭rtmp
    aw_streamer_close_rtmp_context();
    
    //释放 s_rtmp_ctx;
    free_aw_rtmp_context(&s_rtmp_ctx);
    
    s_state_changed_cb = NULL;
    
    if (s_rtmp_url) {
        aw_free(s_rtmp_url);
        s_rtmp_url = NULL;
    }
    
    //释放outbuf
    aw_streamer_release_out_buf();
    
    //调试mem leak
    //    aw_print_alloc_description();
}
