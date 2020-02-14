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

        return try IDChainRequest(IDChainRequestOperation.CREATE)
                .setPayload(doc)
                .seal(signKey, storePass)
    }
    
    class func update(_ doc: DIDDocument,
                      _ previousTransactionId: String,
                      _ signKey: DIDURL,
                      _ storePass: String) throws -> IDChainRequest {

        return try IDChainRequest(IDChainRequestOperation.UPDATE)
                .setPreviousTransactionId(previousTransactionId)
                .setPayload(doc)
                .seal(signKey, storePass)
    }
    
    class func deactivate(_ doc: DIDDocument,
                      _ signKey: DIDURL,
                      _ storePass: String) throws -> IDChainRequest {

        return try IDChainRequest(IDChainRequestOperation.DEACTIVATE)
                .setPayload(doc)
                .seal(signKey, storePass)
    }
    
    class func deactivate(_ target: DID,
                      _ targetSignKey: DIDURL,
                      _ doc: DIDDocument,
                      _ signKey: DIDURL,
                      _ storePass: String) throws -> IDChainRequest {

        return try IDChainRequest(IDChainRequestOperation.DEACTIVATE)
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

    class func fromJson(_ node: JsonNode) throws -> IDChainRequest {
        let errorGenerator = { (desc: String) -> DIDError in
            return DIDError.malformedDocument(desc)
        }

        let header = node.getItem(Constants.HEADER)
        guard let _ = header else {
            throw DIDError.didResolveError("Missing header")
        }

        let spec = try JsonHelper.getString(header!, Constants.SPECIFICATION,
                                            false, nil, Constants.SPECIFICATION,
                                            errorGenerator)
        guard spec! == IDChainRequest.CURRENT_SPECIFICATION else {
            throw DIDError.didResolveError("Unknown DID specification")
        }

        let opString = try JsonHelper.getString(header!, Constants.OPERATION, false, nil,
                                            Constants.OPERATION, errorGenerator)
        let operation = IDChainRequestOperation.valueOf(opString!)

        let request = IDChainRequest(operation)
        if operation == .UPDATE {
            let transactionId = try JsonHelper.getString(header!, Constants.PREVIOUS_TXID, false, nil,
                                                Constants.PREVIOUS_TXID, errorGenerator)
            _ = request.setPreviousTransactionId(transactionId!)
        }

        let payload = try JsonHelper.getString(node, Constants.PAYLOAD, false, nil,
                                               Constants.PAYLOAD, errorGenerator)
        _ = try request.setPayload(payload!)

        let proof = node.getItem(Constants.PROOF)
        guard let _ = proof else {
            throw DIDError.didResolveError("Missing proof")
        }

        let keyType = try JsonHelper.getString(proof!, Constants.KEY_TYPE, true,
                                              Constants.DEFAULT_PUBLICKEY_TYPE,
                                              Constants.KEY_TYPE, errorGenerator)
        guard keyType! == Constants.DEFAULT_PUBLICKEY_TYPE else {
            throw DIDError.didResolveError("Unknown signature key type")
        }

        let signKey = try JsonHelper.getDidUrl(proof!, Constants.VERIFICATION_METHOD,
                    request.did, Constants.VERIFICATION_METHOD, errorGenerator)

        let signature = try JsonHelper.getString(proof!, Constants.SIGNATURE, false,
                            nil, Constants.SIGNATURE, errorGenerator)

        _ = request.setProof(keyType!, signKey!, signature!)
        return request
    }

    class func fromJson(_ json: String) throws -> IDChainRequest {
        guard !json.isEmpty else {
            throw DIDError.illegalArgument()
        }

        do {
            let node = try JsonNode.fromText(json)
            return try fromJson(node)
        } catch {
            throw DIDError.didResolveError("Parse resolved result error")
        }
    }
}
