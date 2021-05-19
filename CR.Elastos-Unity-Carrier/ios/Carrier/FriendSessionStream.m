//
//  FriendSessionStream.m
//  ELASTOS_RN_FRAMEWORK
//
//  Created by jacky.li on 2018/11/28.
//  Copyright © 2018 Facebook. All rights reserved.
//

#import "FriendSessionStream.h"

static NSMutableDictionary *all_map = nil;

static int N = 0;

@implementation FriendSessionStream

-(instancetype) initWithFriendId : (NSString *) frinedId
{
  if(self = [super init]){
    
    if(all_map == nil){
      all_map = [NSMutableDictionary dictionary];
    }
    
    [self setId:frinedId];
    
    N++;
    int nid = N;
    [self setStreamId:nid];
    
    [self setSdp:@""];
    
    // ELACarrierStreamStateClosed
    [self setState: (int)6];
  }
  return self;
}

+(FriendSessionStream *) getInstanceByStreamId: (int)streamId
{
  FriendSessionStream *rs = nil;
  for (NSString *fid in all_map) {
    FriendSessionStream *tmp = [all_map valueForKey:fid];
    if(tmp.streamId == streamId){
      rs = tmp;
      break;
    }
  }
  
  return rs;
}
+(FriendSessionStream *) getInstanceByStream: (ELACarrierStream *)stream
{
  FriendSessionStream *rs = nil;
  for (NSString *fid in all_map) {
    FriendSessionStream *tmp = [all_map valueForKey:fid];
    if(tmp.stream == stream){
      rs = tmp;
      break;
    }
  }
  
  return rs;
}

+(FriendSessionStream *) getInstanceByFriendId: (NSString *)friendId
{
  return [all_map valueForKey:friendId];
}

+(void) putByFriendId: (NSString *)friendId
                 data:(FriendSessionStream *)data
{
  [all_map setValue:data forKey:friendId];
}

+(void) removeByFriendId: (NSString *)friendId
{
  [all_map removeObjectForKey:friendId];
}


@end
