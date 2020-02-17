import Foundation

typealias JsonSerializerErrorGenerator = (String) -> DIDError

class JsonSerializer {
    private let data: Dictionary<String, Any>

    init(_ data: Dictionary<String, Any>) {
        self.data = data
    }

    func getString(_ keyName: String, _ options: Options<String>) throws -> String? {
        let value = self.data[keyName] as? String
        guard let _ = value else {
            if options.optional {
                return options.defValue
            }
            throw options.error("Missing \(options.hint)")
        }
        guard !value!.isEmpty else {
            throw options.error("Invalid \(options.hint)")
        }
        return value
    }

    func getInteger(_ keyName: String, _ options: Options<Int>) throws -> Int {
        let value = self.data[keyName] as? Int
        guard let _ = value else {
            if options.optional {
                return options.defValue!
            }
            throw DIDError.malformedDocument("Missing \(options.hint)")
        }
        return value!
    }

    func getDID(_ keyName: String, _ options: Options<DID>) throws -> DID? {
        let value = self.data[keyName] as? String
        guard let _ = value else {
            if options.optional {
                return options.defValue
            }
            throw options.error("Missing \(options.hint)")
        }
        guard !value!.isEmpty else {
            throw options.error("Invalid \(options.hint)")
        }

        let did: DID
        do {
            did = try DID(value!)
        } catch {
            throw options.error("Invalid \(options.hint)")
        }
        return did
    }

    func getDIDURL(_ keyName: String, _ options: Options<DIDURL>) throws -> DIDURL? {
        // TODO;
        return nil
    }

    func getDate(_ keyName: String, _ options: Options<Date>) throws -> Date {
        // TODO:
        return Date()
    }

    class Options<T> {
        var optional: Bool
        var defValue: T?
        var hint: String
        var error: JsonSerializerErrorGenerator

        init() {
            self.optional = false
            self.hint = ""
        }

        func withOptional() -> Options {
            self.optional = true
            return self
        }

        func withOptional(_ optional: Bool) -> Options {
            return optional ? withOptional() : self
        }

        func withDefValue(_ defValue: T?) -> Options {
            self.defValue = defValue
            return self
        }

        func withHint(_ hint: String) -> Options {
            self.hint = hint
            return self
        }

        func withError(_ error: @escaping JsonSerializerErrorGenerator) -> Options {
            self.error = error
            return self
        }
    }
}

/*
class JsonHelper {
    static func getDidUrl(_ node: JsonNode,
                          _ name: String,
                          _ optional: Bool,
                          _ ref: DID?,
                          _ hint: String,
                          _ errorGenerator: DIDErrorGenerator) throws -> DIDURL? {

        let item = node.getItem(name)
        guard let _ = item else {
            guard !optional else {
                return nil
            }
            throw errorGenerator("Missing \(hint).")
        }

        guard item!.isTextual else {
            throw errorGenerator("Invalid \(hint) value")
        }

        let value = item!.asText()
        guard value?.isEmpty ?? true else {
            throw errorGenerator("Invalid \(hint) value")
        }

        let id: DIDURL
        do {
            if ref != nil && value!.hasPrefix("#") {
                id = try DIDURL(ref!, "TODO")   // TODO:
            } else  {
                id = try DIDURL(value!)
            }
        } catch {
            throw errorGenerator("Invalid \(hint):\(value!)")
        }

        return id
    }

    static func getDidUrl(_ node: JsonNode,
                          _ name: String,
                          _ ref: DID?,
                          _ hint: String,
                          _ errorGenerator: DIDErrorGenerator) throws -> DIDURL? {
        return try getDidUrl(node, name, false, ref, hint, errorGenerator)
    }

    static func getDidUrl(_ node: JsonNode,
                          _ ref: DID?,
                          _ hint: String,
                          _ errorGenerator: DIDErrorGenerator) throws -> DIDURL? {
        // TODO
        return nil
    }
    
    static func getDate(_ node: JsonNode,
                          _ name: String,
                          _ optional: Bool,
                          _ ref: Date?,
                          _ hint: String,
                          _ errorGenerator: DIDErrorGenerator) throws -> Date? {
        // TODO
        return nil
    }
    
    static func fromDate(_ date: Date) -> String? {
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd'T'HH:mm:ss'Z'"
        formatter.timeZone = TimeZone(identifier: "UTC")
        return formatter.string(from: date)
    }

    static func parseDate(_ dateStr: String) -> Date? {
        // TODO
        return nil
    }
}
*/
