/*
 copyright 2016 wanghongyu.
 The project page：https://github.com/hardman/AWLive
 My blog page: http://www.jianshu.com/u/1240d2400ca1
 */

#import "TestAVCapture.h"

#import "AWAVCaptureManager.h"

/*
 测试代码
 */

//请修改此地址
static NSString *sRtmpUrl = @"rtmp://192.168.1.135:1935/live/test";

@interface TestVideoCapture ()<AWAVCaptureDelegate>

//按钮
@property (nonatomic, strong) UIButton *startBtn;
@property (nonatomic, strong) UIButton *switchBtn;

//状态
@property (nonatomic, strong) UILabel *stateLabel;

//预览
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
        _captureManager.audioEncoderType = AWAudioEncoderTypeHWAACLC;
        _captureManager.videoEncoderType = AWVideoEncoderTypeHWH264;
        _captureManager.audioConfig = [[AWAudioConfig alloc] init];
        _captureManager.videoConfig = [[AWVideoConfig alloc] init];
        
        //竖屏推流
        _captureManager.videoConfig.orientation = UIInterfaceOrientationPortrait;
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
    
    self.stateLabel = [[UILabel alloc] init];
    self.stateText = @"未连接";
    [self.viewController.view addSubview:self.stateLabel];
    
    self.startBtn = [[UIButton alloc] init];
    [self.startBtn setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
    [self.startBtn setTitleColor:[UIColor grayColor] forState:UIControlStateHighlighted];
    self.startBtn.backgroundColor = [UIColor blackColor];
    [self.startBtn setTitle:@"开始直播" forState:UIControlStateNormal];
    [self.startBtn addTarget:self action:@selector(onStartClick) forControlEvents:UIControlEventTouchUpInside];
    [self.viewController.view addSubview:self.startBtn];
    
    self.startBtn.layer.borderWidth = 0.5;
    self.startBtn.layer.borderColor = [[UIColor colorWithRed:0.2 green:0.2 blue:0.2 alpha:1] CGColor];
    self.startBtn.layer.cornerRadius = 5;
    
    self.switchBtn = [[UIButton alloc] init];
    UIImage *switchImage = [self imageWithPath:@"camera_switch.png" scale:2];
    switchImage = [switchImage imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];
    [self.switchBtn setImage:switchImage forState:UIControlStateNormal];
    [self.switchBtn setTintColor:[UIColor whiteColor]];
    [self.switchBtn addTarget:self action:@selector(onSwitchClick) forControlEvents:UIControlEventTouchUpInside];
    [self.viewController.view addSubview:self.switchBtn];
}

-(UIImage *)imageWithPath:(NSString *)path scale:(CGFloat)scale{
    NSString *imagePath = [[NSBundle mainBundle] pathForResource:path ofType:nil];
    if (imagePath) {
        NSData *imgData = [NSData dataWithContentsOfFile:imagePath];
        if (imgData) {
            UIImage *image = [UIImage imageWithData:imgData scale:scale];
            return image;
        }
    }
    
    return nil;
}

-(void) onLayout{
    CGSize screenSize = [UIScreen mainScreen].bounds.size;
    
    self.stateLabel.frame = CGRectMake(30, 130, 100, 30);
    
    self.startBtn.frame = CGRectMake(40, screenSize.height - 150 - 40, screenSize.width - 80, 40);
    
    self.switchBtn.frame = CGRectMake(screenSize.width - 30 - self.switchBtn.currentImage.size.width, 130, self.switchBtn.currentImage.size.width, self.switchBtn.currentImage.size.height);
    
    self.preview.frame = self.viewController.view.bounds;
    self.avCapture.preview.frame = self.preview.bounds;
}

-(void) setStateText:(NSString *)stateText{
    NSAttributedString *attributeString = [[NSAttributedString alloc] initWithString:stateText
                                                                          attributes:@{
                                                                                       NSForegroundColorAttributeName: [UIColor whiteColor],
                                                                                       NSStrokeColorAttributeName: [UIColor blackColor],
                                                                                       NSStrokeWidthAttributeName: @(-0.5)
                                                                                       }];
    self.stateLabel.attributedText = attributeString;
}

#pragma mark 事件
-(void)avCapture:(AWAVCapture *)capture stateChangeFrom:(aw_rtmp_state)fromState toState:(aw_rtmp_state)toState{
    switch (toState) {
        case aw_rtmp_state_idle: {
            self.startBtn.enabled = YES;
            [self.startBtn setTitle:@"开始直播" forState:UIControlStateNormal];
            self.stateText = @"未连接";
            break;
        }
        case aw_rtmp_state_connecting: {
            self.startBtn.enabled = NO;
            self.stateText = @"连接中";
            break;
        }
        case aw_rtmp_state_opened: {
            self.startBtn.enabled = YES;
            self.stateText = @"正在直播";
            break;
        }
        case aw_rtmp_state_connected: {
            self.stateText = @"连接成功";
            break;
        }
        case aw_rtmp_state_closed: {
            self.startBtn.enabled = YES;
            self.stateText = @"已关闭";
            break;
        }
        case aw_rtmp_state_error_write: {
            self.stateText = @"写入错误";
            break;
        }
        case aw_rtmp_state_error_open: {
            self.stateText = @"连接错误";
            self.startBtn.enabled = YES;
            break;
        }
        case aw_rtmp_state_error_net: {
            self.stateText = @"网络不给力";
            self.startBtn.enabled = YES;
            break;
        }
    }
}

-(void) onStartClick{
    if (self.avCapture.isCapturing) {
        [self.startBtn setTitle:@"开始直播" forState:UIControlStateNormal];
        [self.avCapture stopCapture];
    }else{
        if ([self.avCapture startCaptureWithRtmpUrl:sRtmpUrl]) {
            [self.startBtn setTitle:@"停止直播" forState:UIControlStateNormal];
        }
    }
}

-(void) onSwitchClick{
    [self.avCapture switchCamera];
}

@end
