/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://blog.csdn.net/hard_man/
 */

#import "AWVideoEncoder.h"
#include "libyuv.h"

@implementation AWVideoEncoder

-(NSData *)rotateNV12Data:(NSData *)nv12Data{
    int degree = 0;
    switch (self.videoConfig.orientation) {
        case UIInterfaceOrientationLandscapeLeft:
            degree = 90;
            break;
        case UIInterfaceOrientationLandscapeRight:
            degree = 270;
            break;
        default:
            //do nothing
            break;
    }
    if (degree != 0) {
        uint8_t *src_nv12_bytes = (uint8_t *)nv12Data.bytes;
        uint32_t width = (uint32_t)self.videoConfig.width;
        uint32_t height = (uint32_t)self.videoConfig.height;
        uint32_t w_x_h = (uint32_t)(self.videoConfig.width * self.videoConfig.height);
        
        uint8_t *rotatedI420Bytes = aw_alloc(nv12Data.length);
        
        NV12ToI420Rotate(src_nv12_bytes, width,
                         src_nv12_bytes + w_x_h, width,
                         rotatedI420Bytes, height,
                         rotatedI420Bytes + w_x_h, height / 2,
                         rotatedI420Bytes + w_x_h + w_x_h / 4, height / 2,
                         width, height, (RotationModeEnum)degree);
        
        I420ToNV12(rotatedI420Bytes, height,
                   rotatedI420Bytes + w_x_h, height / 2,
                   rotatedI420Bytes + w_x_h + w_x_h / 4, height / 2,
                   src_nv12_bytes, height, src_nv12_bytes + w_x_h, height,
                   height, width);
        
        aw_free(rotatedI420Bytes);
    }
    
    return nv12Data;
}

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
    // 获取yuv数据
    // 通过CMSampleBufferGetImageBuffer方法，获得CVImageBufferRef。
    // 这里面就包含了yuv420数据的指针
    CVImageBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(videoSample);
    
    //表示开始操作数据
    CVPixelBufferLockBaseAddress(pixelBuffer, 0);
    
    //图像宽度（像素）
    size_t pixelWidth = CVPixelBufferGetWidth(pixelBuffer);
    //图像高度（像素）
    size_t pixelHeight = CVPixelBufferGetHeight(pixelBuffer);
    //yuv中的y所占字节数
    size_t y_size = pixelWidth * pixelHeight;
    //yuv中的u和v分别所占的字节数
    size_t uv_size = y_size / 4;
    
    uint8_t *yuv_frame = aw_alloc(uv_size * 2 + y_size);
    
    //获取CVImageBufferRef中的y数据
    uint8_t *y_frame = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0);
    memcpy(yuv_frame, y_frame, y_size);
    
    //获取CMVImageBufferRef中的uv数据
    uint8_t *uv_frame = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 1);
    memcpy(yuv_frame + y_size, uv_frame, uv_size * 2);
    
    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    
    NSData *nv12Data = [NSData dataWithBytesNoCopy:yuv_frame length:y_size + uv_size * 2];
    
    //旋转
    return [self rotateNV12Data:nv12Data];
}

@end
