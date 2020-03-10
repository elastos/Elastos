import Foundation

public class JsonNode {
    private var node: Any

    init() {
        self.node = [String: Any]()
    }

    init(_ node: Any) {
        if node is [Any] {
            var temp = node as? [Any]
            if temp == nil {
                temp = []
            }
            var result: Array<JsonNode> = []
            for subNode in temp! {
                result.append(JsonNode(subNode))
            }
            self.node = result
        } else if node is [String: Any] {
            var temp = node as? [String: Any]
            if temp == nil {
                temp = [: ]
            }
            var result: [String: JsonNode] = [: ]
            for (key, value) in temp! {
                result[key] = JsonNode(value)
            }
            self.node = result
        } else {
            self.node = node
        }
    }

    init(_ node: [String: Any]) {
        var result: [String: JsonNode] = [: ]
        for (key, value) in node {
            result[key] = JsonNode(value)
        }
        self.node = result
    }

    var isEmpty: Bool {
        // TODO:
        return false
    }

    var count: Int {
        if self.node is [String: Any] {
            var temp = self.node as? [String: Any]
            if temp == nil {
                temp = [: ]
            }
            return temp!.count
        } else if self.node is [Any] {
            var temp = self.node as? [Any]
            if  temp == nil {
                temp = []
            }
            return temp!.count
        }
        return 0
    }

    public func toString() -> String {
        // TODO:
        return "TODO"
    }

    func deepCopy() -> JsonNode? {
        
        if self.node is [JsonNode] {
            var temp = node as? [JsonNode]
            if temp == nil {
                // TODO: check error
                temp = []
            }
            var resultArray: Array<JsonNode> = []
            for subNode in temp! {
                resultArray.append(subNode.deepCopy()!)
            }
            let result = JsonNode()
            result.node = resultArray
            return result
        } else if node is [String: JsonNode] {
            var temp = node as? [String: JsonNode]
            if temp == nil {
                temp = [: ]
            }
            var resultDictionary: [String: JsonNode] = [: ]
            for (key, value) in temp! {
                resultDictionary[key] = value.deepCopy()
            }
            let result = JsonNode()
            result.node = resultDictionary
            return result
        } else {
            return JsonNode(self.node)
        }
    }

    func get(forKey key: String) -> JsonNode? {
        guard self.node is [String: JsonNode] else {
            return nil
        }
        var temp = self.node as? [String: JsonNode]
        if temp == nil {
            temp = [: ]
        }
        return temp![key]
    }
    
    func put(forKey key: String, value: [Any]) {

        guard self.node is [String: JsonNode] else {
            return
        }
        
        let node: JsonNode = JsonNode(value)
        var temp = self.node as? [String: JsonNode]
        if temp == nil {
            temp = [: ]
        }
        temp![key] = node
        self.node = temp!
    }
    
    func put(forKey key: String, value: [String: Any]) {

        guard self.node is [String: JsonNode] else {
            return
        }
        
        let node: JsonNode = JsonNode(value)
        var temp = self.node as? [String: JsonNode]
        if temp == nil {
            temp = [: ]
        }
        temp![key] = node
    }
    
    func put(forKey key: String, value: String) {
        
        guard self.node is [String: JsonNode] else {
            return
        }
        
        let node: JsonNode = JsonNode(value)
        var temp = self.node as? [String: JsonNode]
        if temp == nil {
            temp = [: ]
        }
        temp![key] = node
    }

    func put(forKey key: String, value: Bool) {
        // TODO:
    }

    public func asString() -> String? {
        return self.node as? String
    }

    public func asInteger() -> Int? {
        return self.node as? Int
    }

    public func asArray() -> [JsonNode]? {
        return self.node as? Array
    }

    public func asDictionary() -> [String: JsonNode]? {
        return self.node as? [String: JsonNode]
    }
}
