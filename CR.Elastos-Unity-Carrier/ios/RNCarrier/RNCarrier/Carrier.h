#import <Foundation/Foundation.h>
#import <ElastosCarrier/ElastosCarrier.h>



@interface RNCarrier : NSObject

typedef void (^CarrierSendEvent)(ELACarrier *carrier, NSDictionary *param);

-(void) start:(NSDictionary *)config sendEvent:(CarrierSendEvent)sendEvent completion:(void (^)(NSError *error))completion;
-(ELACarrier *) getIntance;
-(ELACarrierSession *) createNewSession: (NSString *)name friendId:(NSString *)friendId;
-(void) clean: (NSString *)name;
-(void) close;
@end
