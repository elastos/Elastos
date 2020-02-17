import Foundation

class ResolveResult {
    private var _did: DID
    private var _status: ResolveResultStatus
    private var _idtransactionInfos: Array<IDTransactionInfo>?
    
    init(_ did: DID, _ status: Int) {
        self._did = did
        self._status = ResolveResultStatus(rawValue: status)!
    }

    var did: DID {
        return self._did
    }

    var status: ResolveResultStatus {
        return self._status
    }

    var transactionCount: Int {
        return self._idtransactionInfos?.count ?? 0
    }

    func transactionInfo(_ index: Int) -> IDTransactionInfo? {
        return self._idtransactionInfos?[index]
    }

    func appendTransactionInfo( _ info: IDTransactionInfo) { // TODO: should be synchronized ?
        if  self._idtransactionInfos == nil {
            self._idtransactionInfos = Array<IDTransactionInfo>()
        }
        self._idtransactionInfos!.append(info)
    }

    class func fromJson(_ node: Dictionary<String, Any>) throws -> ResolveResult {
        guard node.count > 0 else {
            throw DIDError.illegalArgument()
        }
        let error = { (description: String) -> DIDError in
            return DIDError.didResolveError(description)
        }
        let jsonDict = JsonSerializer(node)
        let did = try jsonDict.getDID(Constants.DID, JsonSerializer.Options<DID>()
                            .withHint("resolved result did")
                            .withError(error))
        let status = try jsonDict.getInteger(Constants.STATUS, JsonSerializer.Options<Int>()
                            .withDefValue(-1)
                            .withHint("resolved status")
                            .withError(error))

        let result = ResolveResult(did!, status)
        if status != ResolveResultStatus.STATUS_NOT_FOUND.rawValue {
            let transactions = node[Constants.TRANSACTION] as? [Dictionary<String, Any>]
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

        let node: Dictionary<String, Any>
        do {
            node = try JSONSerialization.jsonObject(with: json, options: []) as! Dictionary<String, Any>
        } catch {
            throw DIDError.didResolveError("Parse resolve result error")
        }
        return try fromJson(node)
    }

    class func fromJson(_ json: String) throws -> ResolveResult {
        return try fromJson(json.data(using: .utf8)!)
    }

    private func toJson(_ generator: JsonGenerator) throws {
        try generator.writeStartObject()

        try generator.writeStringField(Constants.DID,    self.did.toString())
        try generator.writeNumberField(Constants.STATUS, self.status.rawValue)

        if (self._status != .STATUS_NOT_FOUND) {
            try generator.writeFieldName(Constants.TRANSACTION)
            try generator.writeStartArray()

            for txInfo in self._idtransactionInfos! {
                try txInfo.toJson(generator)
            }
            try generator.writeEndArray()
        }

        try generator.writeEndObject()
    }

    func toJson() throws -> String {
        // TODO
        return "TODO"
    }
}

extension ResolveResult: CustomStringConvertible {
    @objc public var description: String {
        return (try? toJson()) ?? ""
    }
}
