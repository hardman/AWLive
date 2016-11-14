/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://blog.csdn.net/hard_man/
 */

#import "AWGPUImageVideoCamera.h"

@implementation AWGPUImageVideoCamera

-(void)processAudioSampleBuffer:(CMSampleBufferRef)sampleBuffer{
    [super processAudioSampleBuffer:sampleBuffer];
    [self.awAudioDelegate processAudioSample:sampleBuffer];
}

-(void)setCaptureSessionPreset:(NSString *)captureSessionPreset{
    if (!_captureSession || ![_captureSession canSetSessionPreset:captureSessionPreset]) {
        @throw [NSException exceptionWithName:@"Not supported captureSessionPreset" reason:[NSString stringWithFormat:@"captureSessionPreset is [%@]", captureSessionPreset] userInfo:nil];
        return;
    }
    [super setCaptureSessionPreset:captureSessionPreset];
}

@end
