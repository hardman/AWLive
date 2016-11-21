/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://blog.csdn.net/hard_man/
 */

/*
 音视频配置文件，其中有些值有固定范围，不能随意填写。
 */

#import <Foundation/Foundation.h>
#import "aw_all.h"

@interface AWAudioConfig : NSObject
@property (nonatomic, unsafe_unretained) NSInteger bitrate;//可自由设置
@property (nonatomic, unsafe_unretained) NSInteger channelCount;//可选 1 2
@property (nonatomic, unsafe_unretained) NSInteger sampleRate;//可选 44100 22050 11025 5500
@property (nonatomic, unsafe_unretained) NSInteger sampleSize;//可选 16 8

@property (nonatomic, readonly, unsafe_unretained) aw_faac_config faacConfig;
@end

@interface AWVideoConfig : NSObject
@property (nonatomic, unsafe_unretained) NSInteger width;//可选，系统支持的分辨率
@property (nonatomic, unsafe_unretained) NSInteger height;//可选，系统支持的分辨率
@property (nonatomic, unsafe_unretained) NSInteger bitrate;//自由设置
@property (nonatomic, unsafe_unretained) NSInteger fps;//自由设置
@property (nonatomic, unsafe_unretained) NSInteger dataFormat;//目前软编码只能是X264_CSP_NV12，硬编码无需设置

@property (nonatomic, readonly, unsafe_unretained) aw_x264_config x264Config;
@end
