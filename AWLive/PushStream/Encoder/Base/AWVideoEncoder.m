/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://blog.csdn.net/hard_man/
 */

#import "AWVideoEncoder.h"
#include "libyuv.h"

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
    
    //获取CVImageBufferRef中的y数据
    uint8_t *y_frame = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0);
    //获取CMVImageBufferRef中的uv数据
    uint8_t *uv_frame = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 1);
    uint8_t *u_frame = aw_alloc(uv_size);
    uint8_t *v_frame = aw_alloc(uv_size);
    uint8_t *u_frame_p = u_frame;
    uint8_t *v_frame_p = v_frame;
    
    // 注意这里：YUV420也分很多储存方式：如NV12，YV12，I420
    // 它们数据量是相同的，只是字节存储顺序不同。
    // CVImageBufferRef中存储的uv数据是这样的（NV12格式，一个u接一个v）：
    // u0 v0 u1 v1 u2 v2 ... un vn
    // 而我们想要的格式是这样的（I420格式：u和v分开）：
    // u0 u1 u2 ... un v0 v1 v2... vn
    // 所以采用隔位赋值的方法，将uv数据分开
    for (int i = 0; i < uv_size; i++) {
        *u_frame++ = *uv_frame++;
        *v_frame++ = *uv_frame++;
    }
    
    //将yuv数据按照 y0 y1 y2...yn u0 u1 u2 ...un v0 v1 v2 ... vn的格式（I420）拼起来
    uint8_t *yuv_frame = aw_alloc(uv_size * 2 + y_size);
    memcpy(yuv_frame, y_frame, y_size);
    memcpy(yuv_frame + y_size, u_frame_p, uv_size);
    memcpy(yuv_frame + y_size + uv_size, v_frame_p, uv_size);
    
    //释放资源
    aw_free(u_frame_p);
    aw_free(v_frame_p);
    
    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    
    //返回数据
    return [NSData dataWithBytesNoCopy:yuv_frame length:y_size + uv_size * 2];
}

@end
