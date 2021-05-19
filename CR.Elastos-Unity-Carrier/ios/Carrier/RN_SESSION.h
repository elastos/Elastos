//
//  RN_SESSION.h
//  ELASTOS_RN_FRAMEWORK
//
//  Created by jacky.li on 2018/11/28.
//  Copyright © 2018 Facebook. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <ElastosCarrier/ElastosCarrier.h>

#import "Carrier.h"

NS_ASSUME_NONNULL_BEGIN

@interface RN_SESSION : NSObject <ELACarrierStreamDelegate>

@property(assign) Carrier *carrier;
@property(assign) ELACarrier *elaCarrier;
@property(assign) ELACarrierSessionManager *elaSessionManager;


-(instancetype) initWithCarrier: (Carrier *) carrier;


/**
 new session to frinedId

 @param friendId
 @param streamType
 @param streamMode
 @param error
 @return stream id
 */
-(int) start: (NSString *)friendId
  streamType: (int)streamType
  streamMode: (int)streamMode
       error: (NSError *__autoreleasing  _Nullable * _Nullable)error;


/**
 send session request to friendId after create session.

 @param friendId
 @param error
 */
-(void) sessionRequest: (NSString *)friendId
               error: (NSError *__autoreleasing  _Nullable * _Nullable)error;


/**
 reply session request

 @param friendId
 @param status 0:accept 1:refuse
 @param reason if refuse, this is reason
 @param error
 */
-(void) sessionReplyRequest: (NSString *)friendId
                   status: (int)status
                   reason: (NSString *)reason
                    error: (NSError *__autoreleasing  _Nullable * _Nullable)error;


/**
 send data to stream

 @param stream
 @param data
 @param error
 @return data length
 */
-(NSNumber *) writeToStream: (ELACarrierStream *)stream
               data: (NSString *)data
              error: (NSError *__autoreleasing  _Nullable * _Nullable)error;


/**
 close session for friendId

 @param friendId
 @param error
 */
-(void) close: (NSString *)friendId
      error: (NSError *__autoreleasing  _Nullable * _Nullable)error;

-(BOOL) addService: (NSString *)friendId
       serviceName: (NSString *)serviceName
              host: (NSString *)host
              port: (NSString *)port
             error: (NSError *__autoreleasing  _Nullable * _Nullable)error;
-(void) removeService: (NSString *)friendId
          serviceName: (NSString *)serviceName;

-(NSNumber *) openPortFowarding: (NSString *)friendId
                    serviceName: (NSString *)serviceName
                           host: (NSString *)host
                           port: (NSString *)port
                           error: (NSError *__autoreleasing  _Nullable * _Nullable)error;
-(BOOL) closePortForwarding: (NSString *)friendId
           portForwardingId: (NSNumber *)pid
                      error: (NSError *__autoreleasing  _Nullable * _Nullable)error;
-(NSNumber *) openChannel: (NSString *)friendId
                   cookie: (NSString *)cookie
                    error: (NSError * _Nullable * _Nullable)error;
-(BOOL) closeChannel: (NSString *)friendId
           channelId: (NSNumber *)channelId
               error:(NSError * _Nullable * _Nullable)error;
-(NSNumber *)writeChannel: (NSString *)friendId
                channelId: (NSNumber *)channelId
                     data: (NSString *)data
                    error: (NSError * _Nullable * _Nullable)error;
-(BOOL) pendChannel: (NSString *)friendId
            channel: (NSNumber *)channelId
              error:(NSError * _Nullable * _Nullable)error;
-(BOOL) resumeChannel: (NSString *)friendId
              channel: (NSNumber *)channelId
                error: (NSError * _Nullable * _Nullable)error;

@end

NS_ASSUME_NONNULL_END
