import Foundation

class IDTransactionInfo {
    private var _transactionId: String
    private var _timestamp: Date
    private var _request: IDChainRequest
    
    init(_ transactionId: String, _ timestamp: Date, _ request: IDChainRequest) {
        self._transactionId = transactionId;
        self._timestamp = timestamp;
        self._request = request;
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

    class func fromJson(_ node: Dictionary<String, Any>) throws -> IDTransactionInfo {
        guard node.count > 0 else {
            throw DIDError.illegalArgument()
        }

        let error = { (description: String) -> DIDError in
            throw DIDError.didResolveError(description)
        }

        let jsonDict = JsonSerializer(node)
        let transactionId = try jsonDict.getString(Constants.TXID, JsonSerializer.Options<String>()
                                    .withHint("transaction id")
                                    .withError(error as! JsonSerializerErrorGenerator))
        let timestamp = try jsonDict.getDate(Constants.TIMESTAMP, JsonSerializer.Options<Date>()
                                    .withHint("transaction timestamp")
                                    .withError(error as! JsonSerializerErrorGenerator))

        let reqNode = node[Constants.OPERATION] as? Dictionary<String, Any>
        guard let _ = reqNode else {
            throw DIDError.didResolveError("Missing ID operation")
        }

        let request = try IDChainRequest.fromJson(reqNode!)
        return IDTransactionInfo(transactionId!, timestamp, request)
    }

    func toJson(_ generator: JsonGenerator) throws {
        try generator.writeStartObject()
        try generator.writeStringField(Constants.TXID, self.transactionId)
        try generator.writeStringField(Constants.TIMESTAMP, self.timestamp.description)

        // Operation
        try generator.writeFieldName(Constants.OPERATION)
        try self._request.toJson(generator, false)
        try generator.writeEndObject()
    }
}
