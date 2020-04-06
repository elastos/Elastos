import Foundation

public class ResolveResult: DIDHistory{
    var _did: DID!
    var _status: ResolveResultStatus
    var _idtransactionInfos: [IDTransactionInfo] = []

    init(_ did: DID, _ status: Int) {
        self._did = did
        self._status = ResolveResultStatus(rawValue: status)!
    }

    public func getDid() -> DID {
        return _did
    }

    public func getsStatus() -> ResolveResultStatus {
        return _status
    }

    public func getAllTransactions() -> [DIDTransaction] {
        return _idtransactionInfos
    }

    public func getTransactionCount() -> Int {
        return _idtransactionInfos.count
    }

    var did: DID {
        return self._did
    }

    var status: ResolveResultStatus {
        return self._status
    }

    var transactionCount: Int {
        return self._idtransactionInfos.count
    }

    func transactionInfo(_ index: Int) -> IDTransactionInfo? {
        return self._idtransactionInfos[index]
    }

    func appendTransactionInfo( _ info: IDTransactionInfo) {
        self._idtransactionInfos.append(info)
    }

    class func fromJson(_ node: JsonNode) throws -> ResolveResult {
        guard !node.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let error = { (des: String) -> DIDError in
            return DIDError.didResolveError(des)
        }
        let serializer = JsonSerializer(node)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withHint("resolved result did")
                                .withError(error)
        let did = try serializer.getDID(Constants.DID, options)

        options = JsonSerializer.Options()
                                .withRef(-1)
                                .withHint("resolved status")
                                .withError(error)
        let status = try serializer.getInteger(Constants.STATUS, options)

        let result = ResolveResult(did, status)
        if status != ResolveResultStatus.STATUS_NOT_FOUND.rawValue {
            let transactions = node.get(forKey: Constants.TRANSACTION)?.asArray()
            guard transactions?.count ?? 0 > 0 else {
                throw DIDError.didResolveError("invalid resolve result.")
            }
            for transaction in transactions! {
                result.appendTransactionInfo(try IDTransactionInfo.fromJson(transaction))
            }
        }
        return result
    }

    class func fromJson(_ json: Data) throws -> ResolveResult {
        guard !json.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let node: Any
        do {
            node = try JSONSerialization.jsonObject(with: json, options: [])
        } catch {
            throw DIDError.didResolveError("Parse resolve result error")
        }
        return try fromJson(JsonNode(node))
    }

    class func fromJson(_ json: String) throws -> ResolveResult {
        return try fromJson(json.data(using: .utf8)!)
    }

    private func toJson(_ generator: JsonGenerator) {
        generator.writeStartObject()

        generator.writeStringField(Constants.DID, did.toString())
        generator.writeNumberField(Constants.STATUS, status.rawValue)

        if (self._status != .STATUS_NOT_FOUND) {
            generator.writeFieldName(Constants.TRANSACTION)
            generator.writeStartArray()

            for txInfo in _idtransactionInfos {
                txInfo.toJson(generator)
            }
            generator.writeEndArray()
        }

        generator.writeEndObject()
    }

    func toJson() -> String {
        let generator = JsonGenerator()
        toJson(generator)
        return generator.toString()
    }
}

extension ResolveResult: CustomStringConvertible {
    @objc public var description: String {
        return toJson()
    }
}
