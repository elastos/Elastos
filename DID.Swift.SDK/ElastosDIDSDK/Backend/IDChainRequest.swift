import Foundation

class IDChainRequest: NSObject {
    static let CURRENT_SPECIFICATION = "elastos/did/1.0"
    
    // header
    private var _specification: String                // must have value
    private var _operation: IDChainRequestOperation   // must have value
    private var _previousTransactionId: String?

    // payload
    private var _did: DID?
    private var _doc: DIDDocument?
    private var _payload: String?
    
    // signature
    private var _keyType: String?
    private var _signKey: DIDURL?
    private var _signature: String?
    
    private init(_ operation: IDChainRequestOperation) {
        self._specification = IDChainRequest.CURRENT_SPECIFICATION
        self._operation = operation
    }
    
    class func create(_ doc: DIDDocument,
                      _ signKey: DIDURL,
                      _ storePass: String) throws -> IDChainRequest {

        return try IDChainRequest(.CREATE)
                .setPayload(doc)
                .seal(signKey, storePass)
    }
    
    class func update(_ doc: DIDDocument,
                      _ previousTransactionId: String,
                      _ signKey: DIDURL,
                      _ storePass: String) throws -> IDChainRequest {

        return try IDChainRequest(.UPDATE)
                .setPreviousTransactionId(previousTransactionId)
                .setPayload(doc)
                .seal(signKey, storePass)
    }
    
    class func deactivate(_ doc: DIDDocument,
                      _ signKey: DIDURL,
                      _ storePass: String) throws -> IDChainRequest {

        return try IDChainRequest(.DEACTIVATE)
                .setPayload(doc)
                .seal(signKey, storePass)
    }
    
    class func deactivate(_ target: DID,
                      _ targetSignKey: DIDURL,
                      _ doc: DIDDocument,
                      _ signKey: DIDURL,
                      _ storePass: String) throws -> IDChainRequest {

        return try IDChainRequest(.DEACTIVATE)
                .setPayload(target)
                .seal(targetSignKey, doc, signKey, storePass)
    }

    var operation: IDChainRequestOperation {
        return self._operation
    }

    var previousTransactionId: String? {
        return self._previousTransactionId
    }

    var payload: String? {
        return self._payload
    }

    var did: DID? {
        return self._did
    }

    var document: DIDDocument? {
        return self._doc
    }

    private func setPreviousTransactionId(_ transactionId: String) -> IDChainRequest {
        self._previousTransactionId = transactionId
        return self
    }
    
    private func setPayload(_ did: DID) -> IDChainRequest {
        self._did = did
        self._doc = nil
        self._payload = did.description

        return self
    }
    
    private func setPayload(_ doc: DIDDocument) throws -> IDChainRequest {
        self._did = doc.subject
        self._doc = doc

        if self._operation != .DEACTIVATE {
            let json = try doc.toJson(false)
            self._payload = json.base64EncodedString // TODO: checkMe.
        } else {
            self._payload = doc.subject.description
        }

        return self
    }
    
    private func setPayload(_ payload: String) throws  -> IDChainRequest {
        do {
            if self._operation != .DEACTIVATE {
                let json = payload.base64DecodedString  // TODO: checkMe

                self._doc = try DIDDocument.fromJson(json!)
                self._did = self._doc!.subject
            } else {
                self._doc = nil
                self._did = try DID(payload)
            }
        } catch {
            throw DIDError.didResolveError("Parse playload error.")
        }

        self._payload = payload
        return self
    }
    
    private func setProof(_ keyType: String,
                          _ signKey: DIDURL,
                          _ signature: String) -> IDChainRequest {

        self._keyType = keyType
        self._signKey = signKey
        self._signature = signature
        return self
    }
    
    private func seal(_ signKey: DIDURL,
                      _ storePass: String) throws -> IDChainRequest {

        let prevTxid = self._operation == .UPDATE ? self._previousTransactionId! : ""
        var inputs: [Data] = []

        inputs.append(self._specification.data(using: .utf8)!)
        inputs.append(self._operation.description.data(using: .utf8)!)
        inputs.append(prevTxid.description.data(using: .utf8)!)
        inputs.append(self._payload!.data(using: .utf8)!)

        self._signature = try self._doc!.signEx(signKey, storePass, inputs)
        self._signKey = signKey
        self._keyType = Constants.DEFAULT_PUBLICKEY_TYPE

        return self
    }
    
    private func seal(_ targetSignKey: DIDURL,
                      _ doc: DIDDocument,
                      _ signKey: DIDURL,
                      _ storePass: String) throws -> IDChainRequest {

        let prevTxid = self._operation == .UPDATE ? self._previousTransactionId! : ""
        var inputs: [Data] = []

        inputs.append(self._specification.data(using: .utf8)!)
        inputs.append(self._operation.description.data(using: .utf8)!)
        inputs.append(self._payload!.data(using: .utf8)!)
        inputs.append(prevTxid.data(using: .utf8)!)

        self._signature = try self._doc!.signEx(signKey, storePass, inputs)
        self._signKey = targetSignKey
        self._keyType = Constants.DEFAULT_PUBLICKEY_TYPE

        return self
    }
    
    private func checkValid() throws -> Bool {
        // internally using builder pattern "create/update/deactivate" to create
        // new IDChainRequest object.
        // Always be sure have "_doc/_signKey/_storePass" in the object.
        var doc: DIDDocument
        if self._operation != .DEACTIVATE {
            doc = self._doc!
            guard doc.containsAuthenticationKey(forId: self._signKey!) else {
                return false
            }
        } else {
            doc = try self._did!.resolve()!
            guard doc.containsAuthenticationKey(forId: self._signKey!) ||
                  doc.containsAuthorizationKey (forId: self._signKey!) else {
                return false
            }
        }

        let prevTxid = self.operation == .UPDATE ? self._previousTransactionId!: ""
        var inputs: [Data] = [];

        inputs.append(self._specification.data(using: .utf8)!)
        inputs.append(self._operation.description.data(using: .utf8)!)
        inputs.append(self._payload!.data(using: .utf8)!)
        inputs.append(prevTxid.data(using: .utf8)!)

        return try doc.verifyEx(self._signKey!, self._signature!, inputs)
    }

    var isValid: Bool {
        do {
            return try self.checkValid()
        } catch {
            return false
        }
    }

    class func fromJson(_ node: Dictionary<String, Any>) throws -> IDChainRequest {
        guard !node.isEmpty else {
            throw DIDError.illegalArgument()
        }
        let header = node[Constants.HEADER] as? Dictionary<String, Any>
        guard let _ = header else {
            throw DIDError.didResolveError("Missing header")
        }

        let headerDict = JsonSerializer(header!)
        let specs = try headerDict.getString(Constants.SPECIFICATION,
                            JsonSerializer.Options<String>()
                                .withHint(Constants.SPECIFICATION))
        guard specs! == IDChainRequest.CURRENT_SPECIFICATION else {
            throw DIDError.didResolveError("Unkown DID specification.")
        }

        let opStr = try headerDict.getString(Constants.OPERATION,
                            JsonSerializer.Options<String>()
                                .withHint(Constants.OPERATION))
        let operation = IDChainRequestOperation.valueOf(opStr!)

        let request = IDChainRequest(IDChainRequestOperation.valueOf(opStr!))
        if operation == .UPDATE {
            let transactionId = try headerDict.getString(Constants.PREVIOUS_TXID,
                            JsonSerializer.Options<String>().withHint(Constants.PREVIOUS_TXID))
            _ = request.setPreviousTransactionId(transactionId!)
        }


        let nodeDict = JsonSerializer(node)
        let payload = try nodeDict.getString(Constants.PAYLOAD, JsonSerializer.Options<String>()
                                .withHint(Constants.PAYLOAD))
        _  = try request.setPayload(payload!)

        let proof = node[Constants.PROOF] as? Dictionary<String, Any>
        let proofDict = JsonSerializer(proof!)
        let keyType = try proofDict.getString(Constants.KEY_TYPE, JsonSerializer.Options<String>()
                                .withOptional()
                                .withDefValue(Constants.DEFAULT_PUBLICKEY_TYPE)
                                .withHint(Constants.KEY_TYPE))
        guard keyType! == Constants.DEFAULT_PUBLICKEY_TYPE else {
            throw DIDError.didResolveError("Unkown signature key type")
        }

        let signKey = try proofDict.getDIDURL(Constants.VERIFICATION_METHOD,
                            JsonSerializer.Options<DIDURL>()
                                .withHint(Constants.VERIFICATION_METHOD))
        let signature = try proofDict.getString(Constants.SIGNATURE,
                            JsonSerializer.Options<String>()
                                .withHint(Constants.SIGNATURE))
        _ = request.setProof(keyType!, signKey!, signature!)
        return request
    }

    class func fromJson(_ json: Data) throws -> IDChainRequest {
        guard !json.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let node: Dictionary<String, Any>
        do {
            node = try JSONSerialization.jsonObject(with: json, options: []) as! Dictionary<String, Any>
        } catch {
            throw DIDError.didResolveError("Parse resolve result error")
        }
        return try fromJson(node)
    }

    class func fromJson(_ json: String) throws -> IDChainRequest {
        return try fromJson(json.data(using: .utf8)!)
    }

    func toJson(_ generator: JsonGenerator, _ normalized: Bool) throws {
        try generator.writeStartObject()

        // header
        try generator.writeFieldName(Constants.HEADER)

        try generator.writeStartObject()
        try generator.writeStringField(Constants.SPECIFICATION, self._specification)
        try generator.writeStringField(Constants.OPERATION, self.operation.toString())
        if self._operation == .UPDATE {
            try generator.writeFieldName(Constants.PREVIOUS_TXID)
            try generator.writeString(self.previousTransactionId!)
        }
        try generator.writeEndObject() // end of header.

        // payload
        try generator.writeStringField(Constants.PAYLOAD, self.payload!)

        // signature
        try generator.writeFieldName(Constants.PROOF)
        try generator.writeStartObject()

        var keyId: String
        if normalized {
            try generator.writeStringField(Constants.KEY_TYPE, self._keyType!)
            keyId = self._signKey!.description
        } else {
            keyId = "#" + self._signKey!.fragment!
        }
        try generator.writeStringField(Constants.VERIFICATION_METHOD, keyId)
        try generator.writeStringField(Constants.SIGNATURE, self._signature!)

        try generator.writeEndObject()  // end of signature.

        try generator.writeEndObject()
    }

    func toJson(_ normalized: Bool) -> String {
        // TODO
        return "TODO"
    }
}
