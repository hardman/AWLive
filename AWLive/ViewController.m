//
//  ViewController.m
//  AWLive
//
//  Created by wanghongyu on 5/11/16.
//
//

#import "ViewController.h"

#import "TestAVCapture.h"

@interface ViewController ()
@property (nonatomic, strong) TestVideoCapture *testVideoCapture;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    self.testVideoCapture = [[TestVideoCapture alloc] initWithViewController:self];
}

-(void)viewWillLayoutSubviews{
    [super viewWillLayoutSubviews];
    [self.testVideoCapture onLayout];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
