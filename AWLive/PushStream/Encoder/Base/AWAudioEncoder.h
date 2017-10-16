/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

/*
 音频编码器基类，只声明接口，和一些公共转换数据函数。
 */

#import "AWEncoder.h"

@interface AWAudioEncoder : AWEncoder

@property (nonatomic, copy) AWAudioConfig *audioConfig;
//编码
-(aw_flv_audio_tag *) encodePCMDataToFlvTag:(NSData *)pcmData;

-(aw_flv_audio_tag *) encodeAudioSampleBufToFlvTag:(CMSampleBufferRef)audioSample;

//创建 audio specific config
-(aw_flv_audio_tag *) createAudioSpecificConfigFlvTag;

//转换
-(NSData *) convertAudioSmapleBufferToPcmData:(CMSampleBufferRef) audioSample;

@end
