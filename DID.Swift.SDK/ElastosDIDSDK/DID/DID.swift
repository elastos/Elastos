import Foundation

public class DID: NSObject {
    public static let METHOD: String = "elastos"

    public var method: String = ""
    public var methodSpecificId: String = ""
    public var meta: DIDMeta = DIDMeta()

    public init(_ method: String, _ methodSpecificId: String) {
        self.method = method
        self.methodSpecificId = methodSpecificId
    }
    
    public override init() {
        super.init()
    }

    public init(_ did: String) throws {
        super.init()
        try ParserHelper.parase(did, true, DListener(self))
    }
    
    public func setExtra(_ name: String, _ value: String) throws {
        self.meta.setExtra(name, value)
        if meta.attachedStore() {
            try meta.store!.storeDidMeta(self, meta)
        }
    }
    
    public func getExtra(_ name: String) -> String? {
        return meta.getExtra(name)
    }

    public func setAlias(_ alias: String) throws {
        meta.alias = alias
        if meta.attachedStore() {
            try meta.store!.storeDidMeta(self, meta)
        }
    }
    
    public func getAlias() -> String {
        return meta.alias
    }
    
    public func getTransactionId() -> String {
        return meta.transactionId
    }
    
    public func getUpdated() -> Date? {
        return meta.updated
    }
    
    public func isDeactivated() -> Bool {
        return meta.isDeactivated()
    }
    
    public func resolve(_ force: Bool) throws -> DIDDocument? {
        let doc = try DIDBackend.shareInstance().resolve(self, force)
        if doc != nil {
            meta = doc!.meta
        }
        return doc
    }
    
    public func resolve() throws -> DIDDocument? {
        return try resolve(false)
    }
    
    public override var description: String {
        return String("did:\(method):\(methodSpecificId)")
    }
    
    public override var hash: Int {
        return (DID.METHOD + self.methodSpecificId).hash
    }

    public override func isEqual(_ object: Any?) -> Bool {
        if object is DID {
            let did = object as! DID
            return did.description == description
        }
        
        if object is String {
            let did = object as! String
            return did.isEqual(description)
        }
        
        return super.isEqual(object)
    }
}

class DListener: DIDURLBaseListener {
    public var name: String?
    public var value: String?
    public var did: DID?

    init(_ did: DID) {
        super.init()
        self.did = did
    }
    
    public override func exitMethod(_ ctx: DIDURLParser.MethodContext) {
        let method: String = ctx.getText()
        if (method != DID.METHOD){
            // can't throw , print...
            print(DIDError.failue("Unknown method: \(method)"))
        }
        self.did!.method = DID.METHOD
    }
    
    public override func exitMethodSpecificString(_ ctx: DIDURLParser.MethodSpecificStringContext) {
        self.did!.methodSpecificId = ctx.getText()
    }
}
