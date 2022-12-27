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

public enum CarrierReceiptState: Int, CustomStringConvertible  {

    /// Message has been accepted by remote friend via carrier network.
    case ByFriend = 0

    /// Message has been delivered to offline message store.
    case Offline = 1

    /// Message sent before not
    /// Message send unsuccessfully
    case Error = 2

    internal static func format(_ receipt: CarrierReceiptState) -> String {
        var value : String

        switch receipt {
        case ByFriend:
            value = "ByFriend"
        case Offline:
            value = "Offline"
        case Error:
            value = "Error"
        }
        return value
    }

    public var description: String {
        return CarrierReceiptState.format(self)
    }

}
