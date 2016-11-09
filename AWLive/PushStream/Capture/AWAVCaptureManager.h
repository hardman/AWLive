//
//  AWAVCaptureManager.h
//  AWLive
//
//  Created by wanghongyu on 6/11/16.
//
//

/*
 用于生成AVCapture，不同的捕获视频的方法
 */

#import <Foundation/Foundation.h>
#import "AWAVCapture.h"
#import "AWEncoderManager.h"

typedef enum : NSUInteger {
    AWAVCaptureTypeNone,
    AWAVCaptureTypeSystem,
    AWAVCaptureTypeGPUImage,
} AWAVCaptureType;

@interface AWAVCaptureManager : NSObject
//视频捕获类型
@property (nonatomic, unsafe_unretained) AWAVCaptureType captureType;
@property (nonatomic, weak) AWAVCapture *avCapture;

//编码器类型
@property (nonatomic, unsafe_unretained) AWAudioEncoderType audioEncoderType;
@property (nonatomic, unsafe_unretained) AWAudioEncoderType videoEncoderType;

//配置
@property (nonatomic, strong) AWAudioConfig *audioConfig;
@property (nonatomic, strong) AWVideoConfig *videoConfig;
@end
