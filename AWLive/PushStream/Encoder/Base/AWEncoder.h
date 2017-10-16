/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

/*
 编码器基类声明公共接口
 */

#import <AVFoundation/AVFoundation.h>
#import "AWAVConfig.h"

#include "aw_all.h"

typedef enum : NSUInteger {
    AWEncoderErrorCodeVTSessionCreateFailed,
    AWEncoderErrorCodeVTSessionPrepareFailed,
    AWEncoderErrorCodeLockSampleBaseAddressFailed,
    AWEncoderErrorCodeEncodeVideoFrameFailed,
    AWEncoderErrorCodeEncodeCreateBlockBufFailed,
    AWEncoderErrorCodeEncodeCreateSampleBufFailed,
    AWEncoderErrorCodeEncodeGetSpsPpsFailed,
    AWEncoderErrorCodeEncodeGetH264DataFailed,
    
    AWEncoderErrorCodeCreateAudioConverterFailed,
    AWEncoderErrorCodeAudioConverterGetMaxFrameSizeFailed,
    AWEncoderErrorCodeAudioEncoderFailed,
} AWEncoderErrorCode;

@class AWEncoderManager;
@interface AWEncoder : NSObject
@property (nonatomic, weak) AWEncoderManager *manager;
//开始
-(void) open;
//结束
-(void) close;
//错误
-(void) onErrorWithCode:(AWEncoderErrorCode) code des:(NSString *) des;

@end
