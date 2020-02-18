import Foundation

class JsonNode {
    private var node: Dictionary<String, Any>

    init() {
        self.node = Dictionary<String, Any>()
    }

    init(_ node: Dictionary<String, Any>) {
        self.node = node
    }

    var isEmpty: Bool {
        return node.count > 0
    }

    var count: Int {
        return node.count
    }

    func toString() -> String {
        // TODO:
        return "TODO"
    }

    func deepCopy() -> JsonNode? {
        // TODO:
        return nil
    }

    func getDict() -> Dictionary<String, Any> {
        return self.node
    }

    func getValue(_ key: String) -> String? {
        return node[key] as? String
    }

    func getNode(_ key: String) -> JsonNode? {
        return node[key] as? JsonNode
    }

    func getNodeArray(_ key: String) -> [JsonNode]? {
        return node[key] as? [JsonNode]
    }

    func setValue(_ key: String, _ value: String) {
        node[key] = value
    }

    func setValue(_ key: String, _ value: Bool) {
        node[key] = value
    }
}
