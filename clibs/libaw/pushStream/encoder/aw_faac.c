/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#include "aw_faac.h"
#include "aw_alloc.h"
#include "string.h"
#include "aw_utils.h"

static void aw_open_faac_enc_handler(aw_faac_context *faac_ctx){
    //开启faac
    faac_ctx->faac_handler = faacEncOpen(faac_ctx->config.sample_rate, faac_ctx->config.channel_count, &faac_ctx->max_input_sample_count, &faac_ctx->max_output_byte_count);
    
    faac_ctx->max_input_byte_count = faac_ctx->max_input_sample_count * faac_ctx->config.sample_size / 8;
    
    if(!faac_ctx->faac_handler){
        aw_log("[E] aac handler open failed");
        return;
    }
    
    //创建buffer
    faac_ctx->aac_buffer = aw_alloc(faac_ctx->max_output_byte_count);
    
    //获取配置
    faacEncConfigurationPtr faac_config = faacEncGetCurrentConfiguration(faac_ctx->faac_handler);
    if (faac_ctx->config.sample_size == 16) {
        faac_config->inputFormat = FAAC_INPUT_16BIT;
    }else if (faac_ctx->config.sample_size == 24) {
        faac_config->inputFormat = FAAC_INPUT_24BIT;
    }else if (faac_ctx->config.sample_size == 32) {
        faac_config->inputFormat = FAAC_INPUT_32BIT;
    }else{
        faac_config->inputFormat = FAAC_INPUT_FLOAT;
    }
    
    faac_config->aacObjectType = LOW;
    faac_config->mpegVersion = MPEG4;
    faac_config->useTns = 1;
    faac_config->allowMidside = 0;
    if(faac_ctx->config.bitrate){
        faac_config->bitRate = faac_ctx->config.bitrate / faac_ctx->config.channel_count;
    }
    
    faacEncSetConfiguration(faac_ctx->faac_handler, faac_config);
    
    //获取audio specific data
    uint8_t *audio_specific_data = NULL;
    unsigned long audio_specific_data_len = 0;
    faacEncGetDecoderSpecificInfo(faac_ctx->faac_handler, &audio_specific_data, &audio_specific_data_len);
    
    if (audio_specific_data_len > 0) {
        faac_ctx->audio_specific_config_data = alloc_aw_data(0);
        memcpy_aw_data(&faac_ctx->audio_specific_config_data, audio_specific_data, (uint32_t)audio_specific_data_len);
    }
    
}

extern void aw_encode_pcm_frame_2_aac(aw_faac_context *ctx, int8_t *pcm_data, long len){
    reset_aw_data(&ctx->encoded_aac_data);
    
    if (!pcm_data || len <= 0) {
        aw_log("[E] aw_encode_pcm_frame_2_aac params error");
        return;
    }
    //    aw_log("[d] faacEncEncode one frame begin");
    
    long max_input_count = ctx->max_input_byte_count;
    long curr_read_count = 0;
    
    do{
        long remain_count = len - curr_read_count;
        if (remain_count <= 0) {
            break;
        }
        long read_count = 0;
        if (remain_count > max_input_count) {
            read_count = max_input_count;
        }else{
            read_count = remain_count;
        }
        
        long input_samples = read_count * 8 / ctx->config.sample_size;
        int write_count = faacEncEncode(ctx->faac_handler, (int32_t * )(pcm_data + curr_read_count), (uint32_t)input_samples, (uint8_t *)ctx->aac_buffer, (uint32_t)ctx->max_output_byte_count);
        
        //        aw_log("[d] faacEncEncode write_count=%d read_count=%ld", write_count, read_count);
        
        if (write_count > 0) {
            data_writer.write_bytes(&ctx->encoded_aac_data, (const uint8_t *)ctx->aac_buffer, write_count);
        }
        
        curr_read_count += read_count;
    } while (curr_read_count + max_input_count < len);
    
    
    //    aw_log("[d] finish encode input count=%ld, encodec_count=%ld", len, curr_read_count);
    //    
    //    aw_log("[d] faacEncEncode one frame finish");
}

extern aw_faac_config *alloc_aw_faac_config(){
    aw_faac_config *faac_config = aw_alloc(sizeof(aw_faac_config));
    memset(faac_config, 0, sizeof(aw_faac_config));
    return faac_config;
}

extern void free_aw_faac_config(aw_faac_config **faac_config_p){
    aw_faac_config *faac_config = *faac_config_p;
    if (faac_config) {
        aw_free(faac_config);
    }
    *faac_config_p = NULL;
}

extern aw_faac_context * alloc_aw_faac_context(aw_faac_config config){
    aw_faac_context *faac_ctx = aw_alloc(sizeof(aw_faac_context));
    memset(faac_ctx, 0, sizeof(aw_faac_context));
    //config
    memcpy(&faac_ctx->config, &config, sizeof(aw_faac_config));
    
    aw_open_faac_enc_handler(faac_ctx);
    
    faac_ctx->encoded_aac_data = alloc_aw_data(0);
    
    return faac_ctx;
}

extern void free_aw_faac_context(aw_faac_context **context_p){
    aw_faac_context *context = *context_p;
    if (context) {
        //释放aacbuffer
        aw_free(context->aac_buffer);
        
        //关闭句柄
        faacEncClose(context->faac_handler);
        
        //释放awdata
        free_aw_data(&context->encoded_aac_data);
        
        //释放audio specific data
        if (context->audio_specific_config_data) {
            free_aw_data(&context->audio_specific_config_data);
        }
        
        aw_free(context);
    }
    *context_p = NULL;
}
