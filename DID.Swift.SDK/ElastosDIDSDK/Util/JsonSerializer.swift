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
        let child = node.get(forKey: keyName)

        guard let _ = child else {
            if options.optional {
                return options.refValue! as! String
            } else {
                throw options.error("missing \(options.hint)")
            }
        }

        let value = child!.asString()
        guard !(value?.isEmpty ?? false) else {
            throw options.error("invalid \(options.hint)")
        }
        return value!
    }

    func getInteger(_ keyName: String, _ options: Options) throws -> Int {
        let child = node.get(forKey: keyName)

        guard let _ = child else {
            if options.optional {
                return options.refValue as! Int
            } else {
                throw options.error("invalid \(options.hint)")
            }
        }

        let value = child!.asInteger()
        guard let _ = value else {
            throw options.error("invalid \(options.hint)")
        }
        return value!
    }

    func getDID(_ keyName: String, _ options: Options) throws -> DID {
        let child = node.get(forKey: keyName)

        guard let _ = child else {
            if options.optional {
                if options.refValue == nil {
                    throw options.error("missing \(options.hint)")
                }
                return options.refValue as! DID
            } else {
                throw options.error("missing \(options.hint)")
            }
        }

        let value = child!.asString()
        guard !(value?.isEmpty ?? false) else {
            throw options.error("invalid \(options.hint)")
        }

        let did: DID
        do {
            did = try DID(value!)
        } catch {
            throw options.error("invalid \(options.hint)")
        }
        return did
    }

    func getDIDURL(_ keyName: String, _ options: Options) throws -> DIDURL? {
        let child = node.get(forKey: keyName)

        guard let _ = child else {
            if options.optional {
                return nil
            } else {
                throw options.error("missing \(options.hint)")
            }
        }

        let value = child!.asString()
        guard !(value?.isEmpty ?? false) else {
            throw options.error("invalid \(options.hint)")
        }

        let id: DIDURL
        do {
            let ref: DID? = options.refValue as? DID
            if ref != nil && value!.hasPrefix("#") {
                let fragment = String(value!.suffix(value!.count - 1))
                id = try DIDURL(ref!, fragment)
            } else {
                id = try DIDURL(value!)
            }
        } catch {
            throw options.error("invalid \(options.hint)")
        }
        return id
    }

    func getDIDURL(_ options: Options) throws -> DIDURL? {
        let value = node.asString()

        guard let _ = value else {
            if options.optional {
                return nil
            } else {
                throw options.error("missing \(options.hint)")
            }
        }

        guard !(value!.isEmpty) else {
            throw options.error("invalid \(options.hint)")
        }

        let id: DIDURL
        do {
            let ref: DID? = options.refValue as? DID
            if ref != nil && value!.hasPrefix("#") {
                let fragment = String(value!.suffix(value!.count - 1))
                id = try DIDURL(ref!, fragment)
            } else {
                id = try DIDURL(value!)
            }
        } catch {
            throw options.error("invalid \(options.hint)")
        }
        return id
    }

    func getDate(_ keyName: String, _ options: Options) throws -> Date {
        let child = node.get(forKey: keyName)

        guard let _ = child else {
            if options.optional {
                return options.refValue! as! Date
            } else {
                throw options.error("missing \(options.hint)")
            }
        }

        let value = child!.asString()
        guard !(value?.isEmpty ?? false) else {
            throw options.error("invalid \(options.hint)")
        }

        let date = DateFormatter.convertToUTCDateFromString(value!)
        guard let _ = date else {
            throw options.error("invalid \(options.hint)")
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
