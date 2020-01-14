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
    
    init(_ adapter: DIDAdapter){
        self._adapter = adapter
        self._ttl = DIDBackend.DEFAULT_TTL
        super.init()
    }
    
    public static func creatInstance(_ adapter: DIDAdapter) {
        if instance != nil {
            return
        }
        instance = DIDBackend(adapter)
    }
    
    public static func shareInstance() -> DIDBackend {
        return instance!
    }

    public var didAdapter: DIDAdapter {
        get {
            return _adapter
        }
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
        
        let json = try _adapter.resolve(requestId, did.description, false)
        let string = JsonHelper.preHandleString(json)
        let resultJson = JsonHelper.handleString(string) as! OrderedDictionary<String, Any>
        let result: OrderedDictionary<String, Any> = resultJson[DIDBackend.RESULT] as! OrderedDictionary<String, Any>

        guard result.count != 0 else {
            throw DIDResolveError.failue("Resolve DID error .")
        }
        // Check response id, should equals requestId
        let id = resultJson[DIDBackend.ID] as? String
        guard id == requestId else {
            throw DIDResolveError.failue("Missmatched resolve result with request.")
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
        case ResolveResult.STATUS_EXPIRED: do {
            throw DIDExpiredError.failue("")
            }
        case ResolveResult.STATUS_DEACTIVATED: do {
            throw DIDDeactivatedError.failue("")
            }
        case ResolveResult.STATUS_NOT_FOUND: do {
            return nil
            }
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
            let request: IDChainRequest = try IDChainRequest.create(doc, signKey, storepass)
            let jsonString: String = request.toJson(true)
            
            return try _adapter.createIdTransaction(jsonString, nil)
        } catch  {
            throw DIDError.failue("Create ID transaction error: \(error.localizedDescription).")
        }
    }
    
    func update(_ doc: DIDDocument, previousTxid: String? = nil, _ signKey: DIDURL, _ storepass: String) throws -> String {
        do {
            let request: IDChainRequest = try IDChainRequest.update(doc, previousTxid: previousTxid, signKey, storepass)
            let jsonStr: String = request.toJson(true)
            return try _adapter.createIdTransaction(jsonStr, nil)
        } catch {
            throw  DIDError.failue("Create ID transaction error.")
        }
    }
    
    func deactivate(_ doc: DIDDocument, _ signKey: DIDURL, _ storepass: String) throws -> String {
        do {
            let request = try IDChainRequest.deactivate(doc, signKey, storepass)
            let json = request.toJson(true)
            return try _adapter.createIdTransaction(json, nil)
        }
        catch {
            throw DIDStoreError.failue("Create ID transaction error.")
        }
    }
    
    func deactivate(_ target: DID, _ targetSignKey: DIDURL, _ doc: DIDDocument, _ signKey: DIDURL, _ storepass: String) throws -> String {
        do {
            let request: IDChainRequest = try IDChainRequest.deactivate(target, targetSignKey, doc, signKey, storepass)
            let json = request.toJson(true)
            return try _adapter.createIdTransaction(json, nil)
            }
        catch {
            throw DIDStoreError.failue("Create ID transaction error.")
        }
    }
}
