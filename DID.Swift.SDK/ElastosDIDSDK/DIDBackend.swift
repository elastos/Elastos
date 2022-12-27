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

public class DIDBackend {
    private static let TAG = "DIDBackend"
    private static var resolver: DIDResolver?
    private static var _ttl: Int = Constants.DEFAULT_TTL // milliseconds
    private var _adapter: DIDAdapter
    public typealias ResolveHandle = (_ did: DID) throws -> DIDDocument?
    private static var resolveHandle: ResolveHandle? = nil

    class TransactionResult: NSObject {
        private var _transactionId: String?
        private var _status: Int
        private var _message: String?
        private var _filled: Bool
        private let _semaphore: DispatchSemaphore

        override init() {
            self._status = 0
            self._filled = false
            self._semaphore = DispatchSemaphore(value: 0)
        }

        func update(_ transactionId: String, _ status: Int, _ message: String?) {
            self._transactionId = transactionId
            self._status = status
            self._message = message
            self._filled = true
            self._semaphore.signal()
        }

        func update(_ transactionId: String) {
            update(transactionId, 0, nil)
        }

        var transactionId: String {
            return _transactionId!
        }

        var status: Int {
            return _status
        }

        var message: String? {
            return _message
        }

        var isEmpty: Bool {
            return !_filled
        }

        override var description: String {
            var str = ""

            str.append("txid: ")
            str.append(transactionId)
            str.append("status: ")
            str.append(String(status))

            if status != 0 {
                str.append("message: ")
                str.append(message!)
            }

            return str
        }
    }

    class DefaultResolver: DIDResolver {
        private var url: URL

        init(_ resolver: String) throws {
            guard !resolver.isEmpty else {
                throw DIDError.illegalArgument()
            }
            url = URL(string: resolver)!
        }

        func resolve(_ requestId: String, _ did: String, _ all: Bool) throws -> Data {
            Log.i(TAG, "Resolving {}...\(did.description)")

            var request = URLRequest.init(url: url, cachePolicy: .useProtocolCachePolicy, timeoutInterval: 60)
            request.httpMethod = "POST"
            request.setValue("application/json", forHTTPHeaderField: "Content-Type")
            request.setValue("application/json", forHTTPHeaderField: "Accept")

            let parameters: [String: Any] = [
                "jsonrpc": "2.0",
                "method": "resolvedid",
                "params": ["did":did, "all": all],
                "id": requestId
            ]

            do {
                request.httpBody = try JSONSerialization.data(withJSONObject: parameters, options: .prettyPrinted)
            } catch {
                throw DIDError.illegalArgument()
            }

            let semaphore = DispatchSemaphore(value: 0)
            var errDes: String?
            var result: Data?

            let task = URLSession.shared.dataTask(with: request) { data, response, error in
                guard let _ = data,
                    let response = response as? HTTPURLResponse,
                    error == nil else { // check for fundamental networking error

                        errDes = error.debugDescription
                        semaphore.signal()
                        return
                }
                guard (200 ... 299) ~= response.statusCode else { // check for http errors
                    errDes = "Server eror (status code: \(response.statusCode)"
                    semaphore.signal()
                    return
                }

                result = data
                semaphore.signal()
            }

            task.resume()
            semaphore.wait()

            guard let _ = result else {
                throw DIDError.didResolveError(errDes ?? "Unknown error")
            }

            return result!
        }
    }

    init(_ adapter: DIDAdapter) {
        self._adapter = adapter
    }

    /// Initialize DIDBackend to resolve by url.
    /// - Parameters:
    ///   - resolverURL: The URL string.
    ///   - cacheDir: The directory for cache.
    /// - Throws:  if an error occurred, throw error.
    public class func initializeInstance(_ resolverURL: String, _ cacheDir: String) throws {
        guard !resolverURL.isEmpty, !cacheDir.isEmpty else {
            throw DIDError.illegalArgument()
        }

        try initializeInstance(DefaultResolver(resolverURL), cacheDir)
    }

    /// Initialize DIDBackend to resolve by url.
    /// - Parameters:
    ///   - resolverURL: The URL string.
    ///   - cacheDir: The directory for cache.
    /// - Throws: if an error occurred, throw error.
    public class func initializeInstance(_ resolverURL: String, _ cacheDir: URL) throws {
        guard !resolverURL.isEmpty, !cacheDir.isFileURL else {
            throw DIDError.illegalArgument()
        }

        try initializeInstance(DefaultResolver(resolverURL), cacheDir)
    }

    /// Initialize DIDBackend to resolve by DIDResolver.
    /// - Parameters:
    ///   - resolver: The handle to DIDResolver.
    ///   - cacheDir: The directory for cache.
    /// - Throws: if an error occurred, throw error.
    public class func initializeInstance(_ resolver: DIDResolver, _ cacheDir: String) throws {
        guard !cacheDir.isEmpty else {
            throw DIDError.illegalArgument()
        }

        do {
            DIDBackend.resolver = resolver
            try ResolverCache.setCacheDir(cacheDir)
        } catch {
            throw DIDError.illegalArgument()
        }
    }

    /// Initialize DIDBackend to resolve by DIDResolver.
    /// - Parameters:
    ///   - resolver: The handle to DIDResolver.
    ///   - cacheDir: The directory for cache.
    /// - Throws: if an error occurred, throw error.
    public class func initializeInstance(_ resolver: DIDResolver, _ cacheDir: URL) throws {
        guard !cacheDir.isFileURL else {
            throw DIDError.illegalArgument()
        }

        do {
            DIDBackend.resolver = resolver
            try ResolverCache.setCacheDir(cacheDir)
        } catch {
            throw DIDError.illegalArgument()
        }
    }

    /// Get DIDBackend instance.
    /// - Parameter adapter: A handle to DIDAdapter.
    /// - Returns: DIDBackend instance.
    public class func getInstance(_ adapter: DIDAdapter) -> DIDBackend {
        return DIDBackend(adapter)
    }

    class func getTtl() -> Int {
        return _ttl != 0 ? (_ttl / 60 / 1000) : 0
    }

    class func setTtl(_ newValue: Int) {
        self._ttl = newValue > 0 ? (newValue * 60 * 1000) : 0
    }

    private class func generateRequestId() -> String {
        var requestId = ""
        while requestId.count < 16 {
            let randomStr = Int.decTohex(number: Int.randomCustom(min: 0, max: 16))
            requestId.append(randomStr)
        }
        return requestId
    }

    private class func resolveFromBackend(_ did: DID, _ all: Bool) throws -> ResolveResult {
        let requestId = generateRequestId()

        guard let _ = resolver else {
            throw DIDError.didResolveError("DID resolver not initialized")
        }

        let data = try resolver!.resolve(requestId, did.toString(), all)
        let dict: [String: Any]?
        do {
            dict = try JSONSerialization.jsonObject(with: data, options: []) as? [String: Any]
        } catch {
            throw DIDError.didResolveError("parse resolved json error.")
        }

        guard let _ = dict else {
            throw DIDError.didResolveError("invalid json format")
        }

        let node = JsonNode(dict!)
        let id = node.get(forKey: Constants.ID)?.asString()
        guard let _ = id else {
            throw DIDError.didResolveError("missing resolved result id")
        }
        guard id! == requestId else {
            throw DIDError.didResolveError("mismatched request Id for resolved result")
        }

        let resultNode = node.get(forKey: Constants.RESULT)
        guard let _ = resultNode, !resultNode!.isEmpty else {
            let errorNode = node.get(forKey: Constants.ERROR)
            let errorCode = errorNode?.get(forKey: Constants.ERROR_CODE)?.asString() ?? "<null>"
            let errorMsg  = errorNode?.get(forKey: Constants.ERROR_MESSAGE)? .asString() ?? "<null>"

            throw DIDError.didResolveError("resolve DID error(\(errorCode)):\(errorMsg)")
        }

        let result = try ResolveResult.fromJson(resultNode!)
        if result.status != ResolveResultStatus.STATUS_NOT_FOUND {
            do {
                try ResolverCache.store(result)
            } catch {
                NSLog("!!! cache resolved result error: \(DIDError.desription(error as! DIDError))")
            }
        }

        return result
    }

     class func resolveHistory(_ did: DID) throws -> DIDHistory {
        Log.i(TAG, "Resolving {}...\(did.toString())")
        let rr = try resolveFromBackend(did, true)
        guard rr.status != ResolveResultStatus.STATUS_NOT_FOUND else {
            throw DIDError.didResolveError()
        }
        return rr
     }

    ///  User set DID Local Resolve handle in order to give which did document to verify.
    ///  If handle != NULL, set DID Local Resolve Handle.
    /// - Parameter handle: A handle to ResolveHandle.
    public class func setResolveHandle(_ handle: @escaping ResolveHandle) {
        DIDBackend.resolveHandle = handle
    }

    class func resolve(_ did: DID, _ force: Bool) throws -> DIDDocument? {
        Log.i(TAG, "Resolving {\(did.toString())} ...")

        guard DIDBackend.resolveHandle == nil else {
           return try resolveHandle!(did)
        }

        var result: ResolveResult?
        if (!force) {
            result = try ResolverCache.load(did, _ttl)
            Log.d(TAG, "Try load {\(did.toString())} from resolver cache");
        }

        if  result == nil {
            result = try resolveFromBackend(did, false)
        }

        switch result!.status {
            // When DID expired, we should also return the document.
            // case .STATUS_EXPIRED:
            //     throw DIDError.didExpired()

        case .STATUS_DEACTIVATED:
            throw DIDError.didDeactivated()

        case .STATUS_NOT_FOUND:
            return nil

        default:

            let transactionInfo = try result!.transactionInfo(0)
            let doc = transactionInfo?.request.document
            let meta = DIDMeta()

            meta.setTransactionId(transactionInfo!.transactionId)
            meta.setSignature(doc!.proof.signature)
            meta.setPublished(transactionInfo!.getTimestamp())
            meta.setLastModified(transactionInfo!.getTimestamp())
            doc!.setMetadata(meta)
            return doc
        }
    }

    class func resolve(_ did: DID) throws -> DIDDocument? {
        return try resolve(did, false)
    }

    func getAdapter() -> DIDAdapter {
        return _adapter
    }

    private func createTransaction(_ payload: String, _ memo: String?) throws {
        Log.i(DIDBackend.TAG, "Create ID transaction...")
        Log.d(DIDBackend.TAG, "Transaction paload: '{\(payload)}', memo: {\(memo ?? "none")}}")

        try _adapter.createIdTransaction(payload, memo)
        Log.d(DIDBackend.TAG, "ID transaction complete.")
    }

    func create(_ doc: DIDDocument,
                _ signKey: DIDURL,
                _ storePassword: String) throws {

        let request = try IDChainRequest.create(doc, signKey, storePassword)
        return try createTransaction(request.toJson(true), nil)
    }

    func update(_ doc: DIDDocument,
                _ previousTransactionId: String,
                _ signKey: DIDURL,
                _ storePassword: String) throws {

        let request = try IDChainRequest.update(doc, previousTransactionId, signKey, storePassword)
        try createTransaction(request.toJson(true), nil)
        try ResolverCache.invalidate(doc.subject)
    }

    func deactivate(_ doc: DIDDocument,
                    _ signKey: DIDURL,
                    _ storePassword: String) throws {

        let request = try IDChainRequest.deactivate(doc, signKey, storePassword)
        try createTransaction(request.toJson(true), nil)
        try ResolverCache.invalidate(doc.subject)
    }

    func deactivate(_ target: DID,
                    _ targetSignKey: DIDURL,
                    _ doc: DIDDocument,
                    _ signKey: DIDURL,
                    _ storePassword: String) throws {

        let request = try IDChainRequest.deactivate(target, targetSignKey, doc, signKey, storePassword)
        try createTransaction(request.toJson(true), nil)
        try ResolverCache.invalidate(doc.subject)
    }
}
