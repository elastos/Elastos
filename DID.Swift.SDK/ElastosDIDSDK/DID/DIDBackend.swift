import Foundation

public class DIDBackend: NSObject {
    private static let ID = "id"
    private static let RESULT = "result"
    private static let ERROR = "error"
    private static let ERROR_CODE = "code"
    private static let ERROR_MESSAGE = "message"
    private static let DEFAULT_TTL: Int = 24 * 60 * 60 * 1000
    private static var instance: DIDBackend?

    private var _ttl: Int // milliseconds
    private var _adapter: DIDAdapter
    
    init(_ adapter: DIDAdapter, _ cacheDir: String) throws {
        self._adapter = adapter
        self._ttl = DIDBackend.DEFAULT_TTL
        try ResolverCache.setCacheDir(cacheDir)
        super.init()
    }

    public static func creatInstance(_ adapter: DIDAdapter, _ cacheDir: String) throws {
        if instance == nil {
            instance = try DIDBackend(adapter, cacheDir)
        }
    }
    
    public static func shareInstance() -> DIDBackend? {
        return instance
    }

    public var didAdapter: DIDAdapter {
        return _adapter
    }

    public var ttl: Int {
        get {
            return _ttl != 0 ? (_ttl / 60 / 1000) : 0
        }
        set {
            self._ttl = newValue > 0 ? (newValue * 60 * 1000) : 0
        }
    }
    
    private func generateRequestId() -> String {
        var str: String = ""
        while str.count < 16 {
            let random: Int = Int.randomCustom(min: 0, max: 16)
            let randomStr: String = Int.decTohex(number: random)
            str.append(randomStr)
        }
        return str
    }
    
    private func resolveFromBackend(_ did: DID) throws -> ResolveResult {
        let requestId = generateRequestId()

        let json = try? _adapter.resolve(requestId, did.description, false)
        guard json != nil else {
            throw DIDError.didResolveError(_desc: "Resolve DID error")
        }

        let string = JsonHelper.preHandleString(json!)
        let resultJson = JsonHelper.handleString(string) as! OrderedDictionary<String, Any>
        let result: OrderedDictionary<String, Any> = resultJson[DIDBackend.RESULT] as! OrderedDictionary<String, Any>

        guard result.count != 0 else {
            throw DIDError.didResolveError(_desc: "Resolve DID error")
        }
        // Check response ID is matched with request ID
        let id = resultJson[DIDBackend.ID] as? String
        guard id == requestId else {
            throw DIDError.didResolveError(_desc: "Mismatched resolve result with request")
        }

        let rr: ResolveResult = try ResolveResult.fromJson(result)
        if rr.status != ResolveResult.STATUS_NOT_FOUND {
            try ResolverCache.store(rr) 
        }
        return rr
    }
    
    func resolve(_ did: DID, _ force: Bool) throws -> DIDDocument? {
        var rr: ResolveResult?

        if !force {
            rr = try ResolverCache.load(did, ttl)
        }
        if (rr == nil) {
            rr = try resolveFromBackend(did)
        }

        switch rr!.status {
        case ResolveResult.STATUS_EXPIRED:
            throw DIDError.didExpiredError(_desc: "")

        case ResolveResult.STATUS_DEACTIVATED:
            throw DIDError.didDeactivatedError(_desc: "")

        case ResolveResult.STATUS_NOT_FOUND:
            return nil

        default:
            let ti: IDTransactionInfo = rr!.transactionInfo(atIndex:0)!
            let doc: DIDDocument = ti.request.doc!
            let meta: DIDMeta = DIDMeta()
            meta.transactionId = ti.transactionId
            meta.updated = ti.timestamp
            doc.meta = meta
            return doc
        }
    }
    
    public func resolve(_ did: DID) throws -> DIDDocument? {
        return try resolve(did, false)
    }

    func create(_ doc: DIDDocument, _ signKey: DIDURL, _ storepass: String) throws -> String {
        do {
            let request = try IDChainRequest.create(doc, signKey, storepass)
            return try _adapter.createIdTransaction(request.toJson(true), nil)
        } catch  {
            throw DIDError.failue("Create ID transaction error: \(error.localizedDescription).")
        }
    }
    
    func update(_ doc: DIDDocument, previousTxid: String? = nil, _ signKey: DIDURL, _ storepass: String) throws -> String {
        do {
            let request = try IDChainRequest.update(doc, previousTxid: previousTxid, signKey, storepass)
            return try _adapter.createIdTransaction(request.toJson(true), nil)
        } catch {
            throw  DIDError.failue("Update ID transaction error.")
        }
    }
    
    func deactivate(_ doc: DIDDocument, _ signKey: DIDURL, _ storepass: String) throws -> String {
        do {
            let request = try IDChainRequest.deactivate(doc, signKey, storepass)
            return try _adapter.createIdTransaction(request.toJson(true), nil)
        } catch {
            throw DIDError.didStoreError(_desc: "Deactivate ID transaction error.")
        }
    }
    
    func deactivate(_ target: DID, _ targetSignKey: DIDURL, _ doc: DIDDocument, _ signKey: DIDURL, _ storepass: String) throws -> String {
        do {
            let request = try IDChainRequest.deactivate(target, targetSignKey, doc, signKey, storepass)
            return try _adapter.createIdTransaction(request.toJson(true), nil)
        } catch {
            throw DIDError.didStoreError(_desc: "Deactivate ID transaction error.")
        }
    }
}
