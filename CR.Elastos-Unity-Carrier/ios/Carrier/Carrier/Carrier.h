//
//  Carrier.h
//  ELASTOS_RN_FRAMEWORK
//
//  Created by jacky.li on 2018/9/25.
//  Copyright © 2018 Facebook. All rights reserved.
//
#import <Foundation/Foundation.h>
#import <ElastosCarrier/ElastosCarrier.h>



@interface Carrier : NSObject

typedef void (^CarrierSendEvent)(ELACarrier *carrier, NSDictionary *param);

-(void) start:(NSDictionary *)config sendEvent:(CarrierSendEvent)sendEvent completion:(void (^)(NSError *error))completion;
-(ELACarrier *) getIntance;
-(ELACarrierSession *) createNewSession: (NSString *)name friendId:(NSString *)friendId;
-(void) clean: (NSString *)name;
-(void) close;
@end
