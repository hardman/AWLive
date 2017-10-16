/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#import "AWHWH264Encoder.h"
#import <VideoToolbox/VideoToolbox.h>
#import "AWEncoderManager.h"

@interface AWHWH264Encoder()
@property (nonatomic, unsafe_unretained) VTCompressionSessionRef vEnSession;
@property (nonatomic, strong) dispatch_semaphore_t vSemaphore;
@property (nonatomic, copy) NSData *spsPpsData;
@property (nonatomic, copy) NSData *naluData;
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
    
    //视频宽度
    size_t pixelWidth = self.videoConfig.pushStreamWidth;
    //视频高度
    size_t pixelHeight = self.videoConfig.pushStreamHeight;
    
    //现在要把NV12数据放入 CVPixelBufferRef中，因为 硬编码主要调用VTCompressionSessionEncodeFrame函数，此函数不接受yuv数据，但是接受CVPixelBufferRef类型。
    CVPixelBufferRef pixelBuf = NULL;
    //初始化pixelBuf，数据类型是kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange，此类型数据格式同NV12格式相同。
    CVPixelBufferCreate(NULL, pixelWidth, pixelHeight, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange, NULL, &pixelBuf);
    
    // Lock address，锁定数据，应该是多线程防止重入操作。
    if(CVPixelBufferLockBaseAddress(pixelBuf, 0) != kCVReturnSuccess){
        [self onErrorWithCode:AWEncoderErrorCodeLockSampleBaseAddressFailed des:@"encode video lock base address failed"];
        return NULL;
    }
    
    //将yuv数据填充到CVPixelBufferRef中
    size_t y_size = aw_stride(pixelWidth) * pixelHeight;
    size_t uv_size = y_size / 4;
    uint8_t *yuv_frame = (uint8_t *)yuvData.bytes;
    
    //处理y frame
    uint8_t *y_frame = CVPixelBufferGetBaseAddressOfPlane(pixelBuf, 0);
    memcpy(y_frame, yuv_frame, y_size);
    
    uint8_t *uv_frame = CVPixelBufferGetBaseAddressOfPlane(pixelBuf, 1);
    memcpy(uv_frame, yuv_frame + y_size, uv_size * 2);
    
    //硬编码 CmSampleBufRef
    
    //时间戳
    uint32_t ptsMs = self.manager.timestamp + 1; //self.vFrameCount++ * 1000.f / self.videoConfig.fps;
    
    CMTime pts = CMTimeMake(ptsMs, 1000);
    
    //硬编码主要其实就这一句。将携带NV12数据的PixelBuf送到硬编码器中，进行编码。
    status = VTCompressionSessionEncodeFrame(_vEnSession, pixelBuf, pts, kCMTimeInvalid, NULL, pixelBuf, NULL);
    
    if (status == noErr) {
        dispatch_semaphore_wait(self.vSemaphore, DISPATCH_TIME_FOREVER);
        if (_naluData) {
            //此处 硬编码成功，_naluData内的数据即为h264视频帧。
            //我们是推流，所以获取帧长度，转成大端字节序，放到数据的最前面
            uint32_t naluLen = (uint32_t)_naluData.length;
            //小端转大端。计算机内一般都是小端，而网络和文件中一般都是大端。大端转小端和小端转大端算法一样，就是字节序反转就行了。
            uint8_t naluLenArr[4] = {naluLen >> 24 & 0xff, naluLen >> 16 & 0xff, naluLen >> 8 & 0xff, naluLen & 0xff};
            //将数据拼在一起
            NSMutableData *mutableData = [NSMutableData dataWithBytes:naluLenArr length:4];
            [mutableData appendData:_naluData];
            
            //将h264数据合成flv tag，合成flvtag之后就可以直接发送到服务端了。后续会介绍
            aw_flv_video_tag *video_tag = aw_encoder_create_video_tag((int8_t *)mutableData.bytes, mutableData.length, ptsMs, 0, self.isKeyFrame);
            
            //到此，编码工作完成，清除状态。
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
    //通过outputCallbackRefCon获取AWHWH264Encoder的对象指针，将编码好的h264数据传出去。
    AWHWH264Encoder *encoder = (__bridge AWHWH264Encoder *)(outputCallbackRefCon);
    
    //判断是否编码成功
    if (status != noErr) {
        dispatch_semaphore_signal(encoder.vSemaphore);
        [encoder onErrorWithCode:AWEncoderErrorCodeEncodeVideoFrameFailed des:@"encode video frame error 1"];
        return;
    }
    
    //是否数据是完整的
    if (!CMSampleBufferDataIsReady(sampleBuffer)) {
        dispatch_semaphore_signal(encoder.vSemaphore);
        [encoder onErrorWithCode:AWEncoderErrorCodeEncodeVideoFrameFailed des:@"encode video frame error 2"];
        return;
    }
    
    //是否是关键帧，关键帧和非关键帧要区分清楚。推流时也要注明。
    BOOL isKeyFrame = !CFDictionaryContainsKey( (CFArrayGetValueAtIndex(CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, true), 0)), kCMSampleAttachmentKey_NotSync);
    
    //首先获取sps 和pps
    //sps pss 也是h264的一部分，可以认为它们是特别的h264视频帧，保存了h264视频的一些必要信息。
    //没有这部分数据h264视频很难解析出来。
    //数据处理时，sps pps 数据可以作为一个普通h264帧，放在h264视频流的最前面。
    BOOL needSpsPps = NO;
    if (!encoder.spsPpsData) {
        if (isKeyFrame) {
            //获取avcC，这就是我们想要的sps和pps数据。
            //如果保存到文件中，需要将此数据前加上 [0 0 0 1] 4个字节，写入到h264文件的最前面。
            //如果推流，将此数据放入flv数据区即可。
            CMFormatDescriptionRef sampleBufFormat = CMSampleBufferGetFormatDescription(sampleBuffer);
            NSDictionary *dict = (__bridge NSDictionary *)CMFormatDescriptionGetExtensions(sampleBufFormat);
            encoder.spsPpsData = dict[@"SampleDescriptionExtensionAtoms"][@"avcC"];
        }
        needSpsPps = YES;
    }
    
    //获取真正的视频帧数据
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
            
            //naluData 即为一帧h264数据。
            //如果保存到文件中，需要将此数据前加上 [0 0 0 1] 4个字节，按顺序写入到h264文件中。
            //如果推流，需要将此数据前加上4个字节表示数据长度的数字，此数据需转为大端字节序。
            //关于大端和小端模式，请参考此网址：http://blog.csdn.net/hackbuteer1/article/details/7722667
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
    // 创建 video encode session
    // 传入视频宽高，编码类型：kCMVideoCodecType_H264
    // 编码回调：vtCompressionSessionCallback，这个回调函数为编码结果回调，编码成功后，会将数据传入此回调中。
    // (__bridge void * _Nullable)(self)：这个参数会被原封不动地传入vtCompressionSessionCallback中，此参数为编码回调同外界通信的唯一参数。
    // &_vEnSession，c语言可以给传入参数赋值。在函数内部会分配内存并初始化_vEnSession。
    OSStatus status = VTCompressionSessionCreate(NULL, (int32_t)(self.videoConfig.pushStreamWidth), (int32_t)self.videoConfig.pushStreamHeight, kCMVideoCodecType_H264, NULL, NULL, NULL, vtCompressionSessionCallback, (__bridge void * _Nullable)(self), &_vEnSession);
    if (status == noErr) {
        // 设置参数
        // ProfileLevel，h264的协议等级，不同的清晰度使用不同的ProfileLevel。
        VTSessionSetProperty(_vEnSession, kVTCompressionPropertyKey_ProfileLevel, kVTProfileLevel_H264_Main_AutoLevel);
        // 设置码率
        VTSessionSetProperty(_vEnSession, kVTCompressionPropertyKey_AverageBitRate, (__bridge CFTypeRef)@(self.videoConfig.bitrate));
        // 设置实时编码
        VTSessionSetProperty(_vEnSession, kVTCompressionPropertyKey_RealTime, kCFBooleanTrue);
        // 关闭重排Frame，因为有了B帧（双向预测帧，根据前后的图像计算出本帧）后，编码顺序可能跟显示顺序不同。此参数可以关闭B帧。
        VTSessionSetProperty(_vEnSession, kVTCompressionPropertyKey_AllowFrameReordering, kCFBooleanFalse);
        // 关键帧最大间隔，关键帧也就是I帧。此处表示关键帧最大间隔为2s。
        VTSessionSetProperty(_vEnSession, kVTCompressionPropertyKey_MaxKeyFrameInterval, (__bridge CFTypeRef)@(self.videoConfig.fps * 2));
        // 关于B帧 P帧 和I帧，请参考：http://blog.csdn.net/abcjennifer/article/details/6577934
        
        //参数设置完毕，准备开始，至此初始化完成，随时来数据，随时编码
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
