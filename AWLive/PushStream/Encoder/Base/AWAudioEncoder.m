/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://blog.csdn.net/hard_man/
 */

#import "AWAudioEncoder.h"

@implementation AWAudioEncoder
-(aw_flv_audio_tag *) encodePCMDataToFlvTag:(NSData *)pcmData{
    return NULL;
}

-(aw_flv_audio_tag *) encodeAudioSampleBufToFlvTag:(CMSampleBufferRef)audioSample{
    return [self encodePCMDataToFlvTag:[self convertAudioSmapleBufferToPcmData:audioSample]];
}

-(aw_flv_audio_tag *)createAudioSpecificConfigFlvTag{
    return NULL;
}

-(NSData *) convertAudioSmapleBufferToPcmData:(CMSampleBufferRef) audioSample{
    NSInteger audioDataSize = CMSampleBufferGetTotalSampleSize(audioSample);
    
    int8_t *audio_data = aw_alloc((int32_t)audioDataSize);
    
    CMBlockBufferRef dataBuffer = CMSampleBufferGetDataBuffer(audioSample);
    CMBlockBufferCopyDataBytes(dataBuffer, 0, audioDataSize, audio_data);
    
    return [NSData dataWithBytesNoCopy:audio_data length:audioDataSize];
}

@end
