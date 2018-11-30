//
//  FriendSessionStream.h
//  ELASTOS_RN_FRAMEWORK
//
//  Created by jacky.li on 2018/11/28.
//  Copyright © 2018 Facebook. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <ElastosCarrier/ElastosCarrier.h>

NS_ASSUME_NONNULL_BEGIN

@interface FriendSessionStream : NSObject


@property(copy) NSString *id;
@property(retain) ELACarrierSession *session;
@property(retain) ELACarrierStream *stream;
@property(assign) int state;
@property(assign) int streamId;
@property(copy) NSString *sdp;

//-(int) getStreamId;

-(instancetype) initWithFriendId : (NSString *)frinedId;

+(FriendSessionStream *) getInstanceByStreamId: (int)streamId;
+(FriendSessionStream *) getInstanceByFriendId: (NSString *)friendId;
+(FriendSessionStream *) getInstanceByStream: (ELACarrierStream *)stream;
+(void) putByFriendId: (NSString *)friendId
                 data:(FriendSessionStream *)data;
+(void) removeByFriendId: (NSString *)friendId;

@end

NS_ASSUME_NONNULL_END
