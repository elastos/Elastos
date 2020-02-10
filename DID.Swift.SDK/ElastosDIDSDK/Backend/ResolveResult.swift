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

    class func fromJson(_ result: JsonNode) throws -> ResolveResult {
        let errorGenerator = { (desc: String) -> DIDError in
            return DIDError.malformedDocument(desc)
        }

        guard result.size > 0 else {
            throw DIDError.illegalArgument()
        }

        let did = try JsonHelper.getDid(result, Constants.DID, false, nil, "Resolved result DID",
                                        errorGenerator)
        let status = try JsonHelper.getInteger(result, Constants.STATUS, false, -1, "Resolved status",
                                        errorGenerator)

        let resolveResult = ResolveResult(did!, status)
        if status != ResolveResultStatus.STATUS_NOT_FOUND.rawValue {
            let transactions = result.getItem(Constants.TRANSACTION)
            guard transactions?.isArray ?? false else {
                throw DIDError.didResolveError("Invalid resolve result.")
            }
            guard transactions?.size ?? 0 > 0 else {
                throw DIDError.didResolveError("Invalid resolve result")
            }
            // TODO:
        }

        return resolveResult
    }

    class func fromJson(_ json: String) throws -> ResolveResult {
        guard !json.isEmpty else {
            throw DIDError.illegalArgument()
        }

        do {
            let node: JsonNode = try JsonNode.fromText(json)
            return try fromJson(node)
        } catch {
            throw DIDError.didResolveError("Parse resolve result error.")
        }
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
