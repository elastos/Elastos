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
        // operation
        let op: String = "\(operation ?? Operation.CREATE)"
        
        // payload: did or doc
        if operation == Operation.DEACRIVATE {
            payload = did?.toExternalForm()
        } else {
            payload = try doc?.toExternalForm(true)
            payload = payload?.toBase64()
        }
        
        let inputs: [String] = [
            specification,
            op,
            payload!
        ]
        let count = inputs.count / 3
        self.signature = try DIDStore.shareInstance()?.sign(self.did!, key, passphrase, count, inputs)
        self.signKey = key
        self.keyType = Constants.defaultPublicKeyType
        return self
    }

    func verify() throws -> Bool {
        let op: String = operation.toString()
        let inputs = [specification, op, payload]
        let count = inputs.count / 3
        return try (doc?.verify(signKey!, signature!, count, inputs as! [CVarArg]))!
    }

    public func toJson(_ compact: Bool) -> String {
        var json: OrderedDictionary<String, Any> = OrderedDictionary()
        // header
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        dic[IDChainRequest.SPECIFICATION] = specification
        dic[IDChainRequest.OPERATION] = operation.toString()
        json[IDChainRequest.HEADER] = dic
        
        // playload
        json[IDChainRequest.PAYLOAD] = payload
        
        // signature
        var keyId: String
        dic.removeAll(keepCapacity: 0)
        if !compact {
            dic[IDChainRequest.KEY_TYPE] = keyType
            keyId = (signKey?.toExternalForm())!
        }
        else {
            keyId = "#" + signKey!.fragment
        }
        dic[IDChainRequest.KEY_ID] = keyId
        dic[IDChainRequest.SIGNATURE] = signature
        json[IDChainRequest.PROOF] = dic
        
        let jsonString: String = JsonHelper.creatJsonString(dic: json)
        return jsonString
    }
    
    public class func fromJson(_ json: OrderedDictionary<String, Any>) throws -> IDChainRequest {
        let header = json[HEADER] as! OrderedDictionary<String, Any>
        let spec: String = try JsonHelper.getString(header, SPECIFICATION, false, nil, SPECIFICATION)
        guard (spec == CURRENT_SPECIFICATION) else {
            throw DIDError.failue("Missing header.")
        }
        let op: String = try JsonHelper.getString(header, OPERATION, false, nil, OPERATION)
        if op != Operation.CREATE.toString() {
            guard spec == CURRENT_SPECIFICATION else {
                throw DIDError.failue("Invalid DID operation verb.")
            }
        }
        
        let payload: String = try JsonHelper.getString(json, PAYLOAD, false, nil, PAYLOAD)
        
        let buffer: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: 100)
        let cp = payload.withCString { re -> UnsafePointer<Int8> in
            return re
        }
        let _ = base64_url_decode(buffer, cp)
        let docJson: String = String(cString: buffer)
        var doc: DIDDocument
        var request: IDChainRequest
        do {
            doc = try DIDDocument.fromJson(json: docJson)
            let proof = json[PROOF] as! OrderedDictionary<String, Any>
            guard proof.count != 0 else {
                throw DIDError.failue("Missing proof.")
            }
            let keyType: String = try JsonHelper.getString(proof, KEY_TYPE, false, nil, KEY_TYPE)
            guard keyType == Constants.defaultPublicKeyType else {
                throw DIDError.failue("Unknown signature key type.")
            }
            let signKey: DIDURL = try JsonHelper.getDidUrl(proof, KEY_ID, doc.subject!, KEY_ID)
            guard doc.getAuthenticationKey(signKey) != nil else {
                throw DIDError.failue("Unknown signature key.")
            }
            let sig: String = try JsonHelper.getString(proof, SIGNATURE, false, nil, SIGNATURE)
            request = try IDChainRequest(Operation.CREATE, doc)
            request.payload = payload
            request.keyType = keyType
            request.signKey = signKey
            request.signature = sig
            return request
        } catch {
            print(error)
            throw DIDError.failue("\(error)")
        }
    }
    
}
