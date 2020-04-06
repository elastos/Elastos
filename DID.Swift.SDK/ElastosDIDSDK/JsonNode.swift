import Foundation

public enum JsonNodeType: Int {
    case ARRAY
    case BOOLEAN
    case NIL
    case NUMBER
    case DICTIONARY
    case STRING
}
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
        var resultString: String

        if node is [JsonNode] {
            let temp = node as? [JsonNode]
            var result: Array<String> = []
            for subNode in temp! {
                result.append(subNode.toString())
            }
            let data = try! JSONSerialization.data(withJSONObject: result, options: [])
            resultString = String(data: data, encoding: String.Encoding.utf8)!
            resultString = resultString.replacingOccurrences(of: "\\\"", with: "\"")
            return resultString;

        } else if node is [String: JsonNode] {
            let temp = node as? [String: JsonNode]
            var result: [String: String] = [: ]
            for (key, value) in temp! {
                result[key] = value.toString()
            }
            let data = try! JSONSerialization.data(withJSONObject: result, options: [])
            resultString = String(data: data, encoding: String.Encoding.utf8)!
            resultString = resultString.replacingOccurrences(of: "\\\"", with: "\"")
            return resultString;

        } else {
            resultString = "\(node)"
        }
        resultString = resultString.replacingOccurrences(of: "\\\"", with: "\"")
        return resultString;
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
        self.node = temp as Any
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
        self.node = temp as Any
    }

    func put(forKey key: String, value: Bool) {
        // TODO:
    }

    func remove(_ key: String) {
        guard self.node is [String: Any] else {
            return
        }
        var dictionary = self.node as! [String: Any]
        dictionary.removeValue(forKey: key)
        self.node = dictionary
    }

    public func asString() -> String? {
        return self.node as? String
    }

    public func asInteger() -> Int? {
        return self.node as? Int
    }

    public func asNumber() -> Any? {
        return self.node
    }

    public func asBool() -> Bool? {
        return self.node as? Bool
    }

    public func asArray() -> [JsonNode]? {
        return self.node as? Array
    }

    public func asDictionary() -> [String: JsonNode]? {
        return self.node as? [String: JsonNode]
    }

    public func getNodeType() -> JsonNodeType {

        if self.node is Array<Any> {
            return JsonNodeType.ARRAY
        } else if self.node is [String: Any] {
            return JsonNodeType.DICTIONARY
        } else if self.node is String {
            return JsonNodeType.STRING
        }  else if self.node is Bool {
            return JsonNodeType.BOOLEAN
        } else if self.node is Int {
            return JsonNodeType.NUMBER
        } else if self.node is Float {
            return JsonNodeType.NUMBER
        }else {
            return JsonNodeType.NIL
        }
    }
}
