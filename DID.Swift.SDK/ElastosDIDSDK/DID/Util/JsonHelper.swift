import Foundation

class JsonHelper {

    class func getDid(_ dic: OrderedDictionary<String, Any>, _ name: String, _ optional: Bool, _ ref: DID?, _ hint: String) throws -> DID {
        
        let vn = dic[name]
        if vn == nil {
            if (optional) { return ref! }
            else {
                throw DIDError.failue("Missing " + hint + ".")
            }
        }
        let value: String = vn as? String ?? ""
        guard value != "" else {
            throw DIDError.failue("Invalid " + hint + " value.")
        }
        
        return try DID(value)
    }

    class func getDidUrl(_ dic: OrderedDictionary<String, Any>, _ name: String, _ ref: DID, _ hint: String) throws -> DIDURL{
        let vn = dic[name]
        if vn == nil {
            throw DIDError.failue("Invalid " + hint + " value.")
        }
        var value: String = vn as? String ?? ""

        guard !value.isEmpty else {
            throw DIDError.failue("Invalid " + hint + " value.")
        }
        let fragment: String = String(value.prefix(1))
        if fragment == "#" {
            value = String(value.suffix(value.count - 1))
            return try DIDURL(ref, value)
        }
        return try DIDURL(value)
    }

    class func getString(_ dic: OrderedDictionary<String, Any>, _ name: String, _ optional: Bool, _ ref: String?, _ hint: String) throws -> String {
        let vn = dic[name]
        if vn == nil {
            if (optional) { return ref! }
        }

        let value: String = vn as? String ?? ""

        guard value != "" else {
            throw DIDError.failue("Invalid " + hint + " value.")
        }
        return value
    }
}
