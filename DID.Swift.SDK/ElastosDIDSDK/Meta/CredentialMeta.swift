import Foundation

class CredentialMeta: Metadata {
    private var _aliasName: String?

    var aliasName: String {
        return self._aliasName ?? ""
    }

    func setAlias(_ alias: String?) {
        self._aliasName = alias
    }

    class func fromJson(_ metadata: String) throws -> CredentialMeta {
        return try super.fromJson(metadata, CredentialMeta.self)
    }

    override func fromNode(_ node: JsonNode) throws {
        let value = node.get(forKey: Constants.ALIAS)?.asString()
        if  value != nil {
            setAlias(value!)
        }
    }

    override func toNode(_ node: JsonNode) {
        if _aliasName != nil {
            node.put(forKey: Constants.ALIAS, value: _aliasName!)
        }
    }

    override func merge(_ other: Metadata) throws {
        guard other is CredentialMeta else {
            throw DIDError.illegalArgument("Not CredentailMeta object")
        }

        let meta = other as! CredentialMeta
        if let _ = meta._aliasName {
            setAlias(meta.aliasName)
        }

        try super.merge(other)
    }

    override func isEmpty() -> Bool {
        if !aliasName.isEmpty {
            return false
        }
        return super.isEmpty()
    }
}
