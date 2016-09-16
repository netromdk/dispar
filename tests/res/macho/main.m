#import <Foundation/Foundation.h>

@interface Foo : NSObject
- (void)run;
@end

@implementation Foo
- (void)run
{
  NSLog(@"Hello ObjC!");
}
@end

int main()
{
  @autoreleasepool {
    Foo *foo = [[Foo alloc] init];
    [foo run];
    return 0;
  }
}
