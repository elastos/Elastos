import Foundation

public class Metadata: NSObject {
    static let RESERVED_PREFIX = "DX-"
    static let LAST_MODIFIED = RESERVED_PREFIX + "lastModified"
    private var _store: DIDStore?
    private var _extra = Dictionary<String, Any>()

    public required override init() {}

    init(store: DIDStore) {
        super.init()
        setStore(store)
    }

    public var store: DIDStore? {
        return self._store
    }

    public var attachedStore: Bool {
        return self._store != nil
    }

    public func setStore(_ store: DIDStore) {
        self._store = store
    }

    public func setLastModified(_ timestamp: Date) {
        put(key: Metadata.LAST_MODIFIED, value: DateHelper.getTimeStampForString(timestamp))
    }

    public func getLastModified() -> Date? {
        let date = get(key: Metadata.LAST_MODIFIED) as? String
        let time = DateHelper.getDateFromTimeStampWithString(date)
        return time
    }

    public func clearLastModified() {
        _extra.removeValue(forKey: Metadata.LAST_MODIFIED)
    }

    public func setExtra(_ key: String, _ value: String?) {
        self._extra[key] = value
    }

    public func getExtra(_ key: String) -> String? {
        return self._extra[key] as? String
    }

    public func merge(_ meta: Metadata) throws {
        meta._extra.forEach{ (key, value) in
            if _extra.keys.contains(key) {
                if _extra[key] == nil {
                    _extra.removeValue(forKey: key)
                }
            }
            else {
                if case Optional<Any>.none = value {
                    return
                }
                _extra[key] = value
            }
        }
    }

    public func load(reader: FileHandle) throws {
        let data = reader.readDataToEndOfFile()
        defer {
            reader.closeFile()
        }

        let json = try JSONSerialization.jsonObject(with: data, options: .mutableContainers)
        let dic = json as! Dictionary<String, Any>
        let node = JsonNode(dic)

        try load(node)
    }

    public func load(_ node: JsonNode) throws {
        let dic = node.asDictionary()
        try dic?.forEach{ (key, value) in
            switch value.getNodeType() {

            case .BOOLEAN:
                _extra[key] = value.asBool()
                break;
            case .NIL:
                break
            case .NUMBER:
                _extra[key] = value.asNumber()
                break;
            case .STRING:
                _extra[key] = value.asString()
                break;
            default:
                throw DIDError.malformedMeta("Unsupported field: \(key)")
            }
        }
    }

    public func put(key: String, value: Any) {
        _extra[key] = value
    }

    public func get(key: String) -> Any {
        return _extra[key] as Any
    }

    private func save(_ generator: JsonGenerator) throws {
        generator.writeStartObject()

        try _extra.forEach { (key, value) in
            if case Optional<Any>.none = value {
                // Continue
            } else if value is Int {
                generator.writeNumberField(key, value as! Int)
            } else if value is Bool {
                generator.writeBoolField(key, value as! Bool)
            } else if value is String {
                generator.writeStringField(key, value as! String)
            } else if value is Date {
                generator.writeStringField(key, DateHelper.formateDate(value as! Date))
            } else {
                throw DIDError.malformedMeta("Can not serialize attribute: \(key)")
            }
        }
        generator.writeEndObject()
    }

    public func save(path: FileHandle) throws {
        defer {
            path.closeFile()
        }

        let generator = JsonGenerator()
        try save(generator)
        path.write(generator.toString().data(using: .utf8)!)
    }

    public func toJson() throws -> String {
        let generator = JsonGenerator()
        try save(generator)

        return generator.toString()
    }
    
    public func isEmpty() -> Bool {
        return _extra.isEmpty
    }
}

extension Metadata {
    public func toString() throws -> String {
        return try toJson()
    }
}
