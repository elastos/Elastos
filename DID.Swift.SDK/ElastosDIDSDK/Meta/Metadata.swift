import Foundation

private func getFullName(_ name: String) -> String {
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
        let dict = node.asDictionary()
        guard let _ = dict else {
            return meta
        }

        for (key, value) in dict! {
            meta.setExtra(key, value.asString() ?? "")
        }
        return meta
    }

    class func fromJson<T: Metadata>(_ metadata: Data, _ type: T.Type) throws -> T {
        let data: Any
        do {
            data = try JSONSerialization.jsonObject(with: metadata, options: [])
        } catch {
            throw DIDError.malformedMeta("Parse metadata error.")
        }

        return try fromJson(JsonNode(data), type)
    }

    class func fromJson<T: Metadata>(_ metadata: String, _ type: T.Type) throws -> T {
        return try fromJson(metadata.data(using: .utf8)!, type)
    }

    func toJson() -> String {
        if #available(iOS 11.0, *) {
            let data = try! JSONSerialization.data(withJSONObject: _extra, options: [.sortedKeys])
            return String(data: data, encoding: .utf8)!
        } else {
            // Fallback on earlier versions
            let keys = _extra.keys
            let sortedKeys = keys.sorted()
            var result: [String] = []
            for key in sortedKeys {
                let temp: String = "\"\(key)\":\"\(_extra[key] ?? "")\""
                result.append(temp)
            }
            let jsonString = result.joined(separator: ",")
            return "{\(jsonString)}"

        }
    }

    func merge(_ meta: Metadata) throws {
        meta._extra.forEach{ (key, value) in
            _extra[key] = value
        }
    }

    func isEmpty() -> Bool {
        return _extra.isEmpty
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
