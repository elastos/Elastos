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
                      _ storePassword: String) throws -> IDChainRequest {

        return try IDChainRequest(.CREATE)
                .setPayload(doc)
                .sealed(signKey, storePassword)
    }

    class func update(_ doc: DIDDocument,
                      _ previousTransactionId: String,
                      _ signKey: DIDURL,
                      _ storePassword: String) throws -> IDChainRequest {

        return try IDChainRequest(.UPDATE)
                .setPreviousTransactionId(previousTransactionId)
                .setPayload(doc)
                .sealed(signKey, storePassword)
    }

    class func deactivate(_ doc: DIDDocument,
                      _ signKey: DIDURL,
                      _ storePassword: String) throws -> IDChainRequest {

        return try IDChainRequest(.DEACTIVATE)
                .setPayload(doc)
                .sealed(signKey, storePassword)
    }

    class func deactivate(_ target: DID,
                      _ targetSignKey: DIDURL,
                      _ doc: DIDDocument,
                      _ signKey: DIDURL,
                      _ storePassword: String) throws -> IDChainRequest {

        return try IDChainRequest(.DEACTIVATE)
                .setPayload(target)
                .sealed(targetSignKey, doc, signKey, storePassword)
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
            let json = doc.toString()
            let capacity = json.count * 3

            let cInput = json.toUnsafePointerUInt8()
            let cPayload = UnsafeMutablePointer<Int8>.allocate(capacity: capacity)
            let re = base64_url_encode(cPayload, cInput, json.count)
            cPayload[re] = 0
            self._payload = String(cString: cPayload)

//            memset(cPayload, 0, capacity)
//
//            var ref = json
//            let ssz = ref.withUTF8 { ptr in
//                return base64_url_encode(cPayload, ptr.baseAddress!, json.count)
//            }
//
//            guard ssz > 0 else {
//                throw DIDError.unknownFailure("base58 encoding error")
//            }
        } else {
            self._payload = doc.subject.toString()
        }

        return self
    }

    private func setPayload(_ payload: String) throws  -> IDChainRequest {
        do {
            if self._operation != .DEACTIVATE {
//                let buffer = UnsafeMutablePointer<Int8>.allocate(capacity: capacity)
//                memset(buffer, 0, capacity)
//
//                var ref = payload
//                let ssz = ref.withUTF8 { (ptr) in
//                    return base64_url_encode(buffer, ptr.baseAddress!, payload.count)
//                }
//
//                guard ssz > 0 else {
//                    throw DIDError.unknownFailure("base58 encoding error")
//                }
//
//                var json = String(cString: buffer)
//                let endIndex = json.index(json.startIndex, offsetBy: ssz)
//                json = String(json[json.startIndex..<endIndex])

                let capacity = payload.count * 3
                let buffer: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: capacity)
                let cp = payload.toUnsafePointerInt8()
                let c = base64_url_decode(buffer, cp)
                buffer[c] = 0
                let json: String = String(cString: buffer)
                self._doc = try DIDDocument.convertToDIDDocument(fromJson: json)
                self._did = _doc!.subject
            } else {
                self._doc = nil
                self._did = try DID(payload)
            }
        } catch {
            throw DIDError.didtransactionError("Parse playload error.")
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

    private func sealed(_ signKey: DIDURL,
                        _ storePassword: String) throws -> IDChainRequest {

        let prevTxid = _operation == .UPDATE ? self._previousTransactionId! : ""
        var inputs: [Data] = []

        if let data = _specification.data(using: .utf8)  {
            inputs.append(data)
        }
        if let data = _operation.description.data(using: .utf8)  {
            inputs.append(data)
        }
        if let data = prevTxid.description.data(using: .utf8)  {
            inputs.append(data)
        }
        if let data = _payload!.data(using: .utf8)  {
            inputs.append(data)
        }

        self._signature = try _doc!.sign(signKey, storePassword, inputs)
        self._signKey = signKey
        self._keyType = Constants.DEFAULT_PUBLICKEY_TYPE

        return self
    }

    private func sealed(_ targetSignKey: DIDURL,
                        _ doc: DIDDocument,
                        _ signKey: DIDURL,
                        _ storePassword: String) throws -> IDChainRequest {

        let prevTxid = operation == .UPDATE ? self._previousTransactionId! : ""
        var inputs: [Data] = []
        if let data = _specification.data(using: .utf8)  {
            inputs.append(data)
        }
        if let data = _operation.description.data(using: .utf8)  {
            inputs.append(data)
        }
        if let data = prevTxid.description.data(using: .utf8)  {
            inputs.append(data)
        }
        if let data = _payload!.data(using: .utf8)  {
            inputs.append(data)
        }

        self._signature = try doc.sign(signKey, storePassword, inputs)
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
            guard doc.containsAuthenticationKey(forId: _signKey!) else {
                return false
            }
        } else {
            do {
                doc = try self._did!.resolve()
            } catch {
                return false
            }
            guard doc.containsAuthenticationKey(forId: _signKey!) ||
                  doc.containsAuthorizationKey (forId: _signKey!) else {
                return false
            }
        }

        let prevTxid = operation == .UPDATE ? self._previousTransactionId!: ""
        var inputs: [Data] = []
        if let data = _specification.data(using: .utf8)  {
            inputs.append(data)
        }
        if let data = _operation.description.data(using: .utf8)  {
            inputs.append(data)
        }
        if let data = prevTxid.description.data(using: .utf8)  {
            inputs.append(data)
        }
        if let data = _payload!.data(using: .utf8)  {
            inputs.append(data)
        }

        return try doc.verify(_signKey!, _signature!, inputs)
    }

    var isValid: Bool {
        do {
            return try self.checkValid()
        } catch {
            return false
        }
    }

    class func fromJson(_ node: JsonNode) throws -> IDChainRequest {
        let error = { (des: String) -> DIDError in
            return DIDError.didResolveError(des)
        }

        var subNode = node.get(forKey: Constants.HEADER)
        guard let _ = subNode else {
            throw DIDError.didResolveError("missing header")
        }

        var serializer = JsonSerializer(subNode!)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withHint(Constants.SPECIFICATION)
                                .withError(error)
        let specs = try serializer.getString(Constants.SPECIFICATION, options)
        guard specs == IDChainRequest.CURRENT_SPECIFICATION else {
            throw DIDError.didResolveError("unkown did specification.")
        }

        options = JsonSerializer.Options()
                                .withHint(Constants.OPERATION)
                                .withError(error)
        let opstr = try serializer.getString(Constants.OPERATION, options)
        let operation = IDChainRequestOperation.valueOf(opstr)

        let request = IDChainRequest(operation)
        if operation == .UPDATE {
            options = JsonSerializer.Options()
                                .withHint(Constants.PREVIOUS_TXID)
                                .withError(error)
            let transactionId = try serializer.getString(Constants.PREVIOUS_TXID, options)
            _ = request.setPreviousTransactionId(transactionId)
        }

        serializer = JsonSerializer(node)
        options = JsonSerializer.Options()
                                .withHint(Constants.PAYLOAD)
                                .withError(error)
        let payload = try serializer.getString(Constants.PAYLOAD, options)
        _  = try request.setPayload(payload)

        subNode = node.get(forKey: Constants.PROOF)
        guard let _ = subNode else {
            throw DIDError.didResolveError("missing proof.")
        }

        serializer = JsonSerializer(subNode!)
        options = JsonSerializer.Options()
                                .withOptional()
                                .withRef(Constants.DEFAULT_PUBLICKEY_TYPE)
                                .withHint(Constants.KEY_TYPE)
                                .withError(error)
        let keyType = try serializer.getString(Constants.KEY_TYPE, options)
        guard keyType == Constants.DEFAULT_PUBLICKEY_TYPE else {
            throw DIDError.didResolveError("unkown signature key type")
        }

        options = JsonSerializer.Options()
                                .withRef(request.did)
                                .withHint(Constants.VERIFICATION_METHOD)
                                .withError(error)
        let signKey = try serializer.getDIDURL(Constants.VERIFICATION_METHOD, options)

        options = JsonSerializer.Options()
                                .withHint(Constants.SIGNATURE)
                                .withError(error)
        let signature = try serializer.getString(Constants.SIGNATURE, options)

        _ = request.setProof(keyType, signKey!, signature)
        return request
    }

    class func fromJson(_ json: Data) throws -> IDChainRequest {
        guard !json.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let data: Any
        do {
            data = try JSONSerialization.jsonObject(with: json, options: [])
        } catch {
            throw DIDError.didResolveError("Parse resolve result error")
        }
        return try fromJson(JsonNode(data))
    }

    class func fromJson(_ json: String) throws -> IDChainRequest {
        return try fromJson(json.data(using: .utf8)!)
    }

    func toJson(_ generator: JsonGenerator, _ normalized: Bool) {
        generator.writeStartObject()
        generator.writeFieldName(Constants.HEADER)

        generator.writeStartObject()
        generator.writeStringField(Constants.SPECIFICATION, self._specification)
        generator.writeStringField(Constants.OPERATION, self.operation.toString())
        if self._operation == .UPDATE {
            generator.writeFieldName(Constants.PREVIOUS_TXID)
            generator.writeString(self.previousTransactionId!)
        }
        generator.writeEndObject() // end of header.

        // payload
        generator.writeStringField(Constants.PAYLOAD, self.payload!)

        // signature
        generator.writeFieldName(Constants.PROOF)
        generator.writeStartObject()

        var keyId: String
        if normalized {
            generator.writeStringField(Constants.KEY_TYPE, self._keyType!)
            keyId = self._signKey!.description
        } else {
            keyId = "#" + self._signKey!.fragment!
        }
        generator.writeStringField(Constants.VERIFICATION_METHOD, keyId)
        generator.writeStringField(Constants.SIGNATURE, self._signature!)

        generator.writeEndObject()  // end of signature.
        generator.writeEndObject()
    }

    func toJson(_ normalized: Bool) -> String {
        let generator = JsonGenerator()
        toJson(generator, normalized)
        return generator.toString()
    }
}
