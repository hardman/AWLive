/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#import "AWEncoderManager.h"
#import "AWSWFaacEncoder.h"
#import "AWSWX264Encoder.h"
#import "AWHWAACEncoder.h"
#import "AWHWH264Encoder.h"

@interface AWEncoderManager()
//编码器
@property (nonatomic, strong) AWVideoEncoder *videoEncoder;
@property (nonatomic, strong) AWAudioEncoder *audioEncoder;
@end

@implementation AWEncoderManager

-(void) openWithAudioConfig:(AWAudioConfig *) audioConfig videoConfig:(AWVideoConfig *) videoConfig{
    switch (self.audioEncoderType) {
        case AWAudioEncoderTypeHWAACLC:
            self.audioEncoder = [[AWHWAACEncoder alloc] init];
            break;
        case AWAudioEncoderTypeSWFAAC:
            self.audioEncoder = [[AWSWFaacEncoder alloc] init];
            break;
        default:
            NSLog(@"[E] AWEncoderManager.open please assin for audioEncoderType");
            return;
    }
    switch (self.videoEncoderType) {
        case AWVideoEncoderTypeHWH264:
            self.videoEncoder = [[AWHWH264Encoder alloc] init];
            break;
        case AWVideoEncoderTypeSWX264:
            self.videoEncoder = [[AWSWX264Encoder alloc] init];
            break;
        default:
            NSLog(@"[E] AWEncoderManager.open please assin for videoEncoderType");
            return;
    }
    
    self.audioEncoder.audioConfig = audioConfig;
    self.videoEncoder.videoConfig = videoConfig;
    
    self.audioEncoder.manager = self;
    self.videoEncoder.manager = self;
    
    [self.audioEncoder open];
    [self.videoEncoder open];
}

-(void)close{
    [self.audioEncoder close];
    [self.videoEncoder close];
    
    self.audioEncoder = nil;
    self.videoEncoder = nil;
    
    self.timestamp = 0;
    
    self.audioEncoder = AWAudioEncoderTypeNone;
    self.videoEncoder = AWVideoEncoderTypeNone;
}

@end
