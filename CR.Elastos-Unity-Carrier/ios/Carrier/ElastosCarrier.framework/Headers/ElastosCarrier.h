/*
 * Copyright (c) 2018 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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


