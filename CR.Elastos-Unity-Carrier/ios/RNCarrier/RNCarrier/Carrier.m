//
//  Carrier.m
//  ELASTOS_RN_FRAMEWORK
//
//  Created by jacky.li on 2018/9/25.
//  Copyright © 2018 Facebook. All rights reserved.
//

#import "Carrier.h"
#import <React/RCTLog.h>



@interface Carrier () <ELACarrierDelegate>{
  BOOL _init;
  ELACarrierConnectionStatus connectStatus;
  ELACarrier *elaCarrier;
  
  ELACarrierSessionManager *elaSessionManager;
  ELACarrierStream *_stream;
  ELACarrierStreamState _state;
  
  dispatch_queue_t managerCarrierQueue;
  CarrierSendEvent _callback;
}
@end

@implementation Carrier

-(ELACarrier *) getIntance{
  return elaCarrier;
}

//-(ELACarrierSession *) getSessionInstance{
//  return _session;
//}

- (instancetype)init {
  if (self = [super init]) {
    _init = NO;
    _state = 0;
    connectStatus = ELACarrierConnectionStatusDisconnected;
    managerCarrierQueue = dispatch_queue_create("managerCarrierQueue", NULL);
    [ELACarrier setLogLevel: ELACarrierLogLevelDebug];
    
  }
  return self;
}

-(void) start:(NSDictionary *)config sendEvent:(CarrierSendEvent)sendEvent completion:(void (^)(NSError *error))completion{
  if (_init) {
    return;
  }
  
  _init = YES;
  
  dispatch_async(managerCarrierQueue, ^{
    NSError *error = nil;
    if (elaCarrier == nil) {
      NSString *libraryDirectory = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)[0];
      NSString *name = [@"elastos_rn/" stringByAppendingString:config[@"name"]];
      NSString *elaDirectory = [libraryDirectory stringByAppendingPathComponent:name];
      
      if (![[NSFileManager defaultManager] fileExistsAtPath:elaDirectory]) {
        NSURL *url = [NSURL fileURLWithPath:elaDirectory];
        if (![[NSFileManager defaultManager] createDirectoryAtURL:url withIntermediateDirectories:YES attributes:nil error:&error]){
          RCTLog(@"Create ELACarrier persistent directory failed: %@", error);
          connectStatus = ELACarrierConnectionStatusDisconnected;
          _init = NO;
          if (completion) {
            completion(error);
          }
          return;
        }
        
        [url setResourceValue:@YES forKey:NSURLIsExcludedFromBackupKey error:nil];
      }
      
//      NSString *plistPath = [[NSBundle mainBundle]pathForResource:@"ElastosCarrier" ofType:@"plist"];
//      NSDictionary *config = [[NSDictionary alloc]initWithContentsOfFile:plistPath];
      NSArray *bootstraps = config[@"bootstraps"];
      NSMutableArray *bootstrapNodes = [[NSMutableArray alloc] initWithCapacity:bootstraps.count];
      for (NSDictionary *bootstrap in bootstraps) {
        ELABootstrapNode *node = [[ELABootstrapNode alloc] init];
        node.ipv4 = bootstrap[@"ipv4"];
        node.ipv6 = bootstrap[@"ipv6"];
        node.port = bootstrap[@"port"];
        node.publicKey = bootstrap[@"publicKey"];
        [bootstrapNodes addObject:node];
      }
      
      ELACarrierOptions *options = [[ELACarrierOptions alloc] init];
      options.persistentLocation = elaDirectory;
      options.udpEnabled = [config[@"udp_enabled"] boolValue];
      options.bootstrapNodes = bootstrapNodes;
      
      [ELACarrier initializeInstanceWithOptions:options delegate:self error:&error];
      elaCarrier = [ELACarrier getInstance];
      _init = NO;
      if (elaCarrier == nil) {
        RCTLog(@"Create ELACarrier instance failed: %@", error);
        connectStatus = ELACarrierConnectionStatusDisconnected;
        if (completion) {
          completion(error);
        }
        return;
      }
    }
    
    _init = [elaCarrier startWithIterateInterval:1000 error:&error];
    if (_init) {
      _callback = sendEvent;
    }
    else {
      RCTLog(@"Start ELACarrier instance failed: %@", error);
      [elaCarrier kill];
      elaCarrier = nil;
      connectStatus = ELACarrierConnectionStatusDisconnected;
    }
    
    if (completion) {
      completion(error);
    }
  });
}

-(void) clean: (NSString *)name{
  [self close];
  
  NSString *libraryDirectory = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)[0];
  NSString *dir = [@"elastos_rn/" stringByAppendingString:name];
  NSString *elaDirectory = [libraryDirectory stringByAppendingPathComponent:dir];
  [[NSFileManager defaultManager] removeItemAtPath:elaDirectory error:nil];
}

- (void) close{
  [elaCarrier kill];
  elaCarrier = nil;
  
  _init = NO;
  connectStatus = ELACarrierConnectionStatusDisconnected;
}

-(ELACarrierSession *) createNewSession: (NSString *)name friendId:(NSString *)friendid{
  elaSessionManager = [ELACarrierSessionManager getInstance:[self getIntance] error:nil];
//  elaSessionManager = [ELACarrierSessionManager getInstance];
  NSError *error = nil;
  ELACarrierSession *session = [elaSessionManager newSessionTo:friendid error:&error];
  
  NSString *peer = [session getPeer];
  RCTLog(@"%@", peer);
  ELACarrierStreamOptions options = ELACarrierStreamOptionMultiplexing | ELACarrierStreamOptionPortForwarding | ELACarrierStreamOptionReliable;
  
//  NSError *error = nil;
  if(_stream == nil){
    _stream = [session addStreamWithType:ELACarrierStreamTypeApplication options:options delegate:(id)self error:&error];
  }
  
//  [session sendInviteRequestWithResponseHandler:
//   ^(ELACarrierSession *session, NSInteger status, NSString *reason, NSString *sdp) {
//     RCTLog(@"Invite request response, stream state: %zd", status);
//
//     if (status == 0) {
//       NSError *error = nil;
//       if (![session startWithRemoteSdp:sdp error:&error]) {
//         RCTLog(@"Start session error: %@", error);
//       }
//     }
//     else {
//       RCTLog(@"Remote refused session invite: %d, sdp: %@", (int)status, reason);
//     }
//   } error:&error];
  
  return session;
}


#pragma mark - ELACarrierStreamDelegate
-(void) carrierStream:(ELACarrierStream *)stream stateDidChange:(enum ELACarrierStreamState)newState{
  RCTLog(@"Stream state: %d", (int)newState);
  
  if (stream != _stream || _state < 0) {
    return;
  }
  
  _state = newState;

  
  switch (newState) {
    case ELACarrierStreamStateInitialized:
 
      break;
      
    case ELACarrierStreamStateConnected:

      break;
      
    case ELACarrierStreamStateDeactivated:
    case ELACarrierStreamStateClosed:
    case ELACarrierStreamStateError:
     
      break;
      
    default:
      break;
  }
}

#pragma mark - ELACarrierDelegate
-(void) carrier:(ELACarrier *)carrier connectionStatusDidChange:(enum ELACarrierConnectionStatus)newStatus{
  RCTLog(@"connectionStatusDidChange : %d", (int)newStatus);
  
  NSDictionary *param = @{
                          @"type" : @"connectionStatusDidChange",
                          @"data" : @{
                              @"newStatus" : [NSNumber numberWithInt:newStatus]
                              }
                          };
  _callback(carrier, param);
}

-(void) carrierDidBecomeReady:(ELACarrier *)carrier{
  RCTLog(@"didBecomeReady");
  
  NSDictionary *param = @{
                          @"type" : @"carrierDidBecomeReady"
                          };
  _callback(carrier, param);
}

-(void) carrier:(ELACarrier *)carrier selfUserInfoDidChange:(ELACarrierUserInfo *)newInfo{
  RCTLog(@"selfUserInfoDidChange : %@", newInfo);
  NSDictionary *param = @{
                          @"type" : @"selfUserInfoDidChange",
                          @"data" : @{
                              @"userInfo" : newInfo
                              }
                          };
  _callback(carrier, param);
}

-(void) carrier:(ELACarrier *)carrier didReceiveFriendsList:(NSArray<ELACarrierFriendInfo *> *)friends{
  RCTLog(@"didReceiveFriendsList : %@", friends);
  
  NSDictionary *param = @{
                          @"type" : @"didReceiveFriendsList",
                          @"data" : @{
                              @"friends" : friends
                              }
                          };
  _callback(carrier, param);
}

-(void) carrier:(ELACarrier *)carrier friendInfoDidChange:(NSString *)friendId newInfo:(ELACarrierFriendInfo *)newInfo{
  RCTLog(@"friendInfoDidChange : %@", newInfo);
  NSDictionary *param = @{
                          @"type" : @"friendInfoDidChange",
                          @"data" : @{
                              @"friendInfo" : newInfo
                              }
                          };
  _callback(carrier, param);
}

-(void) carrier:(ELACarrier *)carrier friendConnectionDidChange:(NSString *)friendId newStatus:(ELACarrierConnectionStatus)newStatus{
  RCTLog(@"friendConnectionDidChange, userId : %@, newStatus : %zd", friendId, newStatus);
  NSDictionary *param = @{
                          @"type" : @"friendConnectionDidChange",
                          @"data" : @{
                              @"friendId" : friendId,
                              @"status" : [NSNumber numberWithInt:newStatus]
                              }
                          };
  _callback(carrier, param);
}

-(void) carrier:(ELACarrier *)carrier friendPresenceDidChange:(NSString *)friendId newPresence:(ELACarrierPresenceStatus)newPresence{
  RCTLog(@"friendPresenceDidChange, userId : %@, newPresence : %zd", friendId, newPresence);
  NSDictionary *param = @{
                          @"type" : @"friendPresenceDidChange",
                          @"data" : @{
                              @"friendId" : friendId,
                              @"presence" : [NSNumber numberWithInt:newPresence]
                              }
                          };
  _callback(carrier, param);
}

-(void) carrier:(ELACarrier *)carrier didReceiveFriendRequestFromUser:(NSString *)userId withUserInfo:(ELACarrierUserInfo *)userInfo hello:(NSString *)hello{
  RCTLog(@"didReceiveFriendRequestFromUser, userId : %@", userId);
  NSDictionary *param = @{
                          @"type" : @"didReceiveFriendRequestFromUser",
                          @"data" : @{
                              @"userId" : userId,
                              @"userInfo" : userInfo,
                              @"msg" : hello
                              }
                          };
  _callback(carrier, param);
}

-(void) carrier:(ELACarrier *)carrier newFriendAdded:(ELACarrierFriendInfo *)newFriend{
  RCTLog(@"newFriendAdded : %@", newFriend);
  NSDictionary *param = @{
                          @"type" : @"newFriendAdded",
                          @"data" : @{
                              @"friendInfo" : newFriend
                              }
                          };
  _callback(carrier, param);
}

-(void) carrier:(ELACarrier *)carrier friendRemoved:(NSString *)friendId{
  RCTLog(@"friendRemoved : %@", friendId);
  NSDictionary *param = @{
                          @"type" : @"friendRemoved",
                          @"data" : @{
                              @"friendId" : friendId
                              }
                          };
  _callback(carrier, param);
}

-(void) carrier:(ELACarrier *)carrier didReceiveFriendMessage:(NSString *)from withMessage:(NSString *)message{
  RCTLog(@"didReceiveFriendMessage : %@, %@", from, message);
  NSDictionary *param = @{
                          @"type" : @"didReceiveFriendMessage",
                          @"data" : @{
                              @"userId" : from,
                              @"message" : message
                              }
                          };
  _callback(carrier, param);
}



//- (void)carrierWillBecomeIdle: (ELACarrier *)carrier{
//  RCTLog(@"carrierWillBecomeIdle");
//}



@end
