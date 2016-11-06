/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://blog.csdn.net/hard_man/
 */

#import "AWVideoEncoder.h"

@implementation AWVideoEncoder

-(aw_flv_video_tag *) encodeYUVDataToFlvTag:(NSData *)yuvData{
    return NULL;
}

-(aw_flv_video_tag *) encodeVideoSampleBufToFlvTag:(CMSampleBufferRef)videoSample{
    return [self encodeYUVDataToFlvTag:[self convertVideoSmapleBufferToYuvData:videoSample]];
}

-(aw_flv_video_tag *)createSpsPpsFlvTag{
    return NULL;
}

-(NSData *) convertVideoSmapleBufferToYuvData:(CMSampleBufferRef) videoSample{
    //获取yuv数据
    CVImageBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(videoSample);
    
    CVPixelBufferLockBaseAddress(pixelBuffer, 0);
    
    size_t pixelWidth = CVPixelBufferGetWidth(pixelBuffer);
    size_t pixelHeight = CVPixelBufferGetHeight(pixelBuffer);
    size_t y_size = pixelWidth * pixelHeight;
    size_t uv_size = y_size / 4;
    
    uint8_t *y_frame = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0);
    uint8_t *uv_frame = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 1);
    uint8_t *u_frame = aw_alloc(uv_size);
    uint8_t *v_frame = aw_alloc(uv_size);
    uint8_t *u_frame_p = u_frame;
    uint8_t *v_frame_p = v_frame;
    
    for (int i = 0; i < uv_size; i++) {
        *u_frame++ = *uv_frame++;
        *v_frame++ = *uv_frame++;
    }
    
    uint8_t *yuv_frame = aw_alloc(uv_size * 2 + y_size);
    memcpy(yuv_frame, y_frame, y_size);
    memcpy(yuv_frame + y_size, u_frame_p, uv_size);
    memcpy(yuv_frame + y_size + uv_size, v_frame_p, uv_size);
    
    aw_free(u_frame_p);
    aw_free(v_frame_p);
    
    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    
    return [NSData dataWithBytesNoCopy:yuv_frame length:y_size + uv_size * 2];
}

@end
