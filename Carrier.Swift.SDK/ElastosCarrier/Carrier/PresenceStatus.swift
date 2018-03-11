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
    Carrier node presence status to friends
 */
@objc(ELACarrierPresenceStatus)

public enum CarrierPresenceStatus : Int, CustomStringConvertible {

    /// Carrier node in online or available to friends.
    case None = 0

    /// Carrier node is being away.
    case Away = 1

    /// Carrier node is being busy.
    case Busy = 2

    internal static func format(_ presence: CarrierPresenceStatus) -> String {
        var value : String

        switch presence {
        case None:
            value = "None"
        case Away:
            value = "Away"
        case Busy:
            value = "Busy"
        }
        return value
    }

    public var description: String {
        return CarrierPresenceStatus.format(self)
    }
}

internal func convertCPresenceStatusToCarrierPresenceStatus(_ cstatus: Int32) -> CarrierPresenceStatus {
    return CarrierPresenceStatus(rawValue: Int(cstatus))!
}

internal func convertCarrierPresenceStatusToCPresenceStatus(_ status: CarrierPresenceStatus) -> CPresenceStatus {
    return CPresenceStatus(Int32(status.rawValue))
}
