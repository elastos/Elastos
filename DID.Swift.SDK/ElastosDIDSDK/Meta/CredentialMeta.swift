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

    override class func fromNode(_ node: Dictionary<String, Any>) throws {
        // TODO:
    }

    override func toNode(_ node: Dictionary<String, Any>) {
        // TODO:
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
        if self.aliasName != "" {
            return false
        }

        return super.isEmpty()
    }
}
