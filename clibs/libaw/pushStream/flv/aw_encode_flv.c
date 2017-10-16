/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#include "aw_encode_flv.h"
#include "aw_alloc.h"
#include <string.h>
#include "aw_utils.h"

extern aw_flv_script_tag *alloc_aw_flv_script_tag(){
    aw_flv_script_tag *script_tag = aw_alloc(sizeof(aw_flv_script_tag));
    memset(script_tag, 0, sizeof(aw_flv_script_tag));
    
    //初始化
    script_tag->common_tag.script_tag = script_tag;
    script_tag->common_tag.tag_type = aw_flv_tag_type_script;
    
    script_tag->v_codec_id = aw_flv_v_codec_id_H264;
    script_tag->a_codec_id = aw_flv_a_codec_id_AAC;
    
    //经计算，我们写入的script 的body size为255
    script_tag->common_tag.data_size = 255;
    
    return script_tag;
}

extern void free_aw_flv_script_tag(aw_flv_script_tag ** script_tag){
    aw_free(*script_tag);
    *script_tag = NULL;
}

extern aw_flv_audio_tag *alloc_aw_flv_audio_tag(){
    aw_flv_audio_tag *audio_tag = aw_alloc(sizeof(aw_flv_audio_tag));
    memset(audio_tag, 0, sizeof(aw_flv_audio_tag));
    //初始化
    audio_tag->common_tag.audio_tag = audio_tag;
    audio_tag->common_tag.tag_type = aw_flv_tag_type_audio;
    
    return audio_tag;
}

extern void free_aw_flv_audio_tag(aw_flv_audio_tag ** audio_tag){
    if ((*audio_tag)->config_record_data) {
        free_aw_data(&(*audio_tag)->config_record_data);
    }
    if((*audio_tag)->frame_data){
        free_aw_data(&(*audio_tag)->frame_data);
    }
    aw_free(*audio_tag);
    *audio_tag = NULL;
}

extern aw_flv_video_tag *alloc_aw_flv_video_tag(){
    aw_flv_video_tag *video_tag = aw_alloc(sizeof(aw_flv_video_tag));
    memset(video_tag, 0, sizeof(aw_flv_video_tag));
    //初始化
    video_tag->common_tag.video_tag = video_tag;
    video_tag->common_tag.tag_type = aw_flv_tag_type_video;
    
    return video_tag;
}

extern void free_aw_flv_video_tag(aw_flv_video_tag **video_tag){
    if ((*video_tag)->config_record_data) {
        free_aw_data(&(*video_tag)->config_record_data);
    }
    if((*video_tag)->frame_data){
        free_aw_data(&(*video_tag)->frame_data);
    }
    aw_free(*video_tag);
    *video_tag = NULL;
}

static void aw_write_tag_header(aw_data **flv_data, aw_flv_common_tag *common_tag){
    data_writer.write_uint8(flv_data, common_tag->tag_type);
    data_writer.write_uint24(flv_data, common_tag->data_size - 11);
    data_writer.write_uint24(flv_data, common_tag->timestamp);
    data_writer.write_uint8(flv_data, common_tag->timestamp_extend);
    data_writer.write_uint24(flv_data, common_tag->stream_id);
}

static void aw_write_audio_tag_body(aw_data **flv_data, aw_flv_audio_tag *audio_tag){
    uint8_t audio_header = 0;
    audio_header |= audio_tag->sound_format << 4 & 0xf0;
    audio_header |= audio_tag->sound_rate << 2 & 0xc;
    audio_header |= audio_tag->sound_size << 1 & 0x2;
    audio_header |= audio_tag->sound_type & 0x1;
    data_writer.write_uint8(flv_data, audio_header);
    
    if (audio_tag->sound_format == aw_flv_a_codec_id_AAC) {
        data_writer.write_uint8(flv_data, audio_tag->aac_packet_type);
    }
    switch (audio_tag->aac_packet_type) {
        case aw_flv_a_aac_package_type_aac_sequence_header: {
            data_writer.write_bytes(flv_data, audio_tag->config_record_data->data, audio_tag->config_record_data->size);
            break;
        }
        case aw_flv_a_aac_package_type_aac_raw: {
            data_writer.write_bytes(flv_data, audio_tag->frame_data->data, audio_tag->frame_data->size);
            break;
        }
    }
}

static void aw_write_video_tag_body(aw_data **flv_data, aw_flv_video_tag *video_tag){
    uint8_t video_header = 0;
    video_header |= video_tag->frame_type << 4 & 0xf0;
    video_header |= video_tag->codec_id & 0x0f;
    data_writer.write_uint8(flv_data, video_header);
    
    if (video_tag->codec_id == aw_flv_v_codec_id_H264) {
        data_writer.write_uint8(flv_data, video_tag->h264_package_type);
        data_writer.write_uint24(flv_data, video_tag->h264_composition_time);
    }
    
    switch (video_tag->h264_package_type) {
        case aw_flv_v_h264_packet_type_seq_header: {
            data_writer.write_bytes(flv_data, video_tag->config_record_data->data, video_tag->config_record_data->size);
            break;
        }
        case aw_flv_v_h264_packet_type_nalu: {
            data_writer.write_bytes(flv_data, video_tag->frame_data->data, video_tag->frame_data->size);
            break;
        }
        case aw_flv_v_h264_packet_type_end_of_seq: {
            //nothing
            break;
        }
    }
}

static void aw_write_script_tag_body(aw_data **flv_data, aw_flv_script_tag *script_tag){
    //纪录写入了多少字节
    data_writer.end_record_size();
    data_writer.start_record_size();
    
    //2表示类型，字符串
    data_writer.write_uint8(flv_data, 2);
    data_writer.write_string(flv_data, "onMetaData", 2);
    
    //数组类型：8
    data_writer.write_uint8(flv_data, 8);
    //数组长度：11
    data_writer.write_uint32(flv_data, 11);
    
    //28字节
    
    //写入duration 0表示double，1表示uint8
    data_writer.write_string(flv_data, "duration", 2);
    data_writer.write_uint8(flv_data, 0);
    data_writer.write_double(flv_data, script_tag->duration);
    //写入width
    data_writer.write_string(flv_data, "width", 2);
    data_writer.write_uint8(flv_data, 0);
    data_writer.write_double(flv_data, script_tag->width);
    //写入height
    data_writer.write_string(flv_data, "height", 2);
    data_writer.write_uint8(flv_data, 0);
    data_writer.write_double(flv_data, script_tag->height);
    //写入videodatarate
    data_writer.write_string(flv_data, "videodatarate", 2);
    data_writer.write_uint8(flv_data, 0);
    data_writer.write_double(flv_data, script_tag->video_data_rate);
    //写入framerate
    data_writer.write_string(flv_data, "framerate", 2);
    data_writer.write_uint8(flv_data, 0);
    data_writer.write_double(flv_data, script_tag->frame_rate);
    //写入videocodecid
    data_writer.write_string(flv_data, "videocodecid", 2);
    data_writer.write_uint8(flv_data, 0);
    data_writer.write_double(flv_data, script_tag->v_codec_id);
    //写入audiosamplerate
    data_writer.write_string(flv_data, "audiosamplerate", 2);
    data_writer.write_uint8(flv_data, 0);
    data_writer.write_double(flv_data, script_tag->a_sample_rate);
    //写入audiosamplesize
    data_writer.write_string(flv_data, "audiosamplesize", 2);
    data_writer.write_uint8(flv_data, 0);
    data_writer.write_double(flv_data, script_tag->a_sample_size);
    //写入stereo
    data_writer.write_string(flv_data, "stereo", 2);
    data_writer.write_uint8(flv_data, 1);
    data_writer.write_uint8(flv_data, script_tag->stereo);
    //写入a_codec_id
    data_writer.write_string(flv_data, "audiocodecid", 2);
    data_writer.write_uint8(flv_data, 0);
    data_writer.write_double(flv_data, script_tag->a_codec_id);
    //写入file_size
    data_writer.write_string(flv_data, "filesize", 2);
    data_writer.write_uint8(flv_data, 0);
    data_writer.write_double(flv_data, script_tag->file_size);
    
    //3字节的0x9表示metadata结束
    data_writer.write_uint24(flv_data, 9);
    
    //打印写入了多少字节
    AWLog("script tag body size = %ld", data_writer.record_size());
    data_writer.end_record_size();
}

static void aw_write_tag_body(aw_data **flv_data, aw_flv_common_tag *common_tag){
    switch (common_tag->tag_type) {
        case aw_flv_tag_type_audio: {
            aw_write_audio_tag_body(flv_data, common_tag->audio_tag);
            break;
        }
        case aw_flv_tag_type_video: {
            aw_write_video_tag_body(flv_data, common_tag->video_tag);
            break;
        }
        case aw_flv_tag_type_script: {
            aw_write_script_tag_body(flv_data, common_tag->script_tag);
            break;
        }
    }
}

static void aw_write_tag_data_size(aw_data **flv_data, aw_flv_common_tag *common_tag){
    data_writer.write_uint32(flv_data, common_tag->data_size);
}

extern void aw_write_flv_tag(aw_data **flv_data, aw_flv_common_tag *common_tag){
    aw_write_tag_header(flv_data, common_tag);
    aw_write_tag_body(flv_data, common_tag);
    aw_write_tag_data_size(flv_data, common_tag);
}

extern void aw_write_flv_header(aw_data **flv_data){
    uint8_t
    f = 'F', l = 'L', v = 'V',//FLV
    version = 1,//固定值
    av_flag = 5;//5表示av，5表示只有a，1表示只有v
    uint32_t flv_header_len = 9;
    data_writer.write_uint8(flv_data, f);
    data_writer.write_uint8(flv_data, l);
    data_writer.write_uint8(flv_data, v);
    data_writer.write_uint8(flv_data, version);
    data_writer.write_uint8(flv_data, av_flag);
    data_writer.write_uint32(flv_data, flv_header_len);
    
    //first previous tag size
    data_writer.write_uint32(flv_data, 0);
}
