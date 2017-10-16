/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
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
    [self.capture sendAudioSampleBuffer:sampleBuffer];
}

-(void)newFrameReadyAtTime:(CMTime)frameTime atIndex:(NSInteger)textureIndex{
    [super newFrameReadyAtTime:frameTime atIndex:textureIndex];
    if(!self.capture || !self.capture.isCapturing){
        return;
    }
    //将bgra转为yuv
    //图像宽度
    int width = aw_stride((int)imageSize.width);
    //图像高度
    int height = imageSize.height;
    //宽*高
    int w_x_h = width * height;
    //yuv数据长度 = (宽 * 高) * 3 / 2
    int yuv_len = w_x_h * 3 / 2;
    
    //yuv数据
    uint8_t *yuv_bytes = malloc(yuv_len);
    
    //ARGBToNV12这个函数是libyuv这个第三方库提供的一个将bgra图片转为yuv420格式的一个函数。
    //libyuv是google提供的高性能的图片转码操作。支持大量关于图片的各种高效操作，是视频推流不可缺少的重要组件，你值得拥有。
    [self lockFramebufferForReading];
    ARGBToNV12(self.rawBytesForImage, width * 4, yuv_bytes, width, yuv_bytes + w_x_h, width, width, height);
    [self unlockFramebufferAfterReading];
    
    NSData *yuvData = [NSData dataWithBytesNoCopy:yuv_bytes length:yuv_len];
    
    [self.capture sendVideoYuvData:yuvData];
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
    _videoCamera = [[AWGPUImageVideoCamera alloc] initWithSessionPreset:self.captureSessionPreset cameraPosition:AVCaptureDevicePositionFront];
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
    _dataHandler = [[AWGPUImageAVCaptureDataHandler alloc] initWithImageSize:CGSizeMake(self.videoConfig.width, self.videoConfig.height) resultsInBGRAFormat:YES capture:self];
    [_beautifyFilter addTarget:_dataHandler];
    _videoCamera.awAudioDelegate = _dataHandler;
    
    [self.videoCamera startCameraCapture];
    
    [self updateFps:self.videoConfig.fps];
}

-(BOOL)startCaptureWithRtmpUrl:(NSString *)rtmpUrl{
    return [super startCaptureWithRtmpUrl:rtmpUrl];
}

-(void)switchCamera{
    [self.videoCamera rotateCamera];
    [self updateFps:self.videoConfig.fps];
}

-(void)onStartCapture{
}

-(void)onStopCapture{
}

-(void)dealloc{
    [self.videoCamera stopCameraCapture];
}

@end
