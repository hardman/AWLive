/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

/*
 音视频配置文件，其中有些值有固定范围，不能随意填写。
 */

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "aw_all.h"

@interface AWAudioConfig : NSObject<NSCopying>
@property (nonatomic, unsafe_unretained) NSInteger bitrate;//可自由设置
@property (nonatomic, unsafe_unretained) NSInteger channelCount;//可选 1 2
@property (nonatomic, unsafe_unretained) NSInteger sampleRate;//可选 44100 22050 11025 5500
@property (nonatomic, unsafe_unretained) NSInteger sampleSize;//可选 16 8

@property (nonatomic, readonly, unsafe_unretained) aw_faac_config faacConfig;
@end

@interface AWVideoConfig : NSObject<NSCopying>
@property (nonatomic, unsafe_unretained) NSInteger width;//可选，系统支持的分辨率，采集分辨率的宽
@property (nonatomic, unsafe_unretained) NSInteger height;//可选，系统支持的分辨率，采集分辨率的高
@property (nonatomic, unsafe_unretained) NSInteger bitrate;//自由设置
@property (nonatomic, unsafe_unretained) NSInteger fps;//自由设置
@property (nonatomic, unsafe_unretained) NSInteger dataFormat;//目前软编码只能是X264_CSP_NV12，硬编码无需设置

//推流方向
@property (nonatomic, unsafe_unretained) UIInterfaceOrientation orientation;

-(BOOL) shouldRotate;

// 推流分辨率宽高，目前不支持自由设置，只支持旋转。
// UIInterfaceOrientationLandscapeLeft 和 UIInterfaceOrientationLandscapeRight 为横屏，其他值均为竖屏。
@property (nonatomic, readonly, unsafe_unretained) NSInteger pushStreamWidth;
@property (nonatomic, readonly, unsafe_unretained) NSInteger pushStreamHeight;

@property (nonatomic, readonly, unsafe_unretained) aw_x264_config x264Config;
@end
