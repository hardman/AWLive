/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://blog.csdn.net/hard_man/
 */

#import "AWGPUImageAVCapture.h"
#import <GPUImage/GPUImageFramework.h>
#import "GPUImageBeautifyFilter.h"
#import "AWGPUImageVideoCamera.h"
#import "libyuv.h"

//GPUImage data handler
@interface AWGPUImageAVCaptureDataHandler : GPUImageRawDataOutput< AWGPUImageVideoCameraDelegate>
@property (nonatomic, weak) AWAVCapture *capture;
@end

@implementation AWGPUImageAVCaptureDataHandler

- (instancetype)initWithImageSize:(CGSize)newImageSize resultsInBGRAFormat:(BOOL)resultsInBGRAFormat capture:(AWAVCapture *)capture
{
    self = [super initWithImageSize:newImageSize resultsInBGRAFormat:resultsInBGRAFormat];
    if (self) {
        self.capture = capture;
    }
    return self;
}

-(void)processAudioSample:(CMSampleBufferRef)sampleBuffer{
    if(!self.capture || !self.capture.isCapturing){
        return;
    }
    [self.capture sendAudioSmapleBuffer:sampleBuffer toEncodeQueue:self.capture.encodeSampleQueue toSendQueue:self.capture.sendSampleQueue];
}

-(void)newFrameReadyAtTime:(CMTime)frameTime atIndex:(NSInteger)textureIndex{
    [super newFrameReadyAtTime:frameTime atIndex:textureIndex];
    if(!self.capture || !self.capture.isCapturing){
        return;
    }
    //将bgra转为yuv
    int width = imageSize.width;
    int height = imageSize.height;
    int w_x_h = width * height;
    int yuv_len = w_x_h * 3 / 2;
    
    uint8_t *yuv_bytes = malloc(yuv_len);
    
    [self lockFramebufferForReading];
    ARGBToI420(self.rawBytesForImage, width * 4, yuv_bytes, width, yuv_bytes + w_x_h, width / 2, yuv_bytes + w_x_h * 5 / 4, width / 2, width, height);
    [self unlockFramebufferAfterReading];
    
    NSData *yuvData = [NSData dataWithBytesNoCopy:yuv_bytes length:yuv_len];
    
    [self.capture sendVideoYuvData:yuvData toEncodeQueue:self.capture.encodeSampleQueue toSendQueue:self.capture.sendSampleQueue];
}

@end

//GPUImage capture
@interface AWGPUImageAVCapture()
@property (nonatomic, strong) AWGPUImageVideoCamera *videoCamera;
@property (nonatomic, strong) GPUImageView *gpuImageView;
@property (nonatomic, strong) GPUImageBeautifyFilter *beautifyFilter;
@property (nonatomic, strong) AWGPUImageAVCaptureDataHandler *dataHandler;
@end

@implementation AWGPUImageAVCapture

#pragma mark 懒加载

-(void)onInit{
    //摄像头
    _videoCamera = [[AWGPUImageVideoCamera alloc] initWithSessionPreset:AVCaptureSessionPreset640x480 cameraPosition:AVCaptureDevicePositionFront];
    //声音
    [_videoCamera addAudioInputsAndOutputs];
    //屏幕方向
    _videoCamera.outputImageOrientation = UIInterfaceOrientationPortrait;
    //镜像策略
    _videoCamera.horizontallyMirrorRearFacingCamera = NO;
    _videoCamera.horizontallyMirrorFrontFacingCamera = YES;
    
    //预览 view
    _gpuImageView = [[GPUImageView alloc] initWithFrame:self.preview.bounds];
    [self.preview addSubview:_gpuImageView];
    
    //美颜滤镜
    _beautifyFilter = [[GPUImageBeautifyFilter alloc] init];
    [_videoCamera addTarget:_beautifyFilter];
    
    //美颜滤镜
    [_beautifyFilter addTarget:_gpuImageView];
    
    //数据处理
    _dataHandler = [[AWGPUImageAVCaptureDataHandler alloc] initWithImageSize:CGSizeMake(480, 640) resultsInBGRAFormat:YES capture:self];
    [_beautifyFilter addTarget:_dataHandler];
    _videoCamera.awAudioDelegate = _dataHandler;
    
    [self.videoCamera startCameraCapture];
}

-(BOOL)initCaptureWithRtmpUrl:(NSString *)rtmpUrl andVideoConfig:(AWVideoConfig *)videoConfig andAudioConfig:(AWAudioConfig *)audioConfig{
    return [super initCaptureWithRtmpUrl:rtmpUrl andVideoConfig:videoConfig andAudioConfig:audioConfig];
}

-(void)switchCamera{
    [self.videoCamera rotateCamera];
}

-(void)onStartCapture{
}

-(void)onStopCapture{
}

-(void)dealloc{
    [self.videoCamera stopCameraCapture];
}

@end
