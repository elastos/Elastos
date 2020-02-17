import Foundation

class Metadata {
    private var _store: DIDStore?
    private var _extra: Dictionary<String, String>?

    var store: DIDStore? {
        return self._store
    }

    var hasAttachedStore: Bool {
        return store != nil
    }

    func setStore(_ store: DIDStore) {
        self._store = store
    }

    private func fullName(_ name: String) -> String {
        return Constants.EXTRA_PREFIX + name
    }

    func setExtra(_ name: String, _ value: String?) {
        if  self._extra == nil {
            self._extra = Dictionary<String, String>()
        }
        self._extra![fullName(name)] = value
    }

    func getExtra(_ name: String) -> String? {
        return self._extra?[fullName(name)] ?? nil
    }
    
    class func fromNode(_ node: Dictionary<String, Any>) throws {} // abstract method
    func toNode(_ node: Dictionary<String, Any>) {}                // abstract method

    class func fromJson<T: Metadata>(_ node: Dictionary<String, Any>, _ type: T.Type) throws -> T {
        // TODO
    }

    class func fromJson<T: Metadata>(_ metadata: String, _ type: T.Type) throws -> T {
        // TODO
    }

    func toJson() -> String {
        // TODO
        return "TODO"
    }

    func merge(_ meta: Metadata) throws {
        meta._extra?.forEach{ (key, value) in
            self._extra?[key] = value
        }
    }

    func isEmpty() -> Bool {
        return self._extra?.isEmpty ?? false
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
