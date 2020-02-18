import Foundation

typealias JsonSerializerErrorGenerator = (String) -> DIDError

let malformedDocumentError = { (description) -> DIDError in
    return DIDError.malformedDocument(description)
}

class JsonSerializer {
    private var node: JsonNode

    init(_ data: JsonNode) {
        self.node = data
    }

    func getString(_ keyName: String, _ options: Options) throws -> String {
        let value = node.getValue(keyName)
        guard let _ = value else {
            if options.optional {
                return options.refValue! as! String
            }
            throw options.error("Missing \(options.hint)")
        }

        guard !value!.isEmpty else {
            throw options.error("Missing \(options.hint)")
        }

        return value!
    }

    func getInteger(_ keyName: String, _ options: Options) throws -> Int {
        let value = node.getValue(keyName)
        guard let _ = value else {
            if options.optional {
                return options.refValue as! Int
            }
            throw options.error("Missing \(options.hint)")
        }

        return Int(value!) ?? 0
    }

    func getDID(_ keyName: String, _ options: Options) throws -> DID {
        let value = node.getValue(keyName)
        guard let _ = value else {
            if options.optional {
                return options.refValue as! DID
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

    func getDIDURL(_ keyName: String, _ options: Options) throws -> DIDURL? {
        let value = node.getValue(keyName)
        guard let _ = value else {
            if options.optional {
                return nil
            }
            throw options.error("Missing \(options.hint)")
        }
        guard !value!.isEmpty else {
            throw options.error("Invalid \(options.hint)")
        }

        let id: DIDURL
        do {
            let ref: DID? = options.refValue as? DID
            if ref != nil && value!.hasPrefix("#") {
                id = try DIDURL(ref!, "TODO") // TODO:
            } else {
                id = try DIDURL(value!)
            }
        } catch {
            throw options.error("Invalid \(options.hint)")
        }
        return id
    }

    func getDate(_ keyName: String, _ options: Options) throws -> Date {
        let value = node.getValue(keyName)
        guard let _ = value else {
            if options.optional {
                return options.refValue! as! Date
            }
            throw options.error("Missing \(options.hint)")
        }
        guard !value!.isEmpty else {
            throw options.error("Invalid \(options.hint)")
        }

        let date = DateFormatter.convertToUTCDateFromString(value!)
        guard let _ = date else {
            throw options.error("Invalid \(options.hint)")
        }

        return date!
    }

    class Options {
        var optional: Bool
        var refValue: Any?
        var hint: String
        var error: JsonSerializerErrorGenerator

        init() {
            self.optional = false
            self.hint = ""
            self.error = malformedDocumentError
        }

        func withOptional() -> Options {
            self.optional = true
            return self
        }

        func withOptional(_ optional: Bool) -> Options {
            return optional ? withOptional() : self
        }

        func withRef(_ ref: Any?) -> Options {
            self.refValue = ref
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
