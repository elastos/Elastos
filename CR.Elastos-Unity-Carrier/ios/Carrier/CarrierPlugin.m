//
//  CarrierPlugin.m
//  ELASTOS_RN_FRAMEWORK
//
//  Created by jacky.li on 2018/9/26.
//  Copyright © 2018 Facebook. All rights reserved.
//

#import "CarrierPlugin.h"
#import <React/RCTLog.h>
#import "Carrier.h"
#import "RN_SESSION.h"


#define NULL_ERR [NSNull null]

static NSString * const OK = @"ok";

@interface CarrierPlugin(){
  
  NSMutableDictionary *ALL_MAP;
}
@end

@implementation CarrierPlugin
RCT_EXPORT_MODULE();

- (NSArray<NSString *> *)supportedEvents {
  return @[
           @"onIdle",
           @"onConnection",
           @"onReady",
           @"onSelfInfoChanged",
           @"onFriends",
           @"onFriendConnection",
           @"onFriendInfoChanged",
           @"onFriendPresence",
           @"onFriendRequest",
           @"onFriendAdded",
           @"onFriendRemoved",
           @"onFriendMessage",
           @"onFriendInviteRequest",
           @"onSessionRequest",
           @"onStateChanged",
           @"onStreamData",
           @"onChannelOpen", // TODO
           @"onChannelOpened",  // TODO
           @"onChannelClose",  // TODO
           @"onChannelData",  // TODO
           @"onChannelPending",  // TODO
           @"onChannelResume"
           ];
}

RCT_EXPORT_METHOD (test){
  RCTLog(@"this is native carrier test");
}

RCT_EXPORT_METHOD
(createObject : (NSDictionary *)config :(RCTResponseSenderBlock)callback){
  NSString *name = config[@"name"];
  if(ALL_MAP!=nil && [ALL_MAP objectForKey:name]){
    return;
  }
  
  Carrier *_carrier = [[Carrier alloc] init];
//  ELACarrier *elaCarrier = [_carrier getIntance];
  CarrierSendEvent sendEvent = [self carrierCallback:config];
  [_carrier start:config sendEvent:sendEvent completion:^(NSError *error) {
    if(error != nil){
      callback(@[error]);
      return;
    }
    if(ALL_MAP == nil){
      ALL_MAP = [NSMutableDictionary dictionary];
    }
    [ALL_MAP setObject:_carrier forKey:name];
    callback(@[NULL_ERR, OK]);
  }];
}

RCT_EXPORT_METHOD
(getVersion : (RCTResponseSenderBlock)callback){
  NSString *version = [ELACarrier getVersion];
  callback(@[NULL_ERR, version]);
}

RCT_EXPORT_METHOD
(isValidAddress : (NSString *)address :(RCTResponseSenderBlock)callback){
  BOOL rs = [ELACarrier isValidAddress:address];
  callback(@[NULL_ERR, @(rs)]);
}

RCT_EXPORT_METHOD
(isValidId : (NSString *)Id :(RCTResponseSenderBlock)callback){
  BOOL rs = [ELACarrier isValidId:Id];
  callback(@[NULL_ERR, @(rs)]);
}

RCT_EXPORT_METHOD
(getAddress : (NSString *)cid :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  NSString *address = [elaCarrier getAddress];
  callback(@[NULL_ERR, address]);
}

RCT_EXPORT_METHOD
(getNodeId : (NSString *)cid :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  NSString *address = [elaCarrier getNodeId];
  callback(@[NULL_ERR, address]);
}

RCT_EXPORT_METHOD
(getUserId : (NSString *)cid :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  NSString *address = [elaCarrier getUserId];
  callback(@[NULL_ERR, address]);
}

RCT_EXPORT_METHOD
(getSelfInfo : (NSString *)cid :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  ELACarrierUserInfo *info = [elaCarrier getSelfUserInfo:nil];
  NSDictionary *rs = [self user_info:info];
  
  callback(@[NULL_ERR, rs]);
}
RCT_EXPORT_METHOD
(setSelfInfo : (NSString *)cid :(NSDictionary *)info :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  ELACarrierUserInfo *f = [elaCarrier getSelfUserInfo:nil];
  f.name = info[@"name"];
  f.briefDescription = info[@"description"];
  f.gender = info[@"gender"];
  f.region = info[@"region"];
  f.phone = info[@"phone"];
  f.email = info[@"email"];
  
  [elaCarrier setSelfUserInfo:f error:nil];
  callback(@[NULL_ERR, @"ok"]);
}

RCT_EXPORT_METHOD
(getFriendInfo : (NSString *)cid :(NSString *)friendId :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  NSError *error = nil;
  ELACarrierFriendInfo *friend_info = [elaCarrier getFriendInfoForFriend:friendId error:&error];
  
  if(friend_info == nil){
    callback(@[[self create_error:error]]);
  }
  else{
    callback(@[NULL_ERR, [self friend_info:friend_info]]);
  }
}
RCT_EXPORT_METHOD
(addFriend : (NSString *)cid :(NSString *)address :(NSString *)msg :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  NSError *error = nil;
  BOOL flag = [elaCarrier addFriendWith:address withGreeting:msg error:&error];
  if(!flag){
    callback(@[[self create_error:error]]);
  }
  else{
    callback(@[NULL_ERR, OK]);
  }
}
RCT_EXPORT_METHOD
(removeFriend : (NSString *)cid :(NSString *)friendId :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  NSError *error = nil;
  BOOL flag = [elaCarrier removeFriend:friendId error:&error];
  if(!flag){
    callback(@[[self create_error:error]]);
  }
  else{
    callback(@[NULL_ERR, OK]);
  }
}
RCT_EXPORT_METHOD
(acceptFriend : (NSString *)cid :(NSString *)userId :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  NSError *error = nil;
  [elaCarrier acceptFriendWith:userId error:&error];
  if(error != nil){
    callback(@[[self create_error:error]]);
  }
  else{
    callback(@[NULL_ERR, OK]);
  }
}
RCT_EXPORT_METHOD
(sendFriendMessageTo : (NSString *)cid :(NSString *)userId :(NSString *)msg :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  NSError *error = nil;
  BOOL flag = [elaCarrier sendFriendMessageTo:userId withMessage:msg error:&error];
  if(!flag){
    callback(@[[self create_error:error]]);
  }
  else{
    callback(@[NULL_ERR, OK]);
  }
}
RCT_EXPORT_METHOD
(close : (NSString *)cid :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  Carrier *carrier = [self getCarrier:cid];
  [carrier close];
  callback(@[NULL_ERR, OK]);
}
RCT_EXPORT_METHOD
(clean : (NSString *)cid :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  Carrier *carrier = [self getCarrier:cid];
  [carrier clean:cid];
  [ALL_MAP removeObjectForKey:cid];
  callback(@[NULL_ERR, OK]);
}
RCT_EXPORT_METHOD
(setLabel : (NSString *)cid :(NSString *)friendId :(NSString *)label :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  NSError *error = nil;
  BOOL flag = [elaCarrier setLabelForFriend:friendId withLabel:label error:&error];
  if(!flag){
    callback(@[[self create_error:error]]);
  }
  else{
    callback(@[NULL_ERR, OK]);
  }
}
RCT_EXPORT_METHOD
(getFriendList : (NSString *)cid :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  NSError *error = nil;
  NSArray<ELACarrierFriendInfo *> *original_list = [elaCarrier getFriends:&error];
  NSMutableArray *list = [NSMutableArray array];
  for(ELACarrierFriendInfo *item in original_list){
    [list addObject:[self friend_info:item]];
  }
  
  callback(@[NULL_ERR, list]);
}

RCT_EXPORT_METHOD
(setSelfPresence : (NSString *)cid :(ELACarrierPresenceStatus)presence :(RCTResponseSenderBlock)callback){
  if(![self checkCarrierInstance:cid cb:callback]){
    return;
  }
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  NSError *error = nil;
  BOOL flag = [elaCarrier setSelfPresence:presence error:&error];
  
  if(!flag){
    callback(@[[self create_error:error]]);
  }
  else{
    callback(@[NULL_ERR, OK]);
  }
}

RCT_EXPORT_METHOD
(createSession: (NSString *)cid :(NSString *)friendId :(int)streamType :(int)streamMode :(RCTResponseSenderBlock)callback){
  Carrier *carrier = [self getCarrier:cid];
  RN_SESSION *_rn = [carrier getRNSessionInstance];
  
  NSError *error = nil;
  int streamId = [_rn start:friendId streamType:streamType streamMode:streamMode error:&error];
  
  if(error){
    callback(@[[self create_error:error]]);
  }
  else{
    callback(@[NULL_ERR, [NSNumber numberWithInt:streamId]]);
  }
}

RCT_EXPORT_METHOD
(sessionRequest: (NSString *)cid :(NSString *)friendId :(RCTResponseSenderBlock)callback){
  Carrier *carrier = [self getCarrier:cid];
  RN_SESSION *_rn = [carrier getRNSessionInstance];
  
  NSError *error = nil;
  [_rn sessionRequest:friendId error:&error];
  if(error){
    callback(@[[self create_error:error]]);
  }
  else{
    callback(@[NULL_ERR, OK]);
  }
}

RCT_EXPORT_METHOD
(sessionReplyRequest: (NSString *)cid :(NSString *)friendId :(int)status :(NSString *)reason :(RCTResponseSenderBlock)callback){
  Carrier *carrier = [self getCarrier:cid];
  RN_SESSION *_rn = [carrier getRNSessionInstance];
  
  NSError *error = nil;
  [_rn sessionReplyRequest:friendId status:status reason:reason error:&error];
  if(error){
    callback(@[[self create_error:error]]);
  }
  else{
    callback(@[NULL_ERR, OK]);
  }
}

RCT_EXPORT_METHOD
(writeStream: (NSString *)cid :(id)streamIdOrFriendId :(NSString *)data :(RCTResponseSenderBlock)callback){
  Carrier *carrier = [self getCarrier:cid];
  RN_SESSION *_rn = [carrier getRNSessionInstance];
  FriendSessionStream *fss = nil;
  
  if([streamIdOrFriendId isKindOfClass:[NSString class]]){
    fss = [FriendSessionStream getInstanceByFriendId:(NSString *)streamIdOrFriendId];
  }
//  else if([streamIdOrFriendId isKindOfClass:[NSNumber class]]){
  else{
    fss = [FriendSessionStream getInstanceByStreamId:[streamIdOrFriendId intValue]];
  }
  
  if(fss == nil){
    callback(@[@"invlide stream or friend id"]);
    return;
  }
  
  NSError *error = nil;
  [_rn writeToStream:fss.stream data:data error:&error];
  if(error){
    callback(@[[self create_error:error]]);
  }
  else{
    callback(@[NULL_ERR, OK]);
  }
}

RCT_EXPORT_METHOD
(closeSession: (NSString *)cid :(NSString *)friendId :(RCTResponseSenderBlock)callback){
  Carrier *carrier = [self getCarrier:cid];
  RN_SESSION *_rn = [carrier getRNSessionInstance];
  
  NSError *error = nil;
  [_rn close:friendId error:&error];
  
  if(error){
    callback(@[[self create_error:error]]);
  }
  else{
    callback(@[NULL_ERR, OK]);
  }
}


-(CarrierSendEvent) carrierCallback : (NSDictionary *)config{
  __weak __typeof(self) weakSelf = self;
  CarrierSendEvent sendEvent = ^(ELACarrier *carrier, NSDictionary* param){
    NSString *type = param[@"type"];
    NSDictionary *data = param[@"data"];
    if([type isEqualToString:@"carrierDidBecomeReady"]){
      [weakSelf carrierDidBecomeReady:data];
    }
    else if([type isEqualToString:@"connectionStatusDidChange"]){
      [weakSelf connectionStatusDidChange:data];
    }
    else if([type isEqualToString:@"selfUserInfoDidChange"]){
      [weakSelf selfUserInfoDidChange:data];
    }
    else if([type isEqualToString:@"newFriendAdded"]){
      [weakSelf newFriendAdded:data];
    }
    else if([type isEqualToString:@"didReceiveFriendsList"]){
      [weakSelf didReceiveFriendsList:data];
    }
    else if([type isEqualToString:@"friendConnectionDidChange"]){
      [weakSelf friendConnectionDidChange:data];
    }
    else if([type isEqualToString:@"friendInfoDidChange"]){
      [weakSelf friendInfoDidChange:data];
    }
    else if([type isEqualToString:@"didReceiveFriendMessage"]){
      [weakSelf didReceiveFriendMessage:data];
    }
    else if([type isEqualToString:@"friendRemoved"]){
      [weakSelf friendRemoved:data];
    }
    else if([type isEqualToString:@"friendPresenceDidChange"]){
      [weakSelf friendPresenceDidChange:data];
    }
    else if([type isEqualToString:@"didReceiveFriendRequestFromUser"]){
      [weakSelf didReceiveFriendRequestFromUser:data];
    }
    
    else if([type isEqualToString:@"onStateChanged"]){
      [weakSelf onStateChanged:data];
    }
    else if([type isEqualToString:@"onSessionRequest"]){
      [weakSelf onSessionRequest:data];
    }
    else if([type isEqualToString:@"onStreamData"]){
      [weakSelf onStreamData:data];
    }

  };
  
  return sendEvent;
}

-(void) carrierDidBecomeReady: (NSDictionary *)param{
  [self sendEventWithName:@"onReady" body:@[@"ok"]];
}
-(void) connectionStatusDidChange: (NSDictionary *)param{
  [self sendEventWithName:@"onConnection" body:@[param[@"newStatus"]]];
}
-(void) selfUserInfoDidChange: (NSDictionary *)param{
  NSDictionary *info = [self user_info:param[@"userInfo"]];
  [self sendEventWithName:@"onSelfInfoChanged" body:@[info]];
}
-(void) newFriendAdded: (NSDictionary *)param{
  NSDictionary *info = [self friend_info:param[@"friendInfo"]];
  [self sendEventWithName:@"onFriendAdded" body:@[info]];
}
-(void) didReceiveFriendsList: (NSDictionary *)param{
  NSArray<ELACarrierFriendInfo *> *original_list = param[@"friends"];
  NSMutableArray *list = [NSMutableArray array];
  for(ELACarrierFriendInfo *item in original_list){
    [list addObject:[self friend_info:item]];
  }
  [self sendEventWithName:@"onFriends" body:@[list]];
}
-(void) friendConnectionDidChange: (NSDictionary *)param{
  [self sendEventWithName:@"onFriendConnection" body:@[param]];
}
-(void) friendInfoDidChange: (NSDictionary *)param{
  NSDictionary *info = [self friend_info:param[@"friendInfo"]];
  [self sendEventWithName:@"onFriendInfoChanged" body:@[info]];
}
-(void) didReceiveFriendMessage: (NSDictionary *)param{
  [self sendEventWithName:@"onFriendMessage" body:@[param]];
}
-(void) friendRemoved: (NSDictionary *)param{
  [self sendEventWithName:@"onFriendRemoved" body:@[param]];
}
-(void) friendPresenceDidChange: (NSDictionary *)param{
  [self sendEventWithName:@"onFriendPresence" body:@[param]];
}
-(void) didReceiveFriendRequestFromUser: (NSDictionary *)param{
  NSDictionary *body = @{
                         @"userId" : param[@"userId"],
                         @"userInfo" : [self user_info:param[@"userInfo"]],
                         @"msg" : param[@"msg"]
                         };
  [self sendEventWithName:@"onFriendRequest" body:@[body]];
}
-(void) onStateChanged: (NSDictionary *)param{
  [self sendEventWithName:@"onStateChanged" body:@[param]];
}
-(void) onSessionRequest: (NSDictionary *)param{
  [self sendEventWithName:@"onSessionRequest" body:@[param]];
}
-(void) onStreamData: (NSDictionary *)param{
  [self sendEventWithName:@"onStreamData" body:@[param]];
}

-(NSString *) create_error: (id)error{
  if([error isKindOfClass:[NSString class]]){
    return error;
  }
  else if([error isKindOfClass:[NSError class]]){
    return [error localizedDescription];
  }
  
  return @"invalid error param";
}

-(BOOL) checkCarrierInstance: (NSString *)cid cb:(RCTResponseSenderBlock)callback{
  ELACarrier *elaCarrier = [self getELACarrier:cid];
  if(!elaCarrier){
    callback(@[[self create_error:@"carrier instance not exist"]]);
    return NO;
  }
  return YES;
}

-(Carrier *) getCarrier: (NSString *)cid{
  Carrier *rs = [ALL_MAP objectForKey:cid];
  return rs;
}
-(ELACarrier *) getELACarrier: (NSString *)cid{
  Carrier *carrier = [ALL_MAP objectForKey:cid];
  return [carrier getIntance];
}

-(NSDictionary *) user_info: (ELACarrierUserInfo *)info{
  NSDictionary *dic = @{
                        @"name" : info.name,
                        @"description" : info.briefDescription,
                        @"userId" : info.userId,
                        @"gender" : info.gender,
                        @"region" : info.region,
                        @"email" : info.email,
                        @"phone" : info.phone,
                        @"hasAvatar" : info.hasAvatar ? @YES : @NO
                        };
  return dic;
}
-(NSDictionary *) friend_info: (ELACarrierFriendInfo *)info{
  NSDictionary *dic = [self user_info:info];
  NSMutableDictionary *rs = [NSMutableDictionary dictionaryWithDictionary:dic];
  [rs setObject:info.label forKey:@"label"];
  [rs setObject:[NSNumber numberWithInt: info.presence] forKey:@"presence"];
  [rs setObject:[NSNumber numberWithInt: info.status] forKey:@"status"];
  
  return rs;
}

//-(NSDictionary *) caeate_error: (NSError *)error{
//  return [NSDictionary dictionaryWithObjectsAndKeys:@"code", error.code, @"message", error.userInfo, nil];
//}

@end
