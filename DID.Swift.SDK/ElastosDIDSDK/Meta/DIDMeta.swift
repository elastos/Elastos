import Foundation

class DIDMeta: Metadata {
    private var _deactivated: Bool = false
    private var _updatedDate: Date?
    private var _transactionId: String?
    private var _aliasName: String?

    var aliasName: String {
        return _aliasName ?? ""
    }

    func setAlias(_ alias: String?) {
        self._aliasName = alias
    }

    var transactionId: String? {
        return _transactionId
    }

    func setTransactionId(_ transactionId: String?) {
        self._transactionId = transactionId
    }

    var updatedDate: Date? {
        return self._updatedDate
    }

    func setUpdatedDate(_ updateDate: Date?) {
        self._updatedDate = updatedDate
    }

    var isDeactivated: Bool {
        return _deactivated
    }

    func setDeactivated(_ newValue: Bool) {
        self._deactivated = newValue
    }

    class func fromJson(_ metadata: String) throws -> DIDMeta {
        return try super.fromJson(metadata, DIDMeta.self)
    }

    override func fromNode(_ node: JsonNode) throws {
        var value: String?

        value = node.getValue(Constants.ALIAS)
        if value != nil {
            setAlias(value!)
        }

        value = node.getValue(Constants.DEACTIVATED)
        if value != nil {
            setDeactivated(Bool(value!) ?? true)
        }

        value = node.getValue(Constants.TXID)
        if value != nil {
            setTransactionId(value!)
        }

        value = node.getValue(Constants.TIMESTAMP)
        if value != nil {
            setUpdatedDate(DateFormatter.convertToUTCDateFromString(value!))
        }
    }
    
    override func toNode(_ node: JsonNode) {
        if _aliasName != nil {
            node.setValue(Constants.ALIAS, _aliasName!)
        }

        if _deactivated {
            node.setValue(Constants.DEACTIVATED, _deactivated)
        }

        if _transactionId != nil {
            node.setValue(Constants.TXID, _transactionId!)
        }

        if updatedDate != nil {
            node.setValue(Constants.TIMESTAMP, DateHelper.formateDate(_updatedDate!))
        }
    }

    override func merge(_ other: Metadata) throws {
        guard other is DIDMeta else {
            throw DIDError.illegalArgument("Not DIDMeta object.")
        }

        let meta = other as! DIDMeta
        if let _ = meta._aliasName {
            setAlias(meta.aliasName)
        }
        if let _ = meta.transactionId {
            setTransactionId(meta.transactionId)
        }
        if let _ = meta.updatedDate {
            setUpdatedDate(meta.updatedDate)
        }
        if !self.isDeactivated {
            setDeactivated(meta.isDeactivated)
        }

        try super.merge(other)
    }

    override func isEmpty() -> Bool {
        return (aliasName != ""    || transactionId != nil ||
                updatedDate != nil || isDeactivated) ? false : super.isEmpty()
    }
}
