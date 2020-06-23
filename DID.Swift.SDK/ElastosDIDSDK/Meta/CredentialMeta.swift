import Foundation

public class CredentialMeta: Metadata {
    private let ALIAS = RESERVED_PREFIX + "alias"

    override init(store: DIDStore) {
        super.init(store: store)
    }

    public required init() {
        super.init()
    }

    public var aliasName: String? {
        return self.get(key: ALIAS) as? String
    }

    public func setAlias(_ alias: String?) {
        self.put(key: ALIAS, value: alias as Any)
    }
}
