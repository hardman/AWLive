/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#import "AWAVConfig.h"

#include "aw_all.h"

@implementation AWAudioConfig
- (instancetype)init
{
    self = [super init];
    if (self) {
        self.bitrate = 100000;
        self.channelCount = 1;
        self.sampleSize = 16;
        self.sampleRate = 44100;
    }
    return self;
}

-(aw_faac_config)faacConfig{
    aw_faac_config faac_config;
    faac_config.bitrate = (int32_t)self.bitrate;
    faac_config.channel_count = (int32_t)self.channelCount;
    faac_config.sample_rate = (int32_t)self.sampleRate;
    faac_config.sample_size = (int32_t)self.sampleSize;
    return faac_config;
}

-(id)copyWithZone:(NSZone *)zone{
    AWAudioConfig *audioConfig = [[AWAudioConfig alloc] init];
    audioConfig.bitrate = self.bitrate;
    audioConfig.channelCount = self.channelCount;
    audioConfig.sampleRate = self.sampleRate;
    audioConfig.sampleSize = self.sampleSize;
    return audioConfig;
}

@end

@interface AWVideoConfig()
//推流宽高
@property (nonatomic, unsafe_unretained) NSInteger pushStreamWidth;
@property (nonatomic, unsafe_unretained) NSInteger pushStreamHeight;
@end

@implementation AWVideoConfig
- (instancetype)init
{
    self = [super init];
    if (self) {
        self.width = 540;
        self.height = 960;
        self.bitrate = 1000000;
        self.fps = 20;
        self.dataFormat = X264_CSP_NV12;
    }
    return self;
}

-(NSInteger)pushStreamWidth{
    if (self.shouldRotate) {
        return self.height;
    }
    return self.width;
}

-(NSInteger)pushStreamHeight{
    if (self.shouldRotate) {
        return self.width;
    }
    return self.height;
}

-(BOOL)shouldRotate{
    return UIInterfaceOrientationIsLandscape(self.orientation);
}

-(aw_x264_config) x264Config{
    aw_x264_config x264_config;
    x264_config.width = (int32_t)self.pushStreamWidth;
    x264_config.height = (int32_t)self.pushStreamHeight;
    x264_config.bitrate = (int32_t)self.bitrate;
    x264_config.fps = (int32_t)self.fps;
    x264_config.input_data_format = (int32_t)self.dataFormat;
    return x264_config;
}

-(id)copyWithZone:(NSZone *)zone{
    AWVideoConfig *videoConfig = [[AWVideoConfig alloc] init];
    videoConfig.bitrate = self.bitrate;
    videoConfig.fps = self.fps;
    videoConfig.dataFormat = self.dataFormat;
    videoConfig.orientation = self.orientation;
    videoConfig.width = self.width;
    videoConfig.height = self.height;
    return videoConfig;
}

@end
