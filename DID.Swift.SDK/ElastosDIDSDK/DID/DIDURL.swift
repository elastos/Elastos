import Foundation

public class DIDURL: NSObject {
    
    public var did: DID!
    public var parameters: OrderedDictionary<String, String>?
    public var path: String?
    public var query: OrderedDictionary<String, String>?
    public var fragment: String!
    private var listener: DURLListener?
    
    public init(_ id: DID, _ fragment: String) throws {
        self.did = id
        self.fragment = fragment
    }
    
    public init(_ url: String) throws {
        super.init()
        self.listener = DURLListener(self)
        try ParserHelper.parase(url, false, self.listener!)
        
    }
    
    public func mapToString(_ dictionary: OrderedDictionary<String, String>, sep: String) -> String {
        
        var initBoolLean: Bool = true;
        var result: String = ""
        
        for (key, value) in dictionary {
            if initBoolLean {
                initBoolLean = false
            } else {
                result = result + sep
            }
            
            result = result + key
            if !value.isEmpty {
                result = result + "=" + value
            }
        }
        
        return result
    }
    
    public func getParameters() -> String? {
        guard parameters != nil else {
            return nil
        }
        return mapToString(self.parameters!, sep: ";")
    }
    
    public func getParameter(_ name: String) -> String? {
        return parameters![name]!
    }
    
    public func hasParameter(_ name: String) -> Bool {
        guard parameters != nil else {
            return false
        }
        return parameters!.keys.contains(name)
    }
    
    public func addParameter(_ key: String?, _ value: String?) {
        return parameters![key!] = value ?? ""
    }
    
    public func addQueryParameter(_ key: String?, _ value: String?) {
        return query![key!] = value ?? ""
    }
    
    public func getQuery() -> String {
        guard query != nil else {
            return ""
        }
        return mapToString(self.query!, sep: "&")
    }
    
    public func getQueryParameter(_ name: String) -> String? {
        guard query != nil else {
            return query![name]!
        }
        return nil
    }
    
    public func hasQueryParameter(_ name: String) -> Bool {
        guard query != nil else {
            return query!.keys.contains(name)
        }
        return false
    }
    
    public func toExternalForm() -> String {
        let testDID: String = self.did.description
        var params: String = ""
        if self.parameters != nil {
           params = mapToString(self.parameters!, sep: ";")
        }
        let path = self.path ?? ""
        var query: String = ""
        if self.query != nil {
            query = mapToString(self.query!, sep: "&")
        }
        let fragment = self.fragment ?? ""
        return testDID + ";" + params + path + "?" + query + "#" + fragment
    }
    
    public func toString() -> String {
        return toExternalForm()
    }
    
    public override func isEqual(_ object: Any?) -> Bool {
        
        
        if object is DIDURL {
            let url = object as! DIDURL
            let urlExternalForm = url.toExternalForm()
            let selfExternalForm = toExternalForm()
            return urlExternalForm.isEqual(selfExternalForm)
        }
        
        if object is String {
            let url = object as! String
            let selfExternalForm = toExternalForm()
            return url.isEqual(selfExternalForm)
        }
        
        return super.isEqual(object);
    }
    
    private func dictionaryHashCode(_ dictionary: OrderedDictionary<String, String>?) -> Int {
        var hash: Int = 0
        
        if dictionary == nil {
            return hash
        }
        
        for (key, value) in dictionary! {
            hash = hash + key.hash
            hash = hash + value.hash
        }
        
        return hash
    }
    
    public override var hash: Int {
        var hash: Int = did.hash
        hash = hash + dictionaryHashCode(self.parameters)
        if self.path != nil {
            hash = hash + self.path!.hash
        }
        hash = hash + dictionaryHashCode(self.query)
        if self.fragment != nil {
            hash = hash + self.fragment!.hash
        }
        
        return hash
    }
    
}

class DURLListener: DIDURLBaseListener {
    
    public var name: String?
    public var value: String?
    public var didURL: DIDURL?
    
    init(_ didURL: DIDURL) {
        self.didURL = didURL
        self.didURL?.did = DID()
        super.init()
    }
    
    override func exitMethod(_ ctx: DIDURLParser.MethodContext) {
        let method: String = ctx.getText()
        if (method != DID.METHOD){
            // TODO: throw error
            // let error = DIDError.failue("Unknown method: \(method)")
        }
        self.didURL?.did.method = DID.METHOD
    }
    
    override func exitMethodSpecificString(_ ctx: DIDURLParser.MethodSpecificStringContext) {
        self.didURL?.did.methodSpecificId = ctx.getText()
        
    }
    
    override func enterParams(_ ctx: DIDURLParser.ParamsContext) {
        self.didURL?.parameters = OrderedDictionary()
    }
    
    override func exitParamMethod(_ ctx: DIDURLParser.ParamMethodContext) {
        let method: String = ctx.getText()
        if method != DID.METHOD {
            // TODO
            // throw new IllegalArgumentException("Unknown parameter method: " + method);
        }
    }
    
    override func exitParamQName(_ ctx: DIDURLParser.ParamQNameContext) {
        self.name = ctx.getText()
    }
    
    override func exitParamValue(_ ctx: DIDURLParser.ParamValueContext) {
        self.value = ctx.getText()
    }
    
    override func exitParam(_ ctx: DIDURLParser.ParamContext) {
        let name = self.name ?? ""
        let value = self.value ?? ""
        self.didURL?.addParameter(name, value)
        self.name = nil
        self.value = nil
    }
    
    override func exitPath(_ ctx: DIDURLParser.PathContext) {
        self.didURL?.path = "/" + ctx.getText()
    }
    
    override func enterQuery(_ ctx: DIDURLParser.QueryContext) {
        self.didURL?.query = OrderedDictionary()
    }
    
    override func exitQueryParamName(_ ctx: DIDURLParser.QueryParamNameContext) {
        self.name = ctx.getText()
    }
    
    override func exitQueryParamValue(_ ctx: DIDURLParser.QueryParamValueContext) {
        self.value = ctx.getText()
    }
    
    override func exitQueryParam(_ ctx: DIDURLParser.QueryParamContext) {
        self.didURL?.addQueryParameter(self.name, self.value)
        self.name = nil
        self.value = nil
    }
    
    override func exitFrag(_ ctx: DIDURLParser.FragContext) {
        self.didURL?.fragment = ctx.getText()
    }
}

extension Dictionary {
    var queryString: String {
        var output: String = ""
        for (key,value) in self {
            output +=  "\(key)=\(value)&"
        }
        output = String(output.dropLast())
        return output
    }
}

public struct OrderedDictionary<KeyType: Hashable, ValueType> {
    private var _dictionary: Dictionary<KeyType, ValueType>
    private var _keys: Array<KeyType>
    
    init() {
        _dictionary = [:]
        _keys = []
    }
    
    init(minimumCapacity: Int) {
        _dictionary = Dictionary<KeyType, ValueType>(minimumCapacity: minimumCapacity)
        _keys = Array<KeyType>()
    }
    
    init(_ dictionary: Dictionary<KeyType, ValueType>) {
        _dictionary = dictionary
        _keys = dictionary.keys.map { $0 }
    }
    
    subscript(key: KeyType) -> ValueType? {
        get {
            _dictionary[key]
        }
        set {
            if newValue == nil {
                self.removeValueForKey(key: key)
            } else {
                _ = self.updateValue(value: newValue!, forKey: key)
            }
        }
    }
    
    mutating func updateValue(value: ValueType, forKey key: KeyType) -> ValueType? {
        let oldValue = _dictionary.updateValue(value, forKey: key)
        if oldValue == nil {
            _keys.append(key)
        }
        return oldValue
    }
    
    mutating func removeValueForKey(key: KeyType) {
        _keys = _keys.filter {
            $0 != key
        }
        _dictionary.removeValue(forKey: key)
    }
    
    mutating func removeAll(keepCapacity: Int) {
        _keys = []
        _dictionary = Dictionary<KeyType, ValueType>(minimumCapacity: keepCapacity)
    }
    
    var count: Int {
        get {
            _dictionary.count
        }
    }
    
    // keys isn't lazy evaluated because it's just an array anyway
    var keys: [KeyType] {
        get {
            _keys
        }
    }
    
    var values: Array<ValueType> {
        get {
            _keys.map { _dictionary[$0]! }
        }
    }
    
    static func ==<Key: Equatable, Value: Equatable>(lhs: OrderedDictionary<Key, Value>, rhs: OrderedDictionary<Key, Value>) -> Bool {
        lhs._keys == rhs._keys && lhs._dictionary == rhs._dictionary
    }
    
    static func !=<Key: Equatable, Value: Equatable>(lhs: OrderedDictionary<Key, Value>, rhs: OrderedDictionary<Key, Value>) -> Bool {
        lhs._keys != rhs._keys || lhs._dictionary != rhs._dictionary
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
