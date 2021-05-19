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

import Foundation

///
/// Elastos carrier stream type.
///
/// Reference:
/// https://tools.ietf.org/html/rfc4566#section-5.14
/// https://tools.ietf.org/html/rfc4566#section-8
///
@objc(ELACarrierStreamType)
public enum CarrierStreamType: Int, CustomStringConvertible {
    /// Audio stream
    case Audio       = 0

    /// Video stream
    case Video       = 1

    /// Text stream
    case Text        = 2

    /// Application stream
    case Application = 3

    /// Message stream
    case Message     = 4

    internal static func format(_ type: CarrierStreamType) -> String {
        var value: String

        switch type {
        case Audio:
            value = "Audio"
        case Video:
            value = "Video"
        case Text:
            value = "Text"
        case Application:
            value = "Application"
        case Message:
            value = "Message"
        }

        return value
    }

    public var description: String {
        return CarrierStreamType.format(self)
    }
}

internal func convertCarrierStreamTypeToCStreamType(_ type : CarrierStreamType) -> CStreamType {
    return CStreamType(rawValue: UInt32(type.rawValue))
}

internal func convertCStreamTypeToCarrierStreamType(_ ctype : CStreamType) -> CarrierStreamType {
    return CarrierStreamType(rawValue: Int(ctype.rawValue))!
}
