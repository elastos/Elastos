/*
* Copyright (c) 2020 Elastos Foundation
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
public enum IDChainRequestOperation: Int, CustomStringConvertible {
    case CREATE = 0
    case UPDATE = 1
    case DEACTIVATE

    func toString() -> String {
        let desc: String
        switch self.rawValue {
        case 0:
            desc = "create"
        case 1:
            desc = "update"
        default:
            desc = "deactivate"
        }
        return desc;
    }

    static func valueOf(_ str: String) -> IDChainRequestOperation {
        let operation: IDChainRequestOperation

        switch str.uppercased() {
        case "CREATE":
            operation = .CREATE

        case "UPDATE":
            operation = .UPDATE

        case "DEACTIVATE":
            operation = .DEACTIVATE

        default:
            operation = .DEACTIVATE
        }
        return operation
    }

    public var description: String {
        return toString()
    }
}
