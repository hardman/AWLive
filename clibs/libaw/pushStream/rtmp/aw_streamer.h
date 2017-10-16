/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

/*
 rtmp流控制，包括开关，写入flv音/视频数据。
 编码好的flv数据通过此文件发送出去，客户端就可以播放了。
 */

#ifndef aw_streamer_h
#define aw_streamer_h

#include <stdio.h>
#include "aw_all.h"

//单例
//打开流
extern int8_t aw_streamer_open(const char *rtmp_url, aw_rtmp_state_changed_cb state_changed_cb);
//关闭流
extern void aw_streamer_close();

//是否正在streaming
extern int8_t aw_streamer_is_streaming();

//发送视频flv tag
extern void aw_streamer_send_video_data(aw_flv_video_tag *video_tag);
//发送音频flv tag
extern void aw_streamer_send_audio_data(aw_flv_audio_tag *audio_tag);

//发送sps pps
extern void aw_streamer_send_video_sps_pps_tag(aw_flv_video_tag *sps_pps_tag);
//发送 audio specific config
extern void aw_streamer_send_audio_specific_config_tag(aw_flv_audio_tag *asc_tag);

#endif /* aw_streamer_h */
