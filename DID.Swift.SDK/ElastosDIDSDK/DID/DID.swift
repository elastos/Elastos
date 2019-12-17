import Foundation

public class DID: NSObject {
    public static let METHOD: String = "elastos"

    public var method: String!
    public var methodSpecificId: String!
    public var alias: String = ""

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

    public func setAlias(_ alias: String) throws {
        if DIDStore.isInitialized() {
           try DIDStore.shareInstance()!.storeDidAlias(self, alias)
        }
        self.alias = alias
    }
    
    public func getAlias() throws -> String {
        var al = alias
        if (alias == "") {
            if (DIDStore.isInitialized()){
                al = try DIDStore.shareInstance()!.loadDidAlias(self)
            }
        }

        return al
    }
    
    public func toExternalForm() -> String {
        return String("did:\(method!):\(methodSpecificId!)")
    }

    public override var description: String {
        return toExternalForm()
    }

    public override var hash: Int {
        return (DID.METHOD + self.methodSpecificId!).hash
    }

    public override func isEqual(_ object: Any?) -> Bool {
        if object is DID {
            let did = object as! DID
            return did.toExternalForm().isEqual(toExternalForm())
        }
        
        if object is String {
            let did = object as! String
            return did.isEqual(toExternalForm())
        }
        
        return super.isEqual(object)
    }

    public func resolve() throws -> DIDDocument {
        return try DIDStore.shareInstance()!.resolveDid(self)!
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
