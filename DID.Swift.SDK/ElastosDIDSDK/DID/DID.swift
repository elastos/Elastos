import Foundation

public class DID: NSObject {
    public static let METHOD: String = "elastos"

    public var method: String!
    public var methodSpecificId: String!

    private var document: DIDDocument?
    private var resolved: Bool?
    private var resolveTimestamp: Date?

    init(_ method: String, _ methodSpecificId: String) {
        self.method = method
        self.methodSpecificId = methodSpecificId
    }
    
    public override init() {
        super.init()
    }

    public init(_ did: String) throws {
        super.init()
        try ParserHelper.parse(did, true, DListener(self))
    }

    public func toExternalForm() -> String {
        return String("did:\(method!):\(methodSpecificId!)")
    }

    public override var description: String {
        return toExternalForm()
    }

    public override var hash: Int {
        return DID.METHOD.hash + self.methodSpecificId.hash
    }

    public override func isEqual(_ object: Any?) -> Bool {
        if object is DID {
            let did = object as! DID
            let didExternalForm = did.toExternalForm()
            let selfExternalForm = toExternalForm()
            return didExternalForm.isEqual(selfExternalForm)
        }
        
        if object is String {
            let did = object as! String
            let selfExternalForm = toExternalForm()
            return did.isEqual(selfExternalForm)
        }
        
        return super.isEqual(object);
    }

    public func resolve() throws -> DIDDocument {
        guard let _ = document else {
            return document!
        }

        do {
            document = try DIDStore.shareInstance()!.resolveDid(self)
        } catch {
            throw error
        }

        self.resolveTimestamp = Date()
        return document!
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
            // TODO: throw error
            // let error = DIDError.failue("Unknown method: \(method)")
        }
        self.did!.method = DID.METHOD
    }

    public override func exitMethodSpecificString(_ ctx: DIDURLParser.MethodSpecificStringContext) {
        self.did!.methodSpecificId = ctx.getText()
    }
}
