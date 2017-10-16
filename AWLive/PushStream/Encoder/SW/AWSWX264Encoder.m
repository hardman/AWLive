/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#import "AWSWX264Encoder.h"
#import "AWEncoderManager.h"

@implementation AWSWX264Encoder
-(aw_flv_video_tag *) encodeYUVDataToFlvTag:(NSData *)yuvData{
    return aw_sw_encoder_encode_x264_data((int8_t *)yuvData.bytes, yuvData.length, self.manager.timestamp + 1);
}

-(aw_flv_video_tag *)createSpsPpsFlvTag{
    return aw_sw_encoder_create_x264_sps_pps_tag();
}

-(void) open{
    aw_x264_config x264_config = self.videoConfig.x264Config;
    aw_sw_encoder_open_x264_encoder(&x264_config);
}

-(void)close{
    aw_sw_encoder_close_x264_encoder();
}

@end
