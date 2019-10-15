import Foundation

class IDChainRequest: NSObject {
    
    private static let CURRENT_SPECIFICATION: String = "elastos/did/1.0"
    
    private static let HEADER: String = "header"
    private static let SPECIFICATION: String = "specification"
    private static let OPERATION: String = "operation"
    private static let PAYLOAD: String = "payload"
    private static let PROOF: String = Constants.proof
    private static let KEY_TYPE: String = Constants.type;
    private static let KEY_ID: String = Constants.verificationMethod;
    private static let SIGNATURE: String = Constants.signature;
    
    enum Operation: Int {
        case CREATE = 0
        case UPDATE
        case DEACRIVATE
        
        public func toString() -> String {
            if self.rawValue == 2 {
                return "create"
            } else if self.rawValue == 1{
                return "update"
            } else {
                return "deacrivate"
            }
        }
    }
    
    // header
    public var specification: String!
    public var operation: Operation!
    
    // payload
    public var did: DID?
    public var doc: DIDDocument?
    public var payload: String?
    
    // signature
    public var signKey: DIDURL?;
    public var keyType: String?;
    public var signature: String?;
    
    
    public init(_ op: Operation, _ did_: DID) throws {
        if op != Operation.DEACRIVATE {
            throw DIDError.failue("Operation need a DIDDocument.")
        }
        
        specification = IDChainRequest.CURRENT_SPECIFICATION
        operation = op
        did = did_
    }
    
    public init(_ op: Operation, _ doc_: DIDDocument) throws {
        specification = IDChainRequest.CURRENT_SPECIFICATION
        operation = op
        did = doc?.subject
        doc = doc_
    }
    
    public func sign(_ key: DIDURL, _ passphrase: String) throws -> IDChainRequest {
        
        let op: String = "\(operation ?? Operation.CREATE)"
        
        if operation == Operation.DEACRIVATE {
            payload = did?.toExternalForm()
        } else {
            payload = try doc?.toExternalForm(true)
        }
        
        payload = payload?.toBase64()
        
        let inputs: [String] = [
            specification,
            op,
            payload!
        ]
        
        self.signature = try DIDStore.shareInstance()?.sign(self.did!, key, passphrase, inputs)
        self.signKey = key
        self.keyType = Constants.defaultPublicKeyType
        return self
    }
}
