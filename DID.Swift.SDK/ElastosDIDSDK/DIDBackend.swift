import Foundation

public class DIDBackend {
    private static var resolver: DIDResolver?

    private var _ttl: Int = Constants.DEFAULT_TTL // milliseconds
    private var _adapter: DIDAdapter
    
    class TransactionResult {
        private var _transactionId: String?
        private var _status: Int
        private var _message: String?
        private var _filled: Bool
        private let _condition: NSCondition

        init() {
            self._status = 0
            self._filled = false
            self._condition = NSCondition()
        }

        func update(_ transactionId: String, _ status: Int, _ message: String?) {
            self._transactionId = transactionId
            self._status = status
            self._message = message
            self._filled = true

            self._condition.signal()
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
    }

    class DefaultResolver: DIDResolver {
        init(_ resolver: String) throws {
            // TODO:
        }

        func resolve(_ requestId: String, _ did: String, _ all: Bool) throws -> Data {
            // TODO:
            return Data()
        }
    }

    init(_ adapter: DIDAdapter) {
        self._adapter = adapter
    }

    class func initializeInstance(_ resolverURL: String, _ file: FileHandle) throws {
        // TODO:
    }

    class func initializeInstance(_ resolverURL: String, _ cacheDir: String) throws {
        // TODO
    }

    class func initializeInstance(_ resolver: DIDResolver, _ file: FileHandle) throws {
        // TODO:
    }

    class func initializeInstance(_ resolver: DIDResolver, _ cacheDir: String) throws {
        // TODO:
    }

    class func getInstance(_ adapter: DIDAdapter) -> DIDBackend {
        return DIDBackend(adapter)
    }

    func getTtl() -> Int {
        return self._ttl != 0 ? (self._ttl / 60 / 1000) : 0
    }

    func setTtl(_ newValue: Int) {
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
    
    private class func resolveFromBackend(_ did: DID) throws -> ResolveResult {
        let requestId = generateRequestId()

        guard let _ = DIDBackend.resolver else {
            throw DIDError.didResolveError("DID resolver not initialized")
        }

        let data: Data
        do {
            data = try DIDBackend.resolver!.resolve(requestId, did.toString(), false)
        } catch {
            throw DIDError.didResolveError("Unkown error")
        }

        let dict: Dictionary<String, Any>?
        do {
            dict = try JSONSerialization.jsonObject(with: data, options: []) as? Dictionary<String, Any>
        } catch {
            throw DIDError.didResolveError("parse resolved json error.")
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
        if  resultNode == nil || resultNode!.isEmpty {
            let errorNode = node.get(forKey: Constants.ERROR)!
            let errorCode = errorNode.get(forKey: Constants.ERROR_CODE)?.asString() ?? "<null>"
            let errorMsg  = errorNode.get(forKey: Constants.ERROR_MESSAGE)? .asString() ?? "<null>"

            throw DIDError.didResolveError("resolve DID error(\(errorCode)):\(errorMsg)")
        }

        let result = try ResolveResult.fromJson(resultNode!)
        if result.status != ResolveResultStatus.STATUS_NOT_FOUND {
            // TODO: Cache.
        }
        return result
    }
    
    class func resolve(_ did: DID, _ force: Bool) throws -> DIDDocument? {
        let result = try resolveFromBackend(did)

        switch result.status {
        case .STATUS_EXPIRED:
            throw DIDError.didExpired()

        case .STATUS_DEACTIVATED:
            throw DIDError.didDeactivated()

        case .STATUS_NOT_FOUND:
            return nil

        default:
            let transactionInfo = result.transactionInfo(0)
            let doc = transactionInfo?.request.document
            let meta = DIDMeta()

            meta.setTransactionId(transactionInfo!.transactionId)
            meta.setUpdatedDate(transactionInfo!.timestamp)
            doc!.setMeta(meta)
            return doc
        }
    }
    
    public class func resolve(_ did: DID) throws -> DIDDocument? {
        return try resolve(did, false)
    }

    func getAdapter() -> DIDAdapter {
        return _adapter
    }

    private func createTransaction(_ payload: String, _ memo: String?, _ confirms: Int) throws -> String {
        let transResult = TransactionResult()
        let condition = NSCondition()

        _adapter.createIdTransaction(payload, memo, confirms) { (txid, status, message) -> Void in
            transResult.update(txid, status, message)
            condition.signal()
        }

        condition.wait()

        if transResult.status != 0 {
            throw DIDError.transactionError(
                "create transaction failed (\(transResult.status):)\(transResult.message ?? "")")
        }

        return transResult.transactionId
    }

    func create(_ doc: DIDDocument,
                _ signKey: DIDURL,
                _ storePassword: String) throws -> String {
        return try create(doc, 0, signKey, storePassword)
    }

    func create(_ doc: DIDDocument,
                _ confirms: Int,
                _ signKey: DIDURL,
                _ storePassword: String) throws -> String {

        let request = try IDChainRequest.create(doc, signKey, storePassword)
        return try createTransaction(request.toJson(true), nil, confirms)
    }

    func update(_ doc: DIDDocument,
                _ previousTransactionId: String,
                _ signKey: DIDURL,
                _ storePassword: String) throws -> String {
        return try update(doc, previousTransactionId, 0, signKey, storePassword)
    }
    
    func update(_ doc: DIDDocument,
                _ previousTransactionId: String,
                _ confirms: Int,
                _ signKey: DIDURL,
                _ storePassword: String) throws -> String {

        let request = try IDChainRequest.update(doc, previousTransactionId, signKey, storePassword)
        return try createTransaction(request.toJson(true), nil, confirms)
    }

    func deactivate(_ doc: DIDDocument,
                _ signKey: DIDURL,
                _ storePassword: String) throws -> String {
        return try deactivate(doc, 0, signKey, storePassword)
    }
    
    func deactivate(_ doc: DIDDocument,
                _ confirms: Int,
                _ signKey: DIDURL,
                _ storePassword: String) throws -> String {

        let request = try IDChainRequest.deactivate(doc, signKey, storePassword)
        return try createTransaction(request.toJson(true), nil, confirms)
    }

    func deactivate(_ target: DID,
                _ targetSignKey: DIDURL,
                _ doc: DIDDocument,
                _ signKey: DIDURL,
                _ storePassword: String) throws -> String {
        return try deactivate(target, targetSignKey, doc, 0, signKey, storePassword)
    }
    
    func deactivate(_ target: DID,
                _ targetSignKey: DIDURL,
                _ doc: DIDDocument,
                _ confirms: Int,
                _ signKey: DIDURL,
                _ storePassword: String) throws -> String {

        let request = try IDChainRequest.deactivate(target, targetSignKey, doc, signKey, storePassword)
        return try createTransaction(request.toJson(true), nil, confirms)
    }
}
