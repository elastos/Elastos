@import Foundation;

//! Project version number for ElastosCarrier.
FOUNDATION_EXPORT double ElastosCarrierVersionNumber;

//! Project version string for ElastosCarrier.
FOUNDATION_EXPORT const unsigned char ElastosCarrierVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <ElastosCarrier/PublicHeader.h>

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


