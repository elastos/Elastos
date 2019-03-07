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

public enum CarrierError: Error {
    case InvalidArgument
    case GeneralError(ecode: Int)
    case DHTError(ecode: Int)
    case SystemError(ecode: Int)
    case ICEError(ecode: Int)
    case UnknowError(ecode: Int)
}

@inline(__always) internal func getErrorString(_ errno: Int) -> String {
    var data = Data(count: 1024)

    let errstr = data.withUnsafeMutableBytes() {
        (ptr: UnsafeMutablePointer<Int8>) -> String in
        let err: UnsafeMutablePointer  = ela_get_strerror(errno, ptr, 1024) ?? UnsafeMutablePointer<Int8>(mutating: ("unkonw error" as NSString).utf8String!)
        return String(cString: err)
    }
    return errstr
}

extension CarrierError: LocalizedError {
    public var errorDescription: String? {

        switch self {
        case .InvalidArgument:
            return "Invalid argument"
        case .GeneralError(let errno):
            return String(format: "General Error: \(getErrorString(errno))")
        case .DHTError(let errno):
            return String(format: "DHT Error: \(getErrorString(errno))")
        case .SystemError(let errno):
            return String(format: "System Error: \(getErrorString(errno))")
        case .ICEError(let errno):
            return String(format: "ICE Error: \(getErrorString(errno))")
        case .UnknowError(let errno):
            return String(format: "Unknown Error 0x%x", errno)
        }
    }
}

extension CarrierError: CustomNSError {
    public var errorCode: Int {
        switch self {
        case .InvalidArgument:
            return 1
        case .GeneralError(let errno):
            return errno
        case .DHTError(let errno):
            return errno
        case .SystemError(let errno):
            return errno
        case .ICEError(let errno):
            return errno
        case .UnknowError(let errno):
            return errno
        }
    }
}

extension CarrierError {
    internal static func getFacility(_ errno: Int) -> Int {
        return (errno &  0x7FFFFFFF) >> 24
    }

    internal static func getCode(_ errno: Int) -> Int {
        return (errno & 0x00FFFFFF)
    }

    internal static func FromErrorCode(errno: Int) -> CarrierError {
        let facility = getFacility(errno)
        let code = getCode(errno)

        switch facility {
        case 1:
            return CarrierError.GeneralError(ecode: code)
        case 2:
            return CarrierError.SystemError(ecode: code)
        case 5:
            return CarrierError.ICEError(ecode: code)
        case 6:
            return CarrierError.DHTError(ecode: code)
        default:
            return CarrierError.UnknowError(ecode: code)
        }
    }
}
