//
//  ElastosCarrierSDK_macOS.h
//  ElastosCarrierSDK_macOS
//
//  Created by 李爱红 on 2019/4/18.
//  Copyright © 2019 org.elastos. All rights reserved.
//

#import <Cocoa/Cocoa.h>

//! Project version number for ElastosCarrierSDK_macOS.
FOUNDATION_EXPORT double ElastosCarrierSDK_macOSVersionNumber;

//! Project version string for ElastosCarrierSDK_macOS.
FOUNDATION_EXPORT const unsigned char ElastosCarrierSDK_macOSVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <ElastosCarrierSDK_macOS/PublicHeader.h>

/**
 The stream mode options.

 - CarrierStreamOptionPlain: Plain mode
 - CarrierStreamOptionReliable: Reliable mode
 - CarrierStreamOptionMultiplexing: Multiplexing mode
 - CarrierStreamOptionPortForwarding: Support portforwarding over multiplexing
 */
typedef NS_OPTIONS (int, ELACarrierStreamOptions) {
    ELACarrierStreamOptionCompress       = 0x01,
    ELACarrierStreamOptionPlain          = 0x02,
    ELACarrierStreamOptionReliable       = 0x04,
    ELACarrierStreamOptionMultiplexing   = 0x08,
    ELACarrierStreamOptionPortForwarding = 0x10,
} NS_SWIFT_NAME(CarrierStreamOptions);

