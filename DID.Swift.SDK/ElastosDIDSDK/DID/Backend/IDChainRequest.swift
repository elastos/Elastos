import Foundation

public class IDChainRequest: NSObject {
    public static let CURRENT_SPECIFICATION = "elastos/did/1.0"
    
    private static let HEADER = "header"
    private static let SPECIFICATION = "specification"
    private static let OPERATION = "operation"
    private static let PREVIOUS_TXID = "previousTxid"
    private static let PAYLOAD = "payload"
    private static let KEY_TYPE = TYPE
    private static let VERIFICATION_METHOD = "verificationMethod"
    private static let SIGNATURE = "signature"
    // private static let KEY_ID: String = VERIFICATION_METHOD
    
    // header
    public var specification: String = ""
    public var operation: IDChainRequest.Operation
    public var previousTxid: String = ""

    // payload
    public var did: DID?
    public var doc: DIDDocument?
    public var payload: String = ""
    
    // signature
    public var keyType: String = ""
    public var signKey: DIDURL?
    public var signature: String = ""

    public enum Operation: Int, CustomStringConvertible {
        case CREATE = 0
        case UPDATE = 1
        case DEACTIVATE

        public var description: String {
            let desc: String
            switch self.rawValue {
            case 0:
                desc = "create"
            case 1:
                desc = "update"
            default:
                desc = "deactivate"
            }
            return desc;
        }
    }
    
    private init(_ op: Operation) {
        specification = IDChainRequest.CURRENT_SPECIFICATION
        operation = op
    }
    
    public class func create(_ doc: DIDDocument,
                         _ signKey: DIDURL,
                       _ storepass: String) throws -> IDChainRequest {

        let request = IDChainRequest(Operation.CREATE)
        try request.setPayload(doc)
        try request.seal(signKey, storepass)
        return request
    }
    
    public class func update(_ doc: DIDDocument,
                      previousTxid: String? = nil,
                         _ signKey: DIDURL,
                       _ storepass: String) throws -> IDChainRequest {

        let request = IDChainRequest(Operation.UPDATE)
        request.previousTxid = previousTxid != nil ? previousTxid! : ""
        request.setPayload(doc)
        try request.seal(signKey, storepass)
        return request
    }
    
    public class func deactivate(_ doc: DIDDocument,
                             _ signKey: DIDURL,
                           _ storepass: String) throws -> IDChainRequest {

        let request = IDChainRequest(Operation.DEACTIVATE)
        request.setPayload(doc)
        try request.seal(signKey, storepass)
        return request
    }
    
    public class func deactivate(_ target: DID,
                          _ targetSignKey: DIDURL,
                                    _ doc: DIDDocument,
                                _ signKey: DIDURL,
                              _ storepass: String) throws -> IDChainRequest {

        let request = IDChainRequest(Operation.DEACTIVATE)
        request.setPayload(target)
        try request.seal(targetSignKey, doc, signKey, storepass)
        return request
    }
    
    func setPayload(_ did: DID) {
        self.did = did
        self.doc = nil
        self.payload = did.description
    }
    
    func setPayload(_ doc: DIDDocument) {
        self.did = doc.subject
        self.doc = doc
        
        if operation != Operation.DEACTIVATE {
            let json = doc.description(false)
            let c_input = (json.toUnsafePointerUInt8())!
            payload = json + "\0"
            payload = String(cString: payload.toUnsafePointerUInt8()!)
            let c_payload = UnsafeMutablePointer<Int8>.allocate(capacity: 4096)
            print(payload)
            let re = base64_url_encode(c_payload, c_input, payload.count)
            let jsonStr: String = String(cString: c_payload)
            let endIndex = jsonStr.index(jsonStr.startIndex, offsetBy: re)
            payload = String(jsonStr[jsonStr.startIndex..<endIndex])
        }
        else {
            payload = doc.subject!.description
        }
    }
    
    func setPayload(_ payload: String) {
        if (operation != Operation.DEACTIVATE) {
            let buffer: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: 4096)
            let cp = payload.toUnsafePointerInt8()
            let c = base64_url_decode(buffer, cp)
            var json: String = String(cString: buffer)
            let endIndex = json.index(json.startIndex, offsetBy: c)
            json = String(json[json.startIndex..<endIndex])
            
            doc = try? DIDDocument.fromJson(json)
            did = doc?.subject
        } else {
            did = try? DID(payload)
            doc = nil
        }
        self.payload = payload
    }
    
    func setProof(_ keyType: String, _ signKey: DIDURL, _ signature: String) {
        self.keyType = keyType
        self.signKey = signKey
        self.signature = signature
    }
    
    func seal(_ signKey: DIDURL, _ storepass: String) throws {
        var inputs: [CVarArg] = []
        if specification.count > 0 {
            inputs.append(specification)
            inputs.append(specification.count)
        }
        if operation.description.count > 0 {
            inputs.append(operation.description)
            inputs.append(operation.description.count)
        }
        if previousTxid.count > 0 {
            inputs.append(previousTxid)
            inputs.append(previousTxid.count)
        }
        if payload.count > 0 {
            inputs.append(payload)
            inputs.append(payload.count)
        }
        let count = inputs.count / 2
        self.signature = (try doc?.sign(signKey, storepass, count, inputs))!
        self.signKey = signKey
        self.keyType = DEFAULT_PUBLICKEY_TYPE
    }
    
    func seal(_ targetSignKey: DIDURL, _ doc: DIDDocument, _ signKey: DIDURL, _ storepass: String) throws {
        var inputs: [CVarArg] = []
        let prevtxid = operation == Operation.UPDATE ? previousTxid : ""
        if specification.count > 0 {
            inputs.append(specification)
            inputs.append(specification.count)
        }
        if operation.description.count > 0 {
            inputs.append(operation.description)
            inputs.append(operation.description.count)
        }
        if prevtxid.count > 0 {
            inputs.append(prevtxid)
            inputs.append(prevtxid.count)
        }
        if payload.count > 0 {
            inputs.append(payload)
            inputs.append(payload.count)
        }
        let count = inputs.count / 2
        self.signature = (try doc.sign(signKey, storepass, count, inputs))
        self.signKey = targetSignKey
        self.keyType = DEFAULT_PUBLICKEY_TYPE
    }
    
    public func isValid() throws -> Bool {
        var doc: DIDDocument
        if (operation != Operation.DEACTIVATE) {
            doc = self.doc!
            if (try !doc.isAuthenticationKey(signKey!)){
                return false
            }
        } else {
            doc = try did!.resolve()!
            if (try !doc.isAuthenticationKey(signKey!) && !doc.isAuthorizationKey(signKey!)){
                return false
            }
        }
        var inputs: [CVarArg] = []
        if specification.count > 0 {
            inputs.append(specification)
            inputs.append(specification.count)
        }
        if operation.description.count > 0 {
            inputs.append(operation.description)
            inputs.append(operation.description.count)
        }
        if previousTxid.count > 0 {
            inputs.append(previousTxid)
            inputs.append(previousTxid.count)
        }
        if payload.count > 0 {
            inputs.append(payload)
            inputs.append(payload.count)
        }
        let count = inputs.count / 2
        
        return try doc.verify(signKey!, signature, count, inputs)
    }
    
    public func toJson(_ normalized: Bool) -> String {
        
        var json: OrderedDictionary<String, Any> = OrderedDictionary()
        // header
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        dic[IDChainRequest.SPECIFICATION] = specification
        dic[IDChainRequest.OPERATION] = operation.description
        if (operation == Operation.UPDATE) {
            dic[IDChainRequest.PREVIOUS_TXID] = previousTxid
        }
        json[IDChainRequest.HEADER] = dic

        // playload
        json[IDChainRequest.PAYLOAD] = payload
        
        // signature
        var keyId: String
        dic.removeAll(keepCapacity: 0)
        
        if normalized {
            dic[IDChainRequest.KEY_TYPE] = keyType
            keyId = signKey!.description
        }
        else {
            keyId = "#" + signKey!.fragment
        }
        dic[IDChainRequest.VERIFICATION_METHOD] = keyId
        dic[IDChainRequest.SIGNATURE] = signature
        json[PROOF] = dic
        
        let jsonString: String = JsonHelper.creatJsonString(dic: json)
        return jsonString
    }
    
    public class func fromJson(_ json: OrderedDictionary<String, Any>) throws -> IDChainRequest {
        let header = json[HEADER] as! OrderedDictionary<String, Any>
        let spec: String = try JsonHelper.getString(header, SPECIFICATION, false, nil, SPECIFICATION)
        guard (spec == CURRENT_SPECIFICATION) else {
            throw DIDError.failue("Unknown DID specifiction.")
        }
        var opstr: String = try JsonHelper.getString(header, OPERATION, false, nil, OPERATION)
        opstr = opstr.uppercased()
        var op: Operation = .CREATE
        switch opstr {
        case "CREATE": do {
            op = .CREATE
            }
        case "UPDATE": do {
            op = .UPDATE
            }
        case "DEACTIVATE": do {
            op = .DEACTIVATE
            }
        default: break
            
        }
        let request: IDChainRequest = IDChainRequest(op)
        if (op == Operation.UPDATE) {
            let txid = try JsonHelper.getString(header, PREVIOUS_TXID, false, nil, PREVIOUS_TXID)
            request.previousTxid = txid
        }
        let payload: String = try JsonHelper.getString(json, PAYLOAD, false, nil, PAYLOAD)
        request.setPayload(payload)
        
        let proof = json[PROOF] as! OrderedDictionary<String, Any>
        let keyType = try JsonHelper.getString(proof, KEY_TYPE, true,
                                               DEFAULT_PUBLICKEY_TYPE, KEY_TYPE)
        guard (keyType == DEFAULT_PUBLICKEY_TYPE) else {
            throw DIDError.didResolveError(_desc: "Unknown signature key type.") 
        }
        let signKey = try JsonHelper.getDidUrl(proof, VERIFICATION_METHOD, request.did,
                                               VERIFICATION_METHOD)
        let sig = try JsonHelper.getString(proof, SIGNATURE, false,
                                           nil, SIGNATURE)
        request.setProof(keyType, signKey!, sig)
        
        return request
    }
    
    public class func fromJson(_ json: String) throws -> IDChainRequest {
        let string = JsonHelper.preHandleString(json)
        let dic: OrderedDictionary = JsonHelper.handleString(string) as! OrderedDictionary<String, Any>
        return try fromJson(dic)
    }
}
