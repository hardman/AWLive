/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

/*
 视频捕获基类。将捕获的音/视频数据送入 encodeSampleQueue串行队列进行编码，然后送入sendSampleQueue队列发送至rtmp接口。
 */

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "aw_all.h"
#import "AWAVConfig.h"
#import "AWEncoder.h"
#import "AWEncoderManager.h"

//rtmp状态回调
extern void aw_rtmp_state_changed_cb_in_oc(aw_rtmp_state old_state, aw_rtmp_state new_state);

@class AWAVCapture;
@protocol AWAVCaptureDelegate <NSObject>
-(void) avCapture:(AWAVCapture *)capture stateChangeFrom:(aw_rtmp_state) fromState toState:(aw_rtmp_state) toState;
@end

@interface AWAVCapture : NSObject
//配置
@property (nonatomic, strong) AWAudioConfig *audioConfig;
@property (nonatomic, strong) AWVideoConfig *videoConfig;

//编码器类型
@property (nonatomic, unsafe_unretained) AWAudioEncoderType audioEncoderType;
@property (nonatomic, unsafe_unretained) AWAudioEncoderType videoEncoderType;

//状态变化回调
@property (nonatomic, weak) id<AWAVCaptureDelegate> stateDelegate;

//是否将数据发送出去
@property (nonatomic, unsafe_unretained) BOOL isCapturing;

//预览view
@property (nonatomic, strong) UIView *preview;

//根据videoConfig获取当前CaptureSession preset分辨率
@property (nonatomic, readonly, copy) NSString *captureSessionPreset;

//初始化
-(instancetype) initWithVideoConfig:(AWVideoConfig *)videoConfig audioConfig:(AWAudioConfig *)audioConfig;

//初始化
-(void) onInit;

//修改fps
-(void) updateFps:(NSInteger) fps;

//切换摄像头
-(void) switchCamera;

//停止capture
-(void) stopCapture;

//停止
-(void) onStopCapture;

//用户开始
-(void) onStartCapture;

//开始capture
-(BOOL) startCaptureWithRtmpUrl:(NSString *)rtmpUrl;

//使用rtmp协议发送数据
-(void) sendVideoSampleBuffer:(CMSampleBufferRef) sampleBuffer;
-(void) sendAudioSampleBuffer:(CMSampleBufferRef) sampleBuffer;

-(void) sendVideoYuvData:(NSData *)videoData;
-(void) sendAudioPcmData:(NSData *)audioData;

-(void) sendFlvVideoTag:(aw_flv_video_tag *)flvVideoTag;
-(void) sendFlvAudioTag:(aw_flv_audio_tag *)flvAudioTag;

@end
