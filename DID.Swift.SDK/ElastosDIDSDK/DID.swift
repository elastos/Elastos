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
import PromiseKit

public class DID {
    private static let TAG = "DID"

    private var _method: String?
    private var _methodSpecificId: String?
    private var _metadata: DIDMeta?

    public static let METHOD: String = "elastos"

    init() {}
    
    init(_ method: String, _ methodSpecificId: String) {
        self._method = method
        self._methodSpecificId = methodSpecificId
    }

    public init(_ did: String) throws {
        guard !did.isEmpty else {
            throw DIDError.illegalArgument("empty did string")
        }

        do {
            try ParserHelper.parse(did, true, DID.Listener(self))
        } catch {
            Log.e(DID.TAG, "Parsing did error: malformed did string \(did)")
            let didError = error as? DIDError
            var errmsg = "Parsing did error: malformed did string \(did)"
            if didError != nil {
                errmsg = DIDError.desription(didError!)
            }
            throw DIDError.malformedDID(errmsg)
        }
    }

    public var method: String {
        return _method!
    }

    func setMethod(_ method: String) {
        self._method = method
    }

    public var methodSpecificId: String {
        return _methodSpecificId!
    }

    func setMethodSpecificId(_ methodSpecificId: String) {
        self._methodSpecificId = methodSpecificId
    }

    public func getMetadata() -> DIDMeta {
        if  self._metadata == nil {
            self._metadata = DIDMeta()
        }
        return _metadata!
    }

    func setMetadata(_ newValue: DIDMeta) {
        self._metadata = newValue
    }

    public func saveMetadata() throws {
        if (_metadata != nil && _metadata!.attachedStore) {
            try _metadata?.store?.storeDidMetadata(self, _metadata!)
        }
    }

    public var isDeactivated: Bool {
        return getMetadata().isDeactivated
    }

    public func resolve(_ force: Bool) throws -> DIDDocument? {
        let doc = try DIDBackend.resolve(self, force)
        if doc != nil {
            setMetadata(doc!.getMetadata())
        }

        return doc
    }
    
    public func resolve() throws -> DIDDocument? {
        return try resolve(false)
    }

    public func resolveAsync(_ force: Bool) -> Promise<DIDDocument?> {
        return Promise<DIDDocument?> { resolver in
            do {
                resolver.fulfill(try resolve(force))
            } catch let error  {
                resolver.reject(error)
            }
        }
    }

    public func resolveAsync() -> Promise<DIDDocument?> {
        return resolveAsync(false)
    }


    public func resolveHistory() throws -> DIDHistory {
        return try DIDBackend.resolveHistory(self)
    }

    public func resolveHistoryAsync() -> Promise<DIDHistory> {
        return Promise<DIDHistory> { resolver in
            do {
                resolver.fulfill(try resolveHistory())
            } catch let error  {
                resolver.reject(error)
            }
        }
    }
}

extension DID: CustomStringConvertible {
    func toString() -> String {
        return String("did:\(_method!):\(_methodSpecificId!)")
    }

    public var description: String {
        return toString()
    }
}

extension DID: Equatable {
    func equalsTo(_ other: DID) -> Bool {
        return methodSpecificId == other.methodSpecificId
    }

    func equalsTo(_ other: String) -> Bool {
        return toString() == other
    }

    public static func == (lhs: DID, rhs: DID) -> Bool {
        return lhs.equalsTo(rhs)
    }

    public static func != (lhs: DID, rhs: DID) -> Bool {
        return !lhs.equalsTo(rhs)
    }
}

extension DID: Hashable {
    public func hash(into hasher: inout Hasher) {
        hasher.combine(self.toString())
    }
}

// Parse Listener
extension DID {
    private class Listener: DIDURLBaseListener {
        private var did: DID

        init(_ did: DID) {
            self.did = did
            super.init()
        }

        public override func exitMethod(_ ctx: DIDURLParser.MethodContext) {
            let method = ctx.getText()
            if (method != Constants.METHOD){
                // can't throw , print...
                Log.e(DID.TAG, "unsupported method: \(method)")
            }
            self.did._method = Constants.METHOD
        }

        public override func exitMethodSpecificString(
                            _ ctx: DIDURLParser.MethodSpecificStringContext) {
            self.did._methodSpecificId = ctx.getText()
        }
    }
}
