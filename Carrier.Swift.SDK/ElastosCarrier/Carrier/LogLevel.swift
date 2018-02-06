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

/**
    Carrier log level to control or filter log output
 */
@objc(ELACarrierLogLevel)
public enum CarrierLogLevel : Int, CustomStringConvertible {

    /// Log level None
    /// Indicate disable log output.
    case None = 0

    /// Log level fatal
    /// Indicate output log with level 'Fatal' only.
    case Fatal = 1

    /// Log level error.
    /// Indicate output log above 'Error' level.
    case Error = 2

    /// Log level warning.
    /// Indicate output log above 'Warning' level.

    case Warning = 3

    /// Log level info.
    /// Indicate output log above 'Info' level.
    case Info = 4

    /// Log level debug.
    /// Indicate output log above 'Debug' level.
    case Debug = 5

    /// Log level trace.
    /// Indicate output log above 'Trace' level.
    case Trace = 6

    /// Log level verbose.
    /// Indicate output log above 'Verbose' level.
    case Verbose = 7    

    internal static func format(_ level: CarrierLogLevel) -> String {
        var value : String

        switch level {
        case None:
            value = "None"
        case Fatal:
            value = "Fatal"
        case Error:
            value = "Error"
        case Warning:
            value = "Warning"
        case Info:
            value = "Info"
        case Debug:
            value = "Debug"
        case Trace:
            value = "Trace"
        case Verbose:
            value = "Verbose"
        }
        return value
    }

    public var description: String {
        return CarrierLogLevel.format(self)
    }
}

internal func convertCarrierLogLevelToCLogLevel(_ level : CarrierLogLevel) -> CLogLevel {
    return CLogLevel(rawValue: UInt32(level.rawValue))
}

internal func >= (left: CarrierLogLevel, right: CarrierLogLevel) -> Bool {
    return (left.rawValue >= right.rawValue)
}
