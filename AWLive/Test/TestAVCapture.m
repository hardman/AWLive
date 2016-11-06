/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://blog.csdn.net/hard_man/
 */

#import "TestAVCapture.h"

#import "AWAVCaptureManager.h"

/*
 测试代码
 */

//请修改此地址
static NSString *sRtmpUrl = @"rtmp://192.168.31.153/live/test";

@interface TestVideoCapture ()<AWAVCaptureDelegate>
//按钮
@property (nonatomic, strong) UIButton *startBtn;
@property (nonatomic, strong) UIButton *switchBtn;

@property (nonatomic, strong) UIView *preview;

@property (nonatomic, weak) ViewController *viewController;

@property (nonatomic, strong) AWAVCaptureManager *captureManager;

@end

@implementation TestVideoCapture

#pragma mark 懒加载
-(AWAVCaptureManager *)captureManager{
    if (!_captureManager) {
        _captureManager = [[AWAVCaptureManager alloc] init];
        
        //下面的3个类型必须设置，否则获取不到AVCapture
        _captureManager.captureType = AWAVCaptureTypeGPUImage;
        _captureManager.audioEncoderType = AWAudioEncoderTypeSWFAAC;
        _captureManager.videoEncoderType = AWVideoEncoderTypeHWH264;
    }
    return _captureManager;
}

-(AWAVCapture *)avCapture{
    AWAVCapture *capture = self.captureManager.avCapture;
    capture.stateDelegate = self;
    return capture;
}

-(UIView *)preview{
    if (!_preview) {
        _preview = [UIView new];
        _preview.frame = self.viewController.view.bounds;
        [self.viewController.view addSubview:_preview];
        [self.viewController.view sendSubviewToBack:_preview];
    }
    return _preview;
}

#pragma mark 初始化
-(instancetype) initWithViewController:(ViewController *)viewCtl{
    if (self = [super init]) {
        self.viewController = viewCtl;
        [self createUI];
    }
    return self;
}

-(void) createUI{
    [self.preview addSubview: self.avCapture.preview];
    self.avCapture.preview.center = self.preview.center;
    self.startBtn = [[UIButton alloc] initWithFrame:CGRectMake(100, 100, 100, 30)];
    [self.startBtn setTitle:@"开始录制！" forState:UIControlStateNormal];
    [self.startBtn addTarget:self action:@selector(onStartClick) forControlEvents:UIControlEventTouchUpInside];
    [self.viewController.view addSubview:self.startBtn];
    
    self.switchBtn = [[UIButton alloc] initWithFrame:CGRectMake(230, 100, 100, 30)];
    [self.switchBtn setTitle:@"换摄像头！" forState:UIControlStateNormal];
    [self.switchBtn addTarget:self action:@selector(onSwitchClick) forControlEvents:UIControlEventTouchUpInside];
    [self.viewController.view addSubview:self.switchBtn];
}

#pragma mark 事件
-(void)avCapture:(AWAVCapture *)capture stateChangeFrom:(aw_rtmp_state)fromState toState:(aw_rtmp_state)toState{
    switch (toState) {
        case aw_rtmp_state_idle: {
            self.startBtn.enabled = YES;
            [self.startBtn setTitle:@"开始录制" forState:UIControlStateNormal];
            break;
        }
        case aw_rtmp_state_connecting: {
            self.startBtn.enabled = NO;
            [self.startBtn setTitle:@"连接中" forState:UIControlStateNormal];
            break;
        }
        case aw_rtmp_state_opened: {
            self.startBtn.enabled = YES;
            [self.startBtn setTitle:@"运行中" forState:UIControlStateNormal];
            break;
        }
        case aw_rtmp_state_connected:
        {
            [self.startBtn setTitle:@"已连接" forState:UIControlStateNormal];
            break;
        }
        case aw_rtmp_state_closed: {
            self.startBtn.enabled = YES;
            [self.startBtn setTitle:@"已关闭" forState:UIControlStateNormal];
            break;
        }
        case aw_rtmp_state_error_write: {
            [self.startBtn setTitle:@"写入错误" forState:UIControlStateNormal];
            break;
        }
        case aw_rtmp_state_error_open: {
            [self.startBtn setTitle:@"连接错误" forState:UIControlStateNormal];
            break;
        }
        case aw_rtmp_state_error_net: {
            [self.startBtn setTitle:@"网络不给力" forState:UIControlStateNormal];
            break;
        }
    }
}

-(void) onStartClick{
    if (self.avCapture.isCapturing) {
        [self.startBtn setTitle:@"开始录制！" forState:UIControlStateNormal];
        [self.avCapture stopCapture];
    }else{
        if ([self.avCapture initCaptureWithRtmpUrl:sRtmpUrl andVideoConfig:[[AWVideoConfig alloc] init] andAudioConfig:[[AWAudioConfig alloc] init]]) {
            [self.startBtn setTitle:@"停止录制！" forState:UIControlStateNormal];
        }
    }
}

-(void) onSwitchClick{
    [self.avCapture switchCamera];
}

@end
