import Foundation

public class DID: NSObject {
    public static let METHOD: String = "elastos"

    public var method: String = ""
    public var methodSpecificId: String = ""
    public var meta: DIDMeta = DIDMeta()

    public override init() {
        super.init()
    }

    public init(_ method: String, _ methodSpecificId: String) {
        super.init()
        self.method = method
        self.methodSpecificId = methodSpecificId
    }

    public init(_ did: String) throws {
        super.init()
        try ParserHelper.parase(did, true, DListener(self))
    }
    
    public func setExtra(_ name: String, _ value: String) throws {
        meta.setExtra(name, value)
        if meta.attachedStore() {
            try meta.store!.storeDidMeta(self, meta)
        }
    }
    
    public func getExtra(_ name: String) -> String? {
        return meta.getExtra(name)
    }

    public var aliasName: String {
        get {
            return meta.alias
        }

        set {
            meta.alias = newValue
            if meta.attachedStore() {
                try? meta.store!.storeDidMeta(self, meta)
            }
        }
    }

    public var transactionId: String? {
        return meta.transactionId
    }

    public var updatedTimestamp: Date? {
        return meta.updated
    }

    public var isDeactivated: Bool {
        return meta.isDeactivated()
    }
    
    public func resolve(_ force: Bool) throws -> DIDDocument? {
        let doc = try DIDBackend.shareInstance().resolve(self, force)
        guard doc != nil else {  // TODO: can't make sure doc is not nil ?
            return nil
        }

        meta = doc!.meta
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
        guard object != nil else {
            return false;
        }

        guard !(object is DID) else {
            let did = object as! DID
            return did.description == description
        }

        guard !(object is String) else {
            let did = object as! String
            return did.isEqual(description)
        }
        
        return super.isEqual(object)
    }
}

extension DID: Comparable {
    public static func < (lhs: DID, rhs: DID) -> Bool {
        return lhs.isEqual(rhs)
    }
}

class DListener: DIDURLBaseListener {
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
