import Foundation

public class DIDURL: NSObject {
    
    public var did: DID!
    public var parameters: OrderedDictionary<String, String>?
    public var path: String?
    public var query: OrderedDictionary<String, String>?
    public var fragment: String!
    private var listener: DURLListener?
    private var url: String!
    
    public init(_ id: DID, _ fragment: String) throws {
        self.did = id
        self.fragment = fragment
    }
    
    public init(_ url: String) throws {
        super.init()
        self.listener = DURLListener(self)
        try ParserHelper.parase(url, false, self.listener!)
        self.url = url
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
        return parameters![name]
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
        guard query == nil else {
            return query![name]
        }
        return nil
    }
    
    public func hasQueryParameter(_ name: String) -> Bool {
        guard query == nil else {
            return query!.keys.contains(name)
        }
        return false
    }
    
    public func toExternalForm() -> String {
        let testDID: String = self.did.description
        var params: String = ""
        if self.parameters != nil {
            params = ";" + mapToString(self.parameters!, sep: ";")
        }
        let path = (self.path ?? "")
        var query: String = ""
        if self.query != nil {
            query = "?" + mapToString(self.query!, sep: "&")
        }
        var fragment = self.fragment ?? ""
        if self.fragment != nil {
            fragment = "#" + fragment
        }
        return testDID + params + path + query + fragment
    }
    
    public override var description: String {
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
        let param: String = (self.parameters != nil) ? mapToString(self.parameters!, sep: ";") : ""
        let query: String = (self.query != nil) ? mapToString(self.query!, sep: "&") : ""
        let method: String = (did.method != nil) ? String("\(did.method)") : ""
        let methodSpecificId: String = (did.methodSpecificId != nil) ? String("\(did.methodSpecificId)") : ""
        let path: String = (self.path != nil) ? self.path! : ""
        let fragment: String = (self.fragment != nil) ? self.fragment! : ""
        
        let hash: Int = String(method + methodSpecificId + param + query + path + fragment).hash
        
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
            print("Unknown method: \(method)")
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
            print("Unknown parameter method: \(method)")
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

