
import Foundation

public class Metadata {
    
    let EXTRA_PREFIX: String = "X-"
    public var store: DIDStore?
    var extra: OrderedDictionary<String, Any> = OrderedDictionary()
    
    required init() {
        
    }
    
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
    
    func fromJson(_ json: Dictionary<String, String>) throws {
        
    }
    
    class func fromString<T: Metadata>(_ metadata: String, _ clazz: T.Type) throws -> T {
        
        let meta = clazz.init()
        if metadata == "" {
            return meta
        }
        // check as! Dictionary<String, String>
        let dic = JsonHelper.handleString(metadata) as! Dictionary<String, String>
        
        try meta.fromJson(dic)
        
        return meta
        
        /*

         Iterator<Map.Entry<String,JsonNode>> it = node.fields();
         while (it.hasNext()) {
             Map.Entry<String,JsonNode> field = it.next();
             JsonNode vn = field.getValue();
             String key = field.getKey();
             String value = vn != null ? vn.asText() : null;

             meta.setExtraInternal(key, value);
         }

         return meta;
         
         */
    }
    
    func toJson(_ json: Dictionary<String, Any>) {}
    
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
