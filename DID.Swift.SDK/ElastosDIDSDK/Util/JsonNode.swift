import Foundation

public class JsonNode {
    class func fromText(_ json: String) throws -> JsonNode {
        // TODO:
    }

    public var size: Int {
        // TODO
        return 0
    }

    func getItem(_ keyName: String) -> JsonNode? {
        // TODO
        return nil
    }

    var isTextual: Bool {
        // TODO:
        return false
    }

    func asText() -> String? {
        // TODO:
        return nil
    }

    var isNumber: Bool {
        // TODO:
        return false
    }

    func asInt(_ value: Int) -> Int {
        // TODO:
        return value
    }

    var isArray: Bool {
        // TODO
        return false
    }
}
