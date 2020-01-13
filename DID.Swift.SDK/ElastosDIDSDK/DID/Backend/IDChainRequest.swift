import Foundation

public class IDChainRequest: NSObject {
    
    private static let CURRENT_SPECIFICATION: String = "elastos/did/1.0"
    
    private static let HEADER: String = "header"
    private static let SPECIFICATION: String = "specification"
    private static let OPERATION: String = "operation"
    private static let PREVIOUS_TXID: String  = "previousTxid"
    private static let PAYLOAD: String = "payload"
    private static let KEY_TYPE: String = TYPE
    private static let KEY_ID: String = VERIFICATION_METHOD
    
    public enum Operation: Int {
        case CREATE = 0
        case UPDATE = 1
        case DEACTIVATE
        
        public func toString() -> String {
            if self.rawValue == 0 {
                return "create"
            } else if self.rawValue == 1{
                return "update"
            } else {
                return "deactivate"
            }
        }
    }
    
    // header
    public var specification: String = ""
    public var operation: Operation!
    public var previousTxid: String = ""

    // payload
    public var did: DID?
    public var doc: DIDDocument?
    public var payload: String = ""
    
    // signature
    public var keyType: String = ""
    public var signKey: DIDURL?
    public var signature: String = ""
    
    public init(_ op: Operation) throws {
        specification = IDChainRequest.CURRENT_SPECIFICATION
        operation = op
    }
    
    public class func create(_ doc: DIDDocument, _ signKey: DIDURL, _ storepass: String) throws -> IDChainRequest {
        let request: IDChainRequest = try IDChainRequest(Operation.CREATE)
        try request.setPayload(doc)
        try request.seal(signKey, storepass)
        
        return request
    }
    
    public class func update(_ doc: DIDDocument, previousTxid: String? = nil, _ signKey: DIDURL, _ storepass: String) throws -> IDChainRequest {
        let request: IDChainRequest = try IDChainRequest(Operation.UPDATE)
        request.previousTxid = previousTxid != nil ? previousTxid! : ""
        try request.setPayload(doc)
        try request.seal(signKey, storepass)
        
        return request
    }
    
    public class func deactivate(_ doc: DIDDocument, _ signKey: DIDURL, _ storepass: String) throws -> IDChainRequest {
        let request: IDChainRequest = try IDChainRequest(Operation.DEACTIVATE)
        try request.setPayload(doc)
        try request.seal(signKey, storepass)
        
        return request
    }
    
    public class func deactivate(_ target: DID, _ targetSignKey: DIDURL, _ doc: DIDDocument, _ signKey: DIDURL, _ storepass: String) throws -> IDChainRequest {
        let request = try IDChainRequest(Operation.DEACTIVATE)
        try request.setPayload(target)
        try request.seal(targetSignKey, doc, signKey, storepass)

        return request
    }
    
    func setPayload(_ did: DID) throws {
        self.did = did
        self.doc = nil
        self.payload = did.description
    }
    
    func setPayload(_ doc: DIDDocument) throws {
        self.did = doc.subject
        self.doc = doc
        
        if operation != Operation.DEACTIVATE {
            let json = try doc.description(false)
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
    
    func setPayload(_ payload: String) throws {
        if (operation != Operation.DEACTIVATE) {
            let buffer: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: 4096)
            let cp = payload.toUnsafePointerInt8()
            let c = base64_url_decode(buffer, cp)
            var json: String = String(cString: buffer)
            let endIndex = json.index(json.startIndex, offsetBy: c)
            json = String(json[json.startIndex..<endIndex])
            
            doc = try DIDDocument.fromJson(json)
            did = doc?.subject
        } else {
            did = try DID(payload)
            doc = nil
        }
        self.payload = payload
    }
    
    func setProof(_ keyType: String, _ signKey: DIDURL, _ signature: String) throws {
        self.keyType = keyType
        self.signKey = signKey
        self.signature = signature
    }
    
    func seal(_ signKey: DIDURL, _ storepass: String) throws {
        var inputs: [CVarArg] = [ ]
        if specification.count > 0 {
            inputs.append(specification)
            inputs.append(specification.count)
        }
        if operation.toString().count > 0 {
            inputs.append(operation.toString())
            inputs.append(operation.toString().count)
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
        if operation.toString().count > 0 {
            inputs.append(operation.toString())
            inputs.append(operation.toString().count)
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
        if operation.toString().count > 0 {
            inputs.append(operation.toString())
            inputs.append(operation.toString().count)
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
        dic[IDChainRequest.OPERATION] = operation.toString()
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
        dic[IDChainRequest.KEY_ID] = keyId
        dic[SIGNATURE] = signature
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
        let request: IDChainRequest = try IDChainRequest(op)
        if (op == Operation.UPDATE) {
            let txid = try JsonHelper.getString(header, PREVIOUS_TXID, false, nil, PREVIOUS_TXID)
            request.previousTxid = txid
        }
        let payload: String = try JsonHelper.getString(json, PAYLOAD, false, nil, PAYLOAD)
        try request.setPayload(payload)
        
        let proof = json[PROOF] as! OrderedDictionary<String, Any>
        let keyType = try JsonHelper.getString(proof, KEY_TYPE, true,
                                               DEFAULT_PUBLICKEY_TYPE, KEY_TYPE)
        guard (keyType == DEFAULT_PUBLICKEY_TYPE) else {
            throw DIDResolveError.failue("Unknown signature key type.")
        }
        let signKey = try JsonHelper.getDidUrl(proof, KEY_ID, request.did,
                                               KEY_ID)
        let sig = try JsonHelper.getString(proof, SIGNATURE, false,
                                           nil, SIGNATURE)
        try request.setProof(keyType, signKey!, sig)
        
        return request
    }
    
    class public func fromJson(_ json: String) throws -> IDChainRequest {
        let string = JsonHelper.preHandleString(json)
        let dic: OrderedDictionary = JsonHelper.handleString(string) as! OrderedDictionary<String, Any>
        return try fromJson(dic)
    }
}
