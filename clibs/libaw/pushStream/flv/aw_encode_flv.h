/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

/*
 flv文件编码，此文件描述了一个完整flv文件的编码过程，只要有正确的h264和aac数据，就能够使用此文件函数合成正确的flv文件。
 */

#ifndef aw_encode_flv_h
#define aw_encode_flv_h

#include <stdio.h>
#include "aw_data.h"

//flv tag type
typedef enum aw_flv_tag_type {
    aw_flv_tag_type_audio = 8,
    aw_flv_tag_type_video = 9,
    aw_flv_tag_type_script = 18,
} aw_flv_tag_type;

//共6种(CodecID)，这里只用h264
typedef enum aw_flv_v_codec_id{
    aw_flv_v_codec_id_H263 = 2,
    aw_flv_v_codec_id_H264 = 7,
} aw_flv_v_codec_id;

//共13种(即sound format)，这里只用aac
typedef enum aw_flv_a_codec_id{
    aw_flv_a_codec_id_MP3 = 2,
    aw_flv_a_codec_id_AAC = 10,
} aw_flv_a_codec_id;

//sound size 8bit 16bit
typedef enum aw_flv_a_sound_size{
    aw_flv_a_sound_size_8_bit = 0,
    aw_flv_a_sound_size_16_bit = 1,
} aw_flv_a_sound_size;

//sound rate 5.5 11 22 44 kHz
typedef enum aw_flv_a_sound_rate{
    aw_flv_a_sound_rate_5_5kHZ = 0,
    aw_flv_a_sound_rate_11kHZ = 1,
    aw_flv_a_sound_rate_22kHZ = 2,
    aw_flv_a_sound_rate_44kHZ = 3,
} aw_flv_a_sound_rate;

//sound type mono/stereo
typedef enum aw_flv_a_sound_type{
    aw_flv_a_sound_type_mono = 0,
    aw_flv_a_sound_type_stereo = 1,
} aw_flv_a_sound_type;

//共5种
typedef enum aw_flv_v_frame_type{
    aw_flv_v_frame_type_key = 1,//关键帧
    aw_flv_v_frame_type_inner = 2,//非关键帧
}aw_flv_v_frame_type;

//h264 packet type
typedef enum aw_flv_v_h264_packet_type{
    aw_flv_v_h264_packet_type_seq_header = 0,
    aw_flv_v_h264_packet_type_nalu = 1,
    aw_flv_v_h264_packet_type_end_of_seq = 2,
}aw_flv_v_h264_packet_type;

typedef enum aw_flv_a_aac_packge_type{
    aw_flv_a_aac_package_type_aac_sequence_header = 0,
    aw_flv_a_aac_package_type_aac_raw = 1,
}aw_flv_a_aac_packge_type;

struct aw_flv_audio_tag;
struct aw_flv_video_tag;
struct aw_flv_script_tag;

// flv tags
typedef struct aw_flv_common_tag{
    aw_flv_tag_type tag_type;//tag类型，1字节
    //数据长度，3字节，
    // total tag size值为 sample_size + [script/audio/video]_tag_header_size + tag_header_size
    // tag header size = 11
    // [script/audio/video]_tag_header_size 会变化
    // tag body size = sample_size + [script/audio/video]_tag_header_size
    uint32_t data_size;//tag 总长度，包含11字节的 common_tag_header_size，包含自己的tag_header_size + tag_body_size;
    uint32_t header_size;//自己的tag header的长度
    uint32_t timestamp;//时间戳，3字节
    uint8_t timestamp_extend;//时间戳扩展位，1字节
    uint32_t stream_id;//总是0，3字节
    
    union{
        struct aw_flv_audio_tag *audio_tag;
        struct aw_flv_video_tag *video_tag;
        struct aw_flv_script_tag *script_tag;
    };
} aw_flv_common_tag;

typedef struct aw_flv_script_tag{
    aw_flv_common_tag common_tag;
    double duration;
    double width;
    double height;
    double video_data_rate;
    double frame_rate;
    double v_frame_rate;
    double a_frame_rate;
    double v_codec_id;
    double a_sample_rate;
    double a_sample_size;
    uint8_t stereo;
    double a_codec_id;
    double file_size;
} aw_flv_script_tag;

extern aw_flv_script_tag *alloc_aw_flv_script_tag();
extern void free_aw_flv_script_tag(aw_flv_script_tag **);

typedef struct aw_flv_audio_tag{
    aw_flv_common_tag common_tag;
    aw_flv_a_codec_id sound_format;//声音格式，4bit(0.5字节)
    aw_flv_a_sound_rate sound_rate;//声音频率，2bit
    aw_flv_a_sound_size sound_size;//声音尺寸, 1bit
    aw_flv_a_sound_type sound_type;//声音类型, 1bit
    aw_flv_a_aac_packge_type aac_packet_type;//aac 包类型，1字节，如果sound format==10(AAC)会有这个字段，否则没有。
    
    //aac sqeuence header，包含了aac转流所需的必备内容，需要作为第一个audio tag发送。
    aw_data *config_record_data;//audio config data
    
    aw_data *frame_data;//audio frame data
} aw_flv_audio_tag;

extern aw_flv_audio_tag *alloc_aw_flv_audio_tag();
extern void free_aw_flv_audio_tag(aw_flv_audio_tag **);

typedef struct aw_flv_video_tag{
    aw_flv_common_tag common_tag;
    aw_flv_v_frame_type frame_type;//帧类型，是否关键帧，4bit(0.5字节)
    aw_flv_v_codec_id codec_id;//编码器id，4bit
    
    //h264才有
    aw_flv_v_h264_packet_type h264_package_type;//h264包类型，1字节
    uint32_t h264_composition_time;//h264 时间调整(即cts，pts = dts + cts), 3字节
    
    //avc sequence header, 包含h264需要的重要信息(sps，pps)，需要在第一个videotag时发送
    aw_data *config_record_data;//video config data
    
    aw_data *frame_data;//video frame data
} aw_flv_video_tag;

extern aw_flv_video_tag *alloc_aw_flv_video_tag();
extern void free_aw_flv_video_tag(aw_flv_video_tag **);

//构造flv的方法
//写入header
extern void aw_write_flv_header(aw_data **flv_data);
//写入flv tag
extern void aw_write_flv_tag(aw_data **flv_data, aw_flv_common_tag *common_tag);

#endif /* aw_encode_flv_h */
