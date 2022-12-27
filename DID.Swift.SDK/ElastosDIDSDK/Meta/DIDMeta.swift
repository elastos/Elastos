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

public class DIDMeta: Metadata {
    private var _deactivated: Bool = false
    private var _transactionId: String?
    private var _aliasName: String?
    private var _prevSignature: String?
    private var _signature: String?
    private var _published: Int?
    private let TXID = RESERVED_PREFIX + "txid"
    private let PREV_SIGNATURE = RESERVED_PREFIX + "prevSignature"
    private let SIGNATURE = RESERVED_PREFIX + "signature"
    private let PUBLISHED = RESERVED_PREFIX + "published"
    private let ALIAS = RESERVED_PREFIX + "alias"
    private let DEACTIVATED = RESERVED_PREFIX + "deactivated"

    public required init() {
        super.init()
    }

    /// The name of alias.
    public var aliasName: String? {
        return self.get(key: ALIAS) as? String
    }

    /// Set alias for did.
    /// - Parameter alias: The ailas string.
    public func setAlias(_ alias: String?) {
        put(key: ALIAS, value: alias as Any)
    }

    /// Get transactionId.
    public var transactionId: String? {
        return self.get(key: TXID) as? String
    }

    /// Set transactionId.
    /// - Parameter newValue: The transactionId string.
    public func setTransactionId(_ newValue: String?) {
        put(key: TXID, value: newValue as Any)
    }

    /// Get the time of previous signature for did.
    public var previousSignature: String? {
       return self.get(key: PREV_SIGNATURE) as? String
    }

    /// Set the time of previous signature for did.
    /// - Parameter newValue: The time of previous signature.
    public func setPreviousSignature(_ newValue: String?) {
         put(key: PREV_SIGNATURE, value: newValue as Any)
    }

    /// Get signature.
    public var signature: String? {
        return self.get(key: SIGNATURE) as? String
    }

    /// Set signature.
    /// - Parameter newValue: The signature string.
    public func setSignature(_ newValue: String?) {
        put(key: SIGNATURE, value: newValue as Any)
    }

    /// Get the time of transaction id for did.
    /// - Returns: The time of transaction.
    public func getPublished() -> Date? {
        let time = self.get(key: PUBLISHED) as? Int
        return DateHelper.getDateFromTimeStamp(time)
    }

    /// Set the time of transaction id for did.
    /// - Parameter timestamp: The time of transaction.
    public func setPublished(_ timestamp: Date) {
        let timestampDate = DateHelper.getTimeStamp(timestamp)
        put(key: PUBLISHED, value: timestampDate as Any)
    }

    /// Get did status, deactived or not.
    public var isDeactivated: Bool {
        let v =  self.get(key: DEACTIVATED)
        if case Optional<Any>.none = v {
            return false
        }
        else {
            return v as! Bool
        }
    }

    /// Set  did status, deactived or not.
    /// - Parameter newValue: Did status.
    public func setDeactivated(_ newValue: Bool) {
        put(key: DEACTIVATED, value: newValue as Any)
    }
}
