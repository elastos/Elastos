import Foundation

public class DID: NSObject {

    public static let METHOD: String = "elastos"
    var method: String!
    public var methodSpecificId: String!
    private var document: DIDDocument?
    private var reslved: Bool?
    private var resolveTimestamp: Date?

    init(_ method: String, _ methodSpecificId: String) {
        self.method = method
        self.methodSpecificId = methodSpecificId
    }

    public init(_ did: String) throws {
       try ParserHelper.parase(did, true, Listener())
    }

    public func toExternalForm() -> String {
        return String("did:\(method):\(methodSpecificId)")
    }

    public func toString() -> String {
        return toExternalForm()
    }

    override public var hash: Int {
        return DID.METHOD.hashValue + methodSpecificId.hashValue
    }

    public override func isEqual(_ object: Any?) -> Bool {
        return true
    }

    public func resolve() throws -> DIDDocument {
        if document != nil {return document!}
        do {
            document = try DIDStore.shareInstance()!.resolveDid(self)
        } catch {
            throw error
        }
        return document!
    }
}
