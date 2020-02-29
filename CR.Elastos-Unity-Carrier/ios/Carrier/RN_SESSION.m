//
//  RN_SESSION.m
//  ELASTOS_RN_FRAMEWORK
//
//  Created by jacky.li on 2018/11/28.
//  Copyright © 2018 Facebook. All rights reserved.
//

#import "RN_SESSION.h"
#import <ElastosCarrierSDK/ElastosCarrierSDK.h>
#import <React/RCTLog.h>

#import "Carrier.h"
#import "FriendSessionStream.h"



@implementation RN_SESSION

- (instancetype)init{
  if(self = [super init]){
 
  }
  return self;
}
-(instancetype) initWithCarrier: (Carrier *) carrier{
  if(self = [super init]){
    [self setCarrier:carrier];
    [self setElaCarrier: [carrier getInstance]];
    
    NSError *error = nil;
  [ELACarrierSessionManager initializeSharedInstance:self.elaCarrier sessionRequestHandler:
    
      ^(ELACarrier *_carrier, NSString *friendId, NSString *sdp){
        RCTLog(@"[ onSessionRequest ] => %@, %@", friendId, sdp);
        
        FriendSessionStream *fss = [self getFriendSessionByFriendId:friendId];
        [fss setSdp:sdp];
      
        NSDictionary *param = @{
                                @"type" : @"onSessionRequest",
                                @"data" : @{
                                    @"sdp" : sdp,
                                    @"friendId" : friendId
                                }
                              };
        self.carrier.callback(self.elaCarrier, param);
      
    } error: &error];
    [ELACarrierSessionManager sharedInstance];
//    [self setElaSessionManager:m];
  }
  return self;
}

-(int) start: (NSString *)friendId
  streamType: (int)streamType
  streamMode: (int)streamMode
       error: (NSError *__autoreleasing  _Nullable * _Nullable)error
{
  FriendSessionStream *fss = [self addStreamByType:friendId streamType:streamType streamMode:streamMode error:error];
  
  return fss.streamId;
}

-(FriendSessionStream *) addStreamByType: (NSString *)friendId
             streamType: (int)streamType
             streamMode: (int)streamMode
                  error: (NSError *__autoreleasing  _Nullable * _Nullable)error
{
  FriendSessionStream *fss = [self getFriendSessionByFriendId:friendId];
  ELACarrierSession *session = fss.session;
  ELACarrierStream *stream = [session addStreamWithType:(ELACarrierStreamType)streamType
                                                options:(ELACarrierStreamOptions)streamMode
                                               delegate:(id)self
                                                  error:error];
  
  [fss setStream:stream];
  
  return fss;
}

-(FriendSessionStream *) getFriendSessionByFriendId: (NSString *)friendId
{
  FriendSessionStream *fss = [FriendSessionStream getInstanceByFriendId:friendId];
  if(!fss){
    fss = [[FriendSessionStream alloc] initWithFriendId:friendId];
    NSError *error = nil;
    ELACarrierSession *session = [self.elaSessionManager newSessionTo:friendId error:&error];
    [fss setSession:session];
    
    [FriendSessionStream putByFriendId:friendId data:fss];
  }
  return fss;
}

-(void) sessionRequest: (NSString *)friendId
                 error: (NSError *__autoreleasing  _Nullable * _Nullable)error
{
  FriendSessionStream *fss = [self getFriendSessionByFriendId:friendId];
  ELACarrierSession *_session = fss.session;
  
  [_session sendInviteRequestWithResponseHandler:
   ^(ELACarrierSession *session, NSInteger status, NSString *reason, NSString *sdp) {
     RCTLog(@"Invite request response, stream state: %zd", status);
     
     if(status == 0){
       NSError *error = nil;
       if(![session startWithRemoteSdp:sdp error:&error]){
         RCTLog(@"Start session error: %@", error);
       }
     }
     else {
       RCTLog(@"Remote refused session invite: %d, sdp: %@", (int)status, reason);
     }
   } error:error];
}

-(void) sessionReplyRequest: (NSString *)friendId
                     status: (int)status
                     reason: (NSString *)reason
                      error: (NSError *__autoreleasing  _Nullable * _Nullable)error
{
  FriendSessionStream *fss = [self getFriendSessionByFriendId:friendId];
  BOOL flag = [fss.session replyInviteRequestWithStatus:(NSInteger)status reason:reason error:error];
  
    if(flag){
        [fss.session startWithRemoteSdp:fss.sdp error:error];
    }
}

-(NSNumber *) writeToStream: (ELACarrierStream *)stream
                data: (NSString *)data
               error: (NSError *__autoreleasing  _Nullable * _Nullable)error
{
  NSNumber *rs = [stream writeData:[data dataUsingEncoding:NSUTF8StringEncoding] error:error];
  return rs;
}

-(void) close: (NSString *)friendId
        error: (NSError *__autoreleasing  _Nullable * _Nullable)error
{
  FriendSessionStream *fss = [FriendSessionStream getInstanceByFriendId:friendId];
  if(fss == nil){
    NSDictionary *dict = @{NSLocalizedDescriptionKey:@"session not start"};
    *error = [NSError errorWithDomain:NSCustomErrorDomain code:10001 userInfo:dict];
    return;
  }
  [fss.session removeStream:fss.stream error:error];
  [fss.session close];
  [FriendSessionStream removeByFriendId:friendId];
}

-(BOOL) addService: (NSString *)friendId
       serviceName: (NSString *)serviceName
              host: (NSString *)host
              port: (NSString *)port
              error: (NSError *__autoreleasing  _Nullable * _Nullable)error
{
    FriendSessionStream *fss = [FriendSessionStream getInstanceByFriendId:friendId];
    if(fss == nil){
        NSDictionary *dict = @{NSLocalizedDescriptionKey:@"session not start"};
        *error = [NSError errorWithDomain:NSCustomErrorDomain code:10001 userInfo:dict];
        return NO;
    }
    BOOL flag = [fss.session addServiceWithName: serviceName protocol:1 host:host port:port error:error];
    return flag;
}
-(void) removeService: (NSString *)friendId
          serviceName: (NSString *)serviceName
{
    FriendSessionStream *fss = [FriendSessionStream getInstanceByFriendId:friendId];
    [fss.session removeServiceWithName: serviceName];
}
-(NSNumber *) openPortFowarding: (NSString *)friendId
                    serviceName: (NSString *)serviceName
                           host: (NSString *)host
                           port: (NSString *)port
                          error: (NSError *__autoreleasing  _Nullable * _Nullable)error
{
    FriendSessionStream *fss = [FriendSessionStream getInstanceByFriendId:friendId];
    
    NSNumber *rs = [fss.stream openPortForwardingForService:serviceName withProtocol:1 host:host port:port error:error];
    return rs;
}
-(BOOL) closePortForwarding: (NSString *)friendId
           portForwardingId: (NSNumber *)pid
                      error: (NSError *__autoreleasing  _Nullable * _Nullable)error
{
    FriendSessionStream *fss = [FriendSessionStream getInstanceByFriendId:friendId];
    if(fss == nil){
        NSDictionary *dict = @{NSLocalizedDescriptionKey:@"session not start"};
        *error = [NSError errorWithDomain:NSCustomErrorDomain code:10001 userInfo:dict];
        return NO;
    }
    
    BOOL flag = [fss.stream closePortForwarding: pid error:error];
    return flag;
}

-(NSNumber *) openChannel: (NSString *)friendId
                   cookie: (NSString *)cookie
                    error: (NSError * _Nullable * _Nullable)error
{
    FriendSessionStream *fss = [FriendSessionStream getInstanceByFriendId:friendId];
    NSNumber *rs = [fss.stream openChannelWithCookie: cookie error:error];
    return rs;
}
-(BOOL) closeChannel: (NSString *)friendId
           channelId: (NSNumber *)channelId
               error:(NSError * _Nullable * _Nullable)error
{
    FriendSessionStream *fss = [FriendSessionStream getInstanceByFriendId:friendId];
    BOOL flag = [fss.stream closeChannel: channelId error:error];
    return flag;
}
-(NSNumber *)writeChannel: (NSString *)friendId
                channelId: (NSNumber *)channelId
                     data: (NSString *)data
                    error: (NSError * _Nullable * _Nullable)error
{
    FriendSessionStream *fss = [FriendSessionStream getInstanceByFriendId:friendId];
    NSNumber *rs = [fss.stream writeChannel: channelId data:[data dataUsingEncoding:NSUTF8StringEncoding] error:error];
    return rs;
}
-(BOOL) pendChannel: (NSString *)friendId
            channel: (NSNumber *)channelId
              error:(NSError * _Nullable * _Nullable)error
{
    FriendSessionStream *fss = [FriendSessionStream getInstanceByFriendId:friendId];
    BOOL flag = [fss.stream pendChannel: channelId error:error];
    return flag;
}
-(BOOL) resumeChannel: (NSString *)friendId
              channel: (NSNumber *)channelId
                error: (NSError * _Nullable * _Nullable)error
{
    FriendSessionStream *fss = [FriendSessionStream getInstanceByFriendId:friendId];
    BOOL flag = [fss.stream resumeChannel: channelId error:error];
    return flag;
}


#pragma mark - ELACarrierStreamDelegate
-(void) carrierStream:(ELACarrierStream *)stream
       stateDidChange:(enum ELACarrierStreamState)newState
{
  RCTLog(@"Stream state: %d", (int)newState);
  
  FriendSessionStream *fss = [FriendSessionStream getInstanceByStream:stream];
  [fss setState:newState];
  NSDictionary *param = @{
                          @"type" : @"onStateChanged",
                          @"data" : @{
                              @"state" : [NSNumber numberWithInt:newState],
                              @"friendId" : fss.id,
                              @"streamId" : [NSNumber numberWithInt:fss.streamId]
                              }
                          };
  self.carrier.callback(self.elaCarrier, param);

}

-(void) carrierStream:(ELACarrierStream *)stream
       didReceiveData:(NSData *)data
{
  NSString *d = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
  RCTLog(@"[ onStreamData ] => data=%@", d);
  
  FriendSessionStream *fss = [FriendSessionStream getInstanceByStream:stream];
  NSDictionary *param = @{
                          @"type" : @"onStreamData",
                          @"data" : @{
                              @"text" : d,
                              @"friendId" : fss.id,
                              @"streamId" : [NSNumber numberWithInt:fss.streamId]
                              }
                          };
  self.carrier.callback(self.elaCarrier, param);
}

@end


