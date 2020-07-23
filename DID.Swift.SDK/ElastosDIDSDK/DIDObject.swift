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

public class DIDObject {
    private var _id: DIDURL?
    private var _type: String?

    init() {}

    init(_ id: DIDURL, _ type: String) {
        self._id = id
        self._type = type
    }

    /// Get 'DIDURL'
    /// - Returns: 'DIDURL'
    public func getId() -> DIDURL {
        return _id!
    }

    func setId(_ id: DIDURL) {
        self._id = id
    }

    /// Get 'Type'
    /// - Returns: 'Type'
    public func getType() -> String {
        return _type!
    }

    func setType(_ type: String) {
        self._type = type
    }

    func isDefType() -> Bool {
        return _type == Constants.DEFAULT_PUBLICKEY_TYPE
    }

    func equalsTo(_ other: DIDObject) -> Bool {
        return getId() == other.getId() &&
               getType() == other.getType()
    }
}
