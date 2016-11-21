/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://blog.csdn.net/hard_man/
 */

#import "AWHWH264Encoder.h"
#import <VideoToolbox/VideoToolbox.h>
#import "AWEncoderManager.h"

@interface AWHWH264Encoder()
@property (nonatomic, unsafe_unretained) VTCompressionSessionRef vEnSession;
@property (nonatomic, strong) dispatch_semaphore_t vSemaphore;
@property (nonatomic, strong) NSData *spsPpsData;
@property (nonatomic, strong) NSData *naluData;
@property (nonatomic, unsafe_unretained) BOOL isKeyFrame;

@end

@implementation AWHWH264Encoder

-(dispatch_semaphore_t)vSemaphore{
    if (!_vSemaphore) {
        _vSemaphore = dispatch_semaphore_create(0);
    }
    return _vSemaphore;
}

-(aw_flv_video_tag *)encodeYUVDataToFlvTag:(NSData *)yuvData{
    if (!_vEnSession) {
        return NULL;
    }
    //yuv 变成 转CVPixelBufferRef
    OSStatus status = noErr;
    
    size_t pixelWidth = self.videoConfig.width;
    size_t pixelHeight = self.videoConfig.height;
    CVPixelBufferRef pixelBuf = NULL;
    CVPixelBufferCreate(NULL, pixelWidth, pixelHeight, kCVPixelFormatType_420YpCbCr8BiPlanarFullRange, NULL, &pixelBuf);
    
    if(CVPixelBufferLockBaseAddress(pixelBuf, 0) != kCVReturnSuccess){
        [self onErrorWithCode:AWEncoderErrorCodeLockSampleBaseAddressFailed des:@"encode video lock base address failed"];
        return NULL;
    }
    
    //将yuv数据填充到CVPixelBufferRef中
    size_t y_size = pixelWidth * pixelHeight;
    size_t uv_size = y_size / 4;
    uint8_t *yuv_frame = (uint8_t *)yuvData.bytes;
    
    //处理y frame
    uint8_t *y_frame = CVPixelBufferGetBaseAddressOfPlane(pixelBuf, 0);
    memcpy(y_frame, yuv_frame, y_size);
    
    uint8_t *uv_frame = CVPixelBufferGetBaseAddressOfPlane(pixelBuf, 1);
    memcpy(uv_frame, yuv_frame + y_size, uv_size * 2);
    
    //硬编码 CmSampleBufRef
    
    uint32_t ptsMs = self.manager.timestamp + 1; //self.vFrameCount++ * 1000.f / self.videoConfig.fps;
    
    CMTime pts = CMTimeMake(ptsMs, 1000);
    
    status = VTCompressionSessionEncodeFrame(_vEnSession, pixelBuf, pts, kCMTimeInvalid, NULL, pixelBuf, NULL);
    
    if (status == noErr) {
        dispatch_semaphore_wait(self.vSemaphore, DISPATCH_TIME_FOREVER);
        if (_naluData) {
            const uint8_t nalu_header[] = {0,0,0,1};
            NSMutableData *mutableData = [NSMutableData dataWithBytes:nalu_header length:4];
            [mutableData appendData:_naluData];
            aw_flv_video_tag *video_tag = aw_encoder_create_video_tag((int8_t *)mutableData.bytes, mutableData.length, ptsMs, 0, self.isKeyFrame);
            
            //清除
            _naluData = nil;
            _isKeyFrame = NO;
            
            CVPixelBufferUnlockBaseAddress(pixelBuf, 0);
            
            CFRelease(pixelBuf);
            
            return video_tag;
        }
    }else{
        [self onErrorWithCode:AWEncoderErrorCodeEncodeVideoFrameFailed des:@"encode video frame error"];
    }
    CVPixelBufferUnlockBaseAddress(pixelBuf, 0);
    
    CFRelease(pixelBuf);
    
    return NULL;
}

-(aw_flv_video_tag *)createSpsPpsFlvTag{
    while(!self.spsPpsData) {
        dispatch_semaphore_wait(self.vSemaphore, DISPATCH_TIME_FOREVER);
    }
    aw_data *sps_pps_data = alloc_aw_data((uint32_t)self.spsPpsData.length);
    memcpy_aw_data(&sps_pps_data, (uint8_t *)self.spsPpsData.bytes, (uint32_t)self.spsPpsData.length);
    aw_flv_video_tag *sps_pps_tag = aw_encoder_create_sps_pps_tag(sps_pps_data);
    free_aw_data(&sps_pps_data);
    return sps_pps_tag;
}

static void vtCompressionSessionCallback (void * CM_NULLABLE outputCallbackRefCon,
                                          void * CM_NULLABLE sourceFrameRefCon,
                                          OSStatus status,
                                          VTEncodeInfoFlags infoFlags,
                                          CM_NULLABLE CMSampleBufferRef sampleBuffer ){
    AWHWH264Encoder *encoder = (__bridge AWHWH264Encoder *)(outputCallbackRefCon);
    if (status != noErr) {
        dispatch_semaphore_signal(encoder.vSemaphore);
        [encoder onErrorWithCode:AWEncoderErrorCodeEncodeVideoFrameFailed des:@"encode video frame error 1"];
        return;
    }
    
    if (!CMSampleBufferDataIsReady(sampleBuffer)) {
        dispatch_semaphore_signal(encoder.vSemaphore);
        [encoder onErrorWithCode:AWEncoderErrorCodeEncodeVideoFrameFailed des:@"encode video frame error 2"];
        return;
    }
    
    BOOL isKeyFrame = !CFDictionaryContainsKey( (CFArrayGetValueAtIndex(CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, true), 0)), kCMSampleAttachmentKey_NotSync);
    
    //首先获取sps 和pps
    BOOL needSpsPps = NO;
    if (!encoder.spsPpsData) {
        if (isKeyFrame) {
            CMFormatDescriptionRef sampleBufFormat = CMSampleBufferGetFormatDescription(sampleBuffer);
            size_t spsLen, spsCount;
            const uint8_t *sps;
            status = CMVideoFormatDescriptionGetH264ParameterSetAtIndex(sampleBufFormat, 0, &sps, &spsLen, &spsCount, 0 );
            if (status == noErr) {
                size_t ppsLen, ppsCount;
                const uint8_t *pps;
                OSStatus statusCode = CMVideoFormatDescriptionGetH264ParameterSetAtIndex(sampleBufFormat, 1, &pps, &ppsLen, &ppsCount, 0 );
                if (statusCode == noErr) {
                    aw_data *sps_pps_data = aw_create_sps_pps_data((uint8_t *)sps, (uint32_t)spsLen, (uint8_t *)pps, (uint32_t)ppsLen);
                    encoder.spsPpsData = [NSData dataWithBytes:sps_pps_data->data length:sps_pps_data->size];
                    free_aw_data(&sps_pps_data);
                }else{
                    [encoder onErrorWithCode:AWEncoderErrorCodeEncodeGetSpsPpsFailed des:@"got pps failed"];
                }
            }else{
                [encoder onErrorWithCode:AWEncoderErrorCodeEncodeGetSpsPpsFailed des:@"got sps failed"];
            }
        }
        needSpsPps = YES;
    }
    
    //获取编码后的h264数据
    CMBlockBufferRef blockBuffer = CMSampleBufferGetDataBuffer(sampleBuffer);
    size_t blockDataLen;
    uint8_t *blockData;
    status = CMBlockBufferGetDataPointer(blockBuffer, 0, NULL, &blockDataLen, (char **)&blockData);
    if (status == noErr) {
        size_t currReadPos = 0;
        //一般情况下都是只有1帧，在最开始编码的时候有2帧，取最后一帧
        while (currReadPos < blockDataLen - 4) {
            uint32_t naluLen = 0;
            memcpy(&naluLen, blockData + currReadPos, 4);
            naluLen = CFSwapInt32BigToHost(naluLen);
            
            encoder.naluData = [NSData dataWithBytes:blockData + currReadPos + 4 length:naluLen];
            
            currReadPos += 4 + naluLen;
            
            encoder.isKeyFrame = isKeyFrame;
        }
    }else{
        [encoder onErrorWithCode:AWEncoderErrorCodeEncodeGetH264DataFailed des:@"got h264 data failed"];
    }
    
    dispatch_semaphore_signal(encoder.vSemaphore);
    if (needSpsPps) {
        dispatch_semaphore_signal(encoder.vSemaphore);
    }
}

-(void)open{
    //创建 video encode session
    OSStatus status = VTCompressionSessionCreate(NULL, (int32_t)(self.videoConfig.width), (int32_t)self.videoConfig.height, kCMVideoCodecType_H264, NULL, NULL, NULL, vtCompressionSessionCallback, (__bridge void * _Nullable)(self), &_vEnSession);
    if (status == noErr) {
        VTSessionSetProperty(_vEnSession, kVTCompressionPropertyKey_ProfileLevel, kVTProfileLevel_H264_Main_AutoLevel);
        VTSessionSetProperty(_vEnSession, kVTCompressionPropertyKey_AverageBitRate, (__bridge CFTypeRef)@(self.videoConfig.bitrate));
        VTSessionSetProperty(_vEnSession, kVTCompressionPropertyKey_RealTime, kCFBooleanTrue);
        VTSessionSetProperty(_vEnSession, kVTCompressionPropertyKey_AllowFrameReordering, kCFBooleanFalse);
        VTSessionSetProperty(_vEnSession, kVTCompressionPropertyKey_MaxKeyFrameInterval, (__bridge CFTypeRef)@(0));
        
        //准备开始
        status = VTCompressionSessionPrepareToEncodeFrames(_vEnSession);
        if (status != noErr) {
            [self onErrorWithCode:AWEncoderErrorCodeVTSessionPrepareFailed des:@"硬编码vtsession prepare失败"];
        }
    }else{
        [self onErrorWithCode:AWEncoderErrorCodeVTSessionCreateFailed des:@"硬编码vtsession创建失败"];
    }
}

-(void)close{
    dispatch_semaphore_signal(self.vSemaphore);
    
    VTCompressionSessionInvalidate(_vEnSession);
    _vEnSession = nil;
    
    self.naluData = nil;
    self.isKeyFrame = NO;
    self.spsPpsData = nil;
}

@end
