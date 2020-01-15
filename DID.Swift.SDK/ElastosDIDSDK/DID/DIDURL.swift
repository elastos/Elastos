import Foundation

public class DIDURL: NSObject {
    public var did: DID!
    var _parameters: OrderedDictionary<String, String>?
    public var path: String?
    var _query: OrderedDictionary<String, String>?
    public var fragment: String!
    public var meta: CredentialMeta = CredentialMeta()

    public init(_ id: DID, _ fragment: String) throws {
        super.init()
        if (fragment.count > 4) {  //TODO:  Why don't use fragment.hasPrefix("did:")
            let str = fragment.prefix(4)
            let prestr: String = String(str)
            if prestr == "did:" {            
                let listener = DURLListener(self)
                try ParserHelper.parase(fragment, false, listener)
                if (did != id) {
                    throw DIDError.failue("Missmatched arguments")
                }
                return
            }
        }
        
        self.did = id
        self.fragment = fragment
    }
    
    public init(_ url: String) throws {
        super.init()
        let listener = DURLListener(self)
        try ParserHelper.parase(url, false, listener)
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

    public var parameters: String? {
        get {
            guard _parameters != nil else {
                return nil
            }
            return mapToString(_parameters!, sep: ";")
        }
    }

    public func parameter(ofKey: String) -> String? {
        return _parameters![ofKey]
    }

    public func hasParameter(forKey: String) -> Bool {
        guard parameters != nil else {
            return false
        }
        return _parameters!.keys.contains(forKey)
    }

    public func append(newParameter: String?, forKey: String) {
        return _parameters![forKey] = newParameter ?? ""
    }

    public var queryParameters: String {
        guard _query != nil else {
            return ""
        }
        return mapToString(_query!, sep: "&")
    }

    public func queryParameter(ofKey: String) -> String? {
        guard _query == nil else {
            return _query![ofKey]
        }
        return nil
    }

    public func hasQueryParameter(forKey: String) -> Bool {
        guard _query == nil else {
            return _query!.keys.contains(forKey)
        }
        return false
    }
    
    public func append(newQueryParameter: String?, forKey: String) {
        return _query![forKey] = newQueryParameter ?? ""
    }
    
    public func setExtra(_ name: String, _ value: String) throws {
        meta.setExtra(name, value)
        if meta.attachedStore() {
            try meta.store?.storeCredentialMeta(did, self, meta)
        }
    }

    public func extraValue(ofKey: String) throws -> String? {
        return meta.getExtra(ofKey)
    }

    public var aliasName: String {
        get {
            return meta.alias
        }
        set {
            do {
                meta.alias = newValue
                if (meta.attachedStore()) {
                    try meta.store?.storeCredentialMeta(did, self, meta)
                }
            } catch {
                print(error)
            }
        }
    }
    
    public func toExternalForm() -> String {
        let testDID: String = self.did.description
        var params: String = ""
        if self.parameters != nil {
            params = ";" + mapToString(_parameters!, sep: ";")
        }
        let path = (self.path ?? "")
        var query: String = ""
        if _query != nil {
            query = "?" + mapToString(_query!, sep: "&")
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
        let param: String = (_parameters != nil) ? mapToString(_parameters!, sep: ";") : ""
        let query: String = (_query != nil) ? mapToString(_query!, sep: "&") : ""
        let method: String = did.method
        let methodSpecificId: String = did.methodSpecificId
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
        self.didURL?._parameters = OrderedDictionary()
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
        self.didURL?.append(newParameter: value, forKey: name)
        self.name = nil
        self.value = nil
    }
    
    override func exitPath(_ ctx: DIDURLParser.PathContext) {
        self.didURL?.path = "/" + ctx.getText()
    }
    
    override func enterQuery(_ ctx: DIDURLParser.QueryContext) {
        self.didURL?._query = OrderedDictionary()
    }
    
    override func exitQueryParamName(_ ctx: DIDURLParser.QueryParamNameContext) {
        self.name = ctx.getText()
    }
    
    override func exitQueryParamValue(_ ctx: DIDURLParser.QueryParamValueContext) {
        self.value = ctx.getText()
    }
    
    override func exitQueryParam(_ ctx: DIDURLParser.QueryParamContext) {
        self.didURL?.append(newQueryParameter: self.value, forKey: self.name!)
        self.name = nil
        self.value = nil
    }
    
    override func exitFrag(_ ctx: DIDURLParser.FragContext) {
        self.didURL?.fragment = ctx.getText()
    }
}


