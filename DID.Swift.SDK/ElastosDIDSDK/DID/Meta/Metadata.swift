
import Foundation

public class Metadata {
    
    let EXTRA_PREFIX: String = "X-"
    public var store: DIDStore?
    var extra: OrderedDictionary<String, Any> = OrderedDictionary()
    
    public required init() { }
    
    func setExtraInternal(_ name: String, _ value: String) {
        if !name.starts(with: EXTRA_PREFIX) {
            return
        }
        extra[name] = value
    }
    
    public func attachedStore() -> Bool {
        return store != nil
    }

    public func setExtra(_ name: String, _ value: String) {
        setExtraInternal(EXTRA_PREFIX + name, value)
    }
    
    public func getExtra(_ name: String) -> String? {
        return extra[EXTRA_PREFIX + name] as? String
    }
    
    func fromJson(_ json: OrderedDictionary<String, Any>) throws {}
    
    class func fromString<T: Metadata>(_ metadata: String, _ clazz: T.Type) throws -> T {
        
        let meta = clazz.init()
        if metadata == "" {
            return meta
        }
        // check as! Dictionary<String, String>
        let string = JsonHelper.preHandleString(metadata)
        let dic = JsonHelper.handleString(string) as! OrderedDictionary<String, Any>
        
        try meta.fromJson(dic)
        dic.forEach { key, value in
            let v = value as! String
            meta.setExtraInternal(key, v)
        }
        return meta
    }
    
    func toJson() -> String { return "" }
    
    func description() -> String {
        if extra.count != 0 {
            let json = JsonHelper.creatJsonString(dic: extra)
            return json
        }
        return ""
    }
    
    public func merge(_ meta: Metadata) throws {
        meta.extra.forEach { key, value in
            extra[key] = value
        }
    }
}
