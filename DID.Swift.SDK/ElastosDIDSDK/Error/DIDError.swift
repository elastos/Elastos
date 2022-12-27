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

public enum DIDError: Error {
    case unknownFailure (_ des: String? = nil)
    case illegalArgument(_ des: String? = nil)

    case malformedMeta  (_ des: String? = nil)
    case malformedDID   (_ des: String? = nil)
    case malformedDIDURL(_ des: String? = nil)
    case malformedDocument  (_ des: String? = nil)
    case malformedCredential(_ des: String? = nil)
    case malformedPresentation(_ des: String? = nil)

    case didStoreError  (_ des: String? = nil)

    case didResolveError(_ des: String? = nil)
    case didDeactivated (_ des: String? = nil)
    case didExpired     (_ des: String? = nil)
    case didtransactionError(_ des: String? = nil)
    case didNotFoundError(_ des: String? = nil)

    case invalidState   (_ des: String? = nil)
    case invalidKeyError(_ des: String? = nil)

    case notFoundError (_ des: String? = nil)
}

extension DIDError {
    static func desription(_ error: DIDError) -> String {
        switch error {
        case .unknownFailure(let des):
            return des ?? "unknown failure"
        case .illegalArgument(let des):
            return des ?? "invalid arguments"

        case .malformedMeta(let des):
            return des ?? "malformed metadata"
        case .malformedDID(let des):
            return des ?? "malformed DID string"
        case .malformedDIDURL(let des):
            return des ?? "malformed DIDURL string"
        case .malformedDocument(let des):
            return des ?? "malformed DID document"
        case .malformedCredential(let des):
            return des ?? "malformed credential"
        case .malformedPresentation(let des):
            return des ?? "malformed presentation"

        case .didStoreError(let des):
            return des ?? "unknown didstore error"

        case .didResolveError(let des):
            return des ?? "did resolve failure"
        case .didDeactivated(let des):
            return des ?? "did was deactivated"
        case .didExpired(let des):
            return des ?? "did was expired"

        case .didtransactionError(let des):
            return des ?? "did transaction failure"

        case .invalidState(let des):
            return des ?? "invalid wrong state"

        case .notFoundError(let des):
            return des ?? "not found"
        case .didNotFoundError(let des):
            return des ?? "did not found"
        case .invalidKeyError(let des):
            return des ?? "invalid key"
        }
    }
}
