

import Foundation

public struct OrderedDictionary<KeyType: Hashable, ValueType> {
    private var _dictionary: Dictionary<KeyType, ValueType>
    private var _keys: Array<KeyType>
    
    public init() {
        _dictionary = [:]
        _keys = []
    }
    
    public init(minimumCapacity: Int) {
        _dictionary = Dictionary<KeyType, ValueType>(minimumCapacity: minimumCapacity)
        _keys = Array<KeyType>()
    }
    
    public init(_ dictionary: Dictionary<KeyType, ValueType>) {
        _dictionary = dictionary
        _keys = dictionary.keys.map { $0 }
    }
    
    public subscript(key: KeyType) -> ValueType? {
        get {
            _dictionary[key]
        }
        set {
            if newValue == nil {
                _ = self.removeValueForKey(key: key)
            } else {
                _ = self.updateValue(value: newValue!, forKey: key)
            }
        }
    }
    
    public mutating func updateValue(value: ValueType, forKey key: KeyType) -> ValueType? {
        let oldValue = _dictionary.updateValue(value, forKey: key)
        if oldValue == nil {
            _keys.append(key)
        }
        return oldValue
    }
    
    public mutating func removeValueForKey(key: KeyType) -> Bool {
        _keys = _keys.filter {
            $0 != key
        }
        return (_dictionary.removeValue(forKey: key) != nil)
    }
    
    public mutating func removeAll(keepCapacity: Int) {
        _keys = []
        _dictionary = Dictionary<KeyType, ValueType>(minimumCapacity: keepCapacity)
    }
    
    public var count: Int {
        get {
            _dictionary.count
        }
    }
    
    // keys isn't lazy evaluated because it's just an array anyway
    public var keys: [KeyType] {
        get {
            _keys
        }
    }
    
    public var values: Array<ValueType> {
        get {
            _keys.map { _dictionary[$0]! }
        }
    }
    
    public static func ==<Key: Equatable, Value: Equatable>(lhs: OrderedDictionary<Key, Value>, rhs: OrderedDictionary<Key, Value>) -> Bool {
        lhs._keys == rhs._keys && lhs._dictionary == rhs._dictionary
    }
    
    public static func !=<Key: Equatable, Value: Equatable>(lhs: OrderedDictionary<Key, Value>, rhs: OrderedDictionary<Key, Value>) -> Bool {
        lhs._keys != rhs._keys || lhs._dictionary != rhs._dictionary
    }
    
    public static func creatJsonString(dic: OrderedDictionary<String, Any>) -> String {
        
        var namedPaird = [String]()
        dic.forEach { (key, value) in
            if value is OrderedDictionary<String, Any> {
                namedPaird.append("\"\(key)\":\(self.creatJsonString(dic: value as! OrderedDictionary<String, Any>))")
            }else if value is [Any] {
                let v = value as! [Any]
                var subName = [String]()
                v.forEach { ve in
                    if ve is String {
                        subName.append("\"\(ve)\"")
                    } else {
                        subName.append("\(self.creatJsonString(dic: ve as! OrderedDictionary<String, Any>))")
                    }
                }
                let st = subName.joined(separator: ",")
                namedPaird.append("\"\(key)\":[\(st)]")
            }else{
                namedPaird.append("\"\(key)\":\"\(value)\"")
            }
        }
        let returnString = namedPaird.joined(separator:",")
        return "{\(returnString)}"
    }
    
//    MARK: json
    public static func isArrayJsonString(_ jsonString: String) -> Bool {
        return jsonString.first == "[" && jsonString.last == "]"
    }
    
    public static func isDictionaryJsonString(_ jsonString: String) -> Bool {
        return jsonString.first == "{" && jsonString.last == "}"
    }
    
    public static func checkAndRemoveFirstAndLastDoubleQuotes(_ string: String) -> String {
        
        var startIndex: String.Index
        var endIndex: String.Index
        if string.first == "\"" {
            startIndex = string.index(string.startIndex, offsetBy: 1)
        } else {
            startIndex = string.startIndex
        }
        if string.last == "\"" {
            endIndex = string.index(string.endIndex, offsetBy: -2)
        } else {
            endIndex = string.index(string.endIndex, offsetBy: -1)
        }
        
        return String(string[startIndex...endIndex])
    }
    
    public static func checkAndRemoveFirstAndLastBrackets(_ string: String) -> String {
        if ((string.first == "{" && string.last == "}")
            || (string.first == "[" && string.last == "]"))
            && string.count > 2 {
            
            let startIndex = string.index(string.startIndex, offsetBy: 1)
            let endIndex = string.index(string.endIndex, offsetBy: -2)
            return String(string[startIndex...endIndex])
        }
        return string
    }
    
    public static func getKeyAndValueFromString(_ string: String) -> (String, String) {
        
        let colonStringIndex = string.index(string.firstIndex(of: ":")!, offsetBy:0)
        let keyStartIndex = string.index(string.startIndex, offsetBy:0)
        let keyEndIndex = string.index(colonStringIndex, offsetBy:-1)
        let valueStartIndex = string.index(colonStringIndex, offsetBy:1)
        let valueEndIndex = string.index(string.endIndex, offsetBy:-1)
        
        let key: String = checkAndRemoveFirstAndLastDoubleQuotes(String(string[keyStartIndex...keyEndIndex]))
        let value: String = checkAndRemoveFirstAndLastDoubleQuotes(String(string[valueStartIndex...valueEndIndex]))
        
        return (key, value)
    }
    
    public static func handleString(_ jsonString: String) -> Any? {
        
        if isDictionaryJsonString(jsonString) {
            
            var orderDictionary: OrderedDictionary<String, Any>
            var keys: Array = [String]()
            var tempStr: String?
            var level: Int = 0
            
            let content = checkAndRemoveFirstAndLastBrackets(jsonString)
            
            orderDictionary = OrderedDictionary<String, Any>()
            
            for (index, char) in content.enumerated() {
                
                if char == "{" || char == "["{
                    level = level + 1
                }
                if char == "}" || char == "]"{
                    level = level - 1
                }
                
                if (level == 0 && char == ",") || (level == 0 && index == String(content).count - 1) {
                    
                    if (level == 0 && index == String(content).count - 1) {
                        tempStr = (tempStr ?? "") + String(char)
                    }
                    
                    if tempStr?.count ?? 0 >= 0 {
                        keys.append(String(tempStr!))
                        tempStr = nil
                    }
                } else {
                    tempStr = (tempStr ?? "") + String(char)
                }
            }
            
            for content: String in keys {
                
                let keyAndValue: (String, String) = getKeyAndValueFromString(content)
                let key: String = keyAndValue.0
                let value: String = keyAndValue.1
                
                
                if isDictionaryJsonString(value) {
                    orderDictionary[key] = self.handleString(String(value))
                } else if isArrayJsonString(value) {
                    orderDictionary[key] = self.handleString(String(value))
                } else {
                    orderDictionary[key] = value
                }
            }
            
            return orderDictionary;
        }
        
        if isArrayJsonString(jsonString) {
            
            var resultArray: Array = Array<Any>()
            var tempStr: String?
            var level: Int = 0
            
            let content = checkAndRemoveFirstAndLastBrackets(jsonString)
            
            for (index, char) in content.enumerated() {
                
                if char == "{" || char == "["{
                    level = level + 1
                }
                if char == "}" || char == "]"{
                    level = level - 1
                }
                
                if (level == 0 && char == ",") || (level == 0 && index == String(content).count - 1) {
                    if (level == 0 && index == String(content).count - 1) {
                        tempStr = (tempStr ?? "") + String(char)
                    }
                    if isDictionaryJsonString(tempStr ?? "") {
                        resultArray.append(self.handleString(tempStr!) as Any)
                    } else if isArrayJsonString(tempStr ?? "") {
                        resultArray.append(self.handleString(tempStr!) as Any)
                    } else {
                        resultArray.append(checkAndRemoveFirstAndLastDoubleQuotes(String(tempStr ?? "")))
                    }
                    tempStr = nil
                } else {
                    tempStr = (tempStr ?? "") + String(char)
                }
                
            }
            return resultArray
        }
        return nil
    }
}

extension OrderedDictionary: Sequence {
    
    public func makeIterator() -> OrderedDictionaryIterator<KeyType, ValueType> {
        OrderedDictionaryIterator<KeyType, ValueType>(sequence: _dictionary, keys: _keys, current: 0)
    }
}

public struct OrderedDictionaryIterator<KeyType: Hashable, ValueType>: IteratorProtocol {
    let sequence: Dictionary<KeyType, ValueType>
    let keys: Array<KeyType>
    var current = 0
    
    mutating public func next() -> (KeyType, ValueType)? {
        defer { current += 1 }
        guard sequence.count > current else {
            return nil
        }
        
        let key = keys[current]
        guard let value = sequence[key] else {
            return nil
        }
        return (key, value)
    }
    
}

