/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#import "AWSWFaacEncoder.h"
#import "AWEncoderManager.h"

@implementation AWSWFaacEncoder

-(aw_flv_audio_tag *) encodePCMDataToFlvTag:(NSData *)pcmData{
    self.manager.timestamp += aw_sw_faac_encoder_max_input_sample_count() * 1000 / self.audioConfig.sampleRate;
    return aw_sw_encoder_encode_faac_data((int8_t *)pcmData.bytes, pcmData.length, self.manager.timestamp);
}

-(aw_flv_audio_tag *)createAudioSpecificConfigFlvTag{
    return aw_sw_encoder_create_faac_specific_config_tag();
}

-(void) open{
    aw_faac_config faac_config = self.audioConfig.faacConfig;
    aw_sw_encoder_open_faac_encoder(&faac_config);
}

-(void)close{
    aw_sw_encoder_close_faac_encoder();
}
@end
