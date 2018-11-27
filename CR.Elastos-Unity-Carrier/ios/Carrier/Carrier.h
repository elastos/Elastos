//
//  Carrier.h
//  ELASTOS_RN_FRAMEWORK
//
//  Created by jacky.li on 2018/9/25.
//  Copyright © 2018 Facebook. All rights reserved.
//
#import <Foundation/Foundation.h>
#import <ElastosCarrier/ElastosCarrier.h>
//#import "ElastosCarrier.h";

#import <UIKit/UIKit.h>

//! Project version number for Carrier.
FOUNDATION_EXPORT double CarrierVersionNumber;

//! Project version string for Carrier.
FOUNDATION_EXPORT const unsigned char CarrierVersionString[];

@interface Carrier : NSObject

typedef void (^CarrierSendEvent)(ELACarrier *carrier, NSDictionary *param);

-(void) start:(NSDictionary *)config sendEvent:(CarrierSendEvent)sendEvent completion:(void (^)(NSError *error))completion;
-(ELACarrier *) getIntance;
-(ELACarrierSession *) newSession: (NSString *)name friendId:(NSString *)friendId;
-(void) clean: (NSString *)name;
-(void) close;
@end
