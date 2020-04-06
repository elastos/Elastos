import Foundation

class IDTransactionInfo: DIDTransaction {
    private var _transactionId: String
    private var _timestamp: Date
    private var _request: IDChainRequest
    
    init(_ transactionId: String, _ timestamp: Date, _ request: IDChainRequest) {
        self._transactionId = transactionId;
        self._timestamp = timestamp;
        self._request = request;
    }

    public func getDid() -> DID {
        return request.did!
    }

    public func getTransactionId() -> String {
        return self.transactionId
    }

    public func getTimestamp() -> Date {
        return self.timestamp
    }

    public func getOperation() -> IDChainRequestOperation {
        return request.operation
    }

    public func getDocument() -> DIDDocument {
        return request.document!
    }

    var transactionId: String {
        return self._transactionId
    }

    var timestamp: Date {
        return self._timestamp
    }

    var did: DID? {
        return self._request.did
    }
    
    var operation: IDChainRequestOperation {
        return self._request.operation
    }

    var payload: String {
        return self._request.toJson(false)
    }

    var request: IDChainRequest {
        return self._request
    }

    class func fromJson(_ node: JsonNode) throws -> IDTransactionInfo {
        let error = { (des: String) -> DIDError in
            return DIDError.didResolveError(des)
        }

        let serializer = JsonSerializer(node)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withHint("transaction id")
                                .withError(error)
        let transactionId = try serializer.getString(Constants.TXID, options)

        options = JsonSerializer.Options()
                                .withHint("transaction timestamp")
                                .withError(error)
        let timestamp = try serializer.getDate(Constants.TIMESTAMP, options)

        let subNode = node.get(forKey: Constants.OPERATION)
        guard let _ = subNode else {
            throw DIDError.didResolveError("missing ID operation")
        }

        let request = try IDChainRequest.fromJson(subNode!)
        return IDTransactionInfo(transactionId, timestamp, request)
    }

    func toJson(_ generator: JsonGenerator) {
        generator.writeStartObject()
        generator.writeStringField(Constants.TXID, self.transactionId)
        generator.writeStringField(Constants.TIMESTAMP, self.timestamp.description)

        generator.writeFieldName(Constants.OPERATION)
        self._request.toJson(generator, false)
        generator.writeEndObject()
    }
}
