import Foundation

func getFullName(_ name: String) -> String {
    return Constants.EXTRA_PREFIX + name
}

class Metadata {
    private var _store: DIDStore?
    private var _extra = Dictionary<String, String>()

    required init() {}

    var store: DIDStore? {
        return self._store
    }

    var attachedStore: Bool {
        return self._store != nil
    }

    func setStore(_ store: DIDStore) {
        self._store = store
    }

    func setExtra(_ name: String, _ value: String?) {
        self._extra[getFullName(name)] = value
    }

    func getExtra(_ name: String) -> String? {
        return self._extra[getFullName(name)]
    }
    
    func fromNode(_ node: JsonNode) throws {}
    func toNode(_ node: JsonNode) {}

    class func fromJson<T: Metadata>(_ node: JsonNode, _ type: T.Type) throws -> T {
        let meta = T.init()
        guard !node.isEmpty else {
            return meta
        }

        try meta.fromNode(node)
        for (key, value) in (node.getDict() as! Dictionary<String, String>) {
            meta.setExtra(key, value)
        }
        return meta
    }

    class func fromJson<T: Metadata>(_ metadata: Data, _ type: T.Type) throws -> T {
        let node: Dictionary<String, Any>?

        do {
            node = try JSONSerialization.jsonObject(with: metadata, options: []) as? Dictionary<String, Any>
        } catch {
            throw DIDError.malformedMeta("Parse metadata error.")
        }

        guard let _ = node else {
            throw DIDError.malformedMeta("Parse metadata error.")
        }
        return try fromJson(JsonNode(node!), type)
    }

    class func fromJson<T: Metadata>(_ metadata: String, _ type: T.Type) throws -> T {
        return try fromJson(metadata.data(using: .utf8)!, type)
    }

    func toJson() -> String {
        // TODO
        return "TODO"
    }

    func merge(_ meta: Metadata) throws {
        meta._extra.forEach{ (key, value) in
            self._extra[key] = value
        }
    }

    func isEmpty() -> Bool {
        return self._extra.isEmpty
    }
}

extension Metadata: CustomStringConvertible {
    func toString() -> String {
        return toJson()
    }

    var description: String {
        return toString()
    }
}
