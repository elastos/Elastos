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

/// Multiplexing channel close reason mode.
@objc(ELACloseReason)
public enum CloseReason: Int, CustomStringConvertible {
    /// Channel closed normally.
    case Normal  = 0

    /// Channel closed because timeout.
    case Timeout = 1

    /// Channel closed because error occured.
    case Error   = 2

    internal static func format(_ reason: CloseReason) -> String {
        var value: String

        switch reason {
        case Normal:
            value = "Normal"
        case Timeout:
            value = "Timeout"
        case Error:
            value = "Error"
        }

        return value
    }

    public var description: String {
        return CloseReason.format(self)
    }
}

internal func convertCCloseReasonToCloseReason(_ creason : CCloseReason) -> CloseReason {
    return CloseReason(rawValue: Int(creason.rawValue))!
}
