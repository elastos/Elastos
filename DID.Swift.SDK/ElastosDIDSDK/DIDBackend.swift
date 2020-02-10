import Foundation

public class DIDBackend {
    private static var instance: DIDBackend?

    private var _ttl: Int // milliseconds
    private var _adapter: DIDAdapter
    
    init(_ adapter: DIDAdapter){
        self._adapter = adapter
        self._ttl = Constants.DEFAULT_TTL
    }
    
    public static func initializeInstance(_ adapter: DIDAdapter) {
        if instance == nil {
            instance = DIDBackend(adapter)
        }
    }

    public static func shareInstance() -> DIDBackend? {
        return instance
    }

    public var didAdapter: DIDAdapter {
        return self._adapter
    }

    public var ttl: Int {
        return self._ttl != 0 ? (self._ttl / 60 / 1000) : 0
    }

    public func setTtl(_ newValue: Int) {
        self._ttl = newValue > 0 ? (newValue * 60 * 1000) : 0
    }
    
    private func generateRequestId() -> String {
        var requestId = ""
        while requestId.count < 16 {
            let randomStr = Int.decTohex(number: Int.randomCustom(min: 0, max: 16))
            requestId.append(randomStr)
        }
        return requestId
    }
    
    private func resolveFromBackend(_ did: DID) throws -> ResolveResult {
        let json = try? self._adapter.resolve(generateRequestId(), did.description, false)
        guard json != nil else {
            throw DIDError.didResolveError("Resolve DID error")
        }

        // TODO:
    }
    
    func resolve(_ did: DID, _ force: Bool) throws -> DIDDocument? {
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
            doc?.setMeta(meta)
            return doc
        }
    }
    
    public func resolve(_ did: DID) throws -> DIDDocument? {
        return try resolve(did, false)
    }

    func create(_ doc: DIDDocument,
                _ signKey: DIDURL,
                _ storePass: String) throws -> String {
        do {
            let request = try IDChainRequest.create(doc, signKey, storePass)
            return try self._adapter.createIdTransaction(request.description, nil)
        } catch  {
            throw DIDError.didStoreError("Create ID transaction error.")
        }
    }
    
    func update(_ doc: DIDDocument,
                _ previousTxId: String,
                _ signKey: DIDURL,
                _ storePass: String) throws -> String {
        do {
            let request = try IDChainRequest.update(doc, previousTxId, signKey, storePass)
            return try self._adapter.createIdTransaction(request.description, nil)
        } catch {
            throw  DIDError.didStoreError("Update ID transaction error.")
        }
    }
    
    func deactivate(_ doc: DIDDocument,
                _ signKey: DIDURL,
                _ storepass: String) throws -> String {
        do {
            let request = try IDChainRequest.deactivate(doc, signKey, storepass)
            return try self._adapter.createIdTransaction(request.description, nil)
        } catch {
            throw DIDError.didStoreError("Deactivate ID transaction error.")
        }
    }
    
    func deactivate(_ target: DID,
                _ targetSignKey: DIDURL,
                _ doc: DIDDocument,
                _ signKey: DIDURL,
                _ storePass: String) throws -> String {
        do {
            let request = try IDChainRequest.deactivate(target, targetSignKey, doc, signKey, storePass)
            return try self._adapter.createIdTransaction(request.description, nil)
        } catch {
            throw DIDError.didStoreError("Deactivate ID transaction error.")
        }
    }
}
