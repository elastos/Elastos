import Foundation

typealias DIDErrorGenerator = (String) -> DIDError

class JsonHelper {

    static func getString(_ node: JsonNode,
                          _ name: String,
                          _ optional: Bool,
                          _ ref: String?,
                          _ hint: String,
                          _ errorGenerator: DIDErrorGenerator) throws -> String? {

        let item = node.getItem(name)
        guard let _ = item else {
            guard !optional else {
                return ref
            }
            throw errorGenerator("Missing \(hint).")
        }
        guard item!.isTextual else {
            throw errorGenerator("Invalid \(hint) value")
        }

        let value = item!.asText()
        guard !(value?.isEmpty ?? true) else {
            throw errorGenerator("Invalid \(hint) value")
        }

        return value
    }

    static func getInteger(_ node: JsonNode,
                          _ name: String,
                          _ optional: Bool,
                          _ ref: Int,
                          _ hint: String,
                          _ errorGenerator: DIDErrorGenerator) throws -> Int {
        let item = node.getItem(name)
        guard let _ = item else {
            guard !optional else {
                return ref
            }
            throw errorGenerator("Missing \(hint).")
        }

        guard item!.isNumber else {
            throw errorGenerator("Invalid \(hint) value")
        }

        return item!.asInt(ref)
    }

    static func getDid(_ node: JsonNode,
                          _ name: String,
                          _ optional: Bool,
                          _ ref: DID?,
                          _ hint: String,
                          _ errorGenerator: DIDErrorGenerator) throws -> DID? {

        let item = node.getItem(name)
        guard let _ = item else {
            guard !optional else {
                return ref
            }
            throw errorGenerator("Missing \(hint).")
        }

        guard item!.isTextual else {
            throw errorGenerator("Invalid \(hint) value.")
        }
        let value = item!.asText()
        guard !(value?.isEmpty ?? true) else {
            throw errorGenerator("Invalid \(hint) value.")
        }

        let did: DID
        do {
            did = try DID(value!)
        } catch {
            throw errorGenerator("Invalid \(hint): \(value!)")
        }

        return did
    }

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

    static func toJson(_ generator: JsonGenerator, _ node: JsonNode) throws {
        try toJson(generator, node, false)
    }

    static func toJson(_ generator: JsonGenerator, _ node: JsonNode, _ objectContext: Bool) throws {
        // TODO:
    }

    static func fromDate(_ date: Date) -> String? {
        // TODO
        return nil
    }

    static func parseDate(_ dateStr: String) -> Date? {
        // TODO
        return nil
    }
}
