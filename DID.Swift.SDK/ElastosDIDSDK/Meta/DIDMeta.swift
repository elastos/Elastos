import Foundation

class DIDMeta: Metadata {
    private var _deactivated: Bool = false
    private var _updatedDate: Date?
    private var _transactionId: String?
    private var _signature: String?
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

    func setTransactionId(_ newValue: String?) {
        self._transactionId = newValue
    }

    var signature: String? {
        return self._signature
    }

    func setSignature(_ newValue: String) {
        self._signature = newValue
    }

    var updatedDate: Date? {
        return self._updatedDate
    }

    func setUpdatedDate(_ newValue: Date?) {
        self._updatedDate = newValue
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

        value = node.get(forKey: Constants.ALIAS)?.asString()
        if value != nil {
            setAlias(value!)
        }

        value = node.get(forKey: Constants.DEACTIVATED)?.asString()
        if value != nil {
            setDeactivated(Bool(value!) ?? true)
        }

        value = node.get(forKey: Constants.TXID)?.asString()
        if value != nil {
            setTransactionId(value!)
        }

        value = node.get(forKey: Constants.SIGNATURE)?.asString()
        if value != nil {
            setSignature(value!)
        }

        value = node.get(forKey: Constants.TIMESTAMP)?.asString()
        if value != nil {
            setUpdatedDate(DateFormatter.convertToUTCDateFromString(value!))
        }
    }
    
    override func toNode(_ node: JsonNode) {
        if _aliasName != nil {
            node.put(forKey: Constants.ALIAS, value: _aliasName!)
        }

        if _deactivated {
            node.put(forKey: Constants.DEACTIVATED, value: _deactivated)
        }

        if _transactionId != nil {
            node.put(forKey: Constants.TXID, value: _transactionId!)
        }

        if _signature != nil {
            node.put(forKey: Constants.SIGNATURE, value: _signature!)
        }

        if updatedDate != nil {
            node.put(forKey: Constants.TIMESTAMP, value: DateHelper.formateDate(_updatedDate!))
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
        if !self.isDeactivated {
            setDeactivated(meta.isDeactivated)
        }
        if let _ = meta.transactionId {
            setTransactionId(meta.transactionId)
        }
        if let _ = meta.signature {
            setSignature(meta.signature!)
        }
        if let _ = meta.updatedDate {
            setUpdatedDate(meta.updatedDate)
        }

        try super.merge(other)
    }

    override func isEmpty() -> Bool {
        return (aliasName != ""    || transactionId != nil ||
                signature != nil   || updatedDate != nil   || isDeactivated)
            ? false : super.isEmpty()
    }
}
