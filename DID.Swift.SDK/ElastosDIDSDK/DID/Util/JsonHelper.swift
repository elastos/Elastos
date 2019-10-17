import Foundation

class JsonHelper {

    class func getDid(_ dic: Dictionary<String, Any>, _ name: String, _ optional: Bool, _ ref: DID?, _ hint: String) throws -> DID {
        
        let vn = dic[name]
        if vn == nil {
            if (optional) { return ref! }
            else {
                throw DIDError.failue("Missing " + hint + ".")
            }
        }

        if !(vn is String) {
            throw DIDError.failue("Invalid " + hint + " value.")
        }
        
        let value: String = vn as! String

        if value.isEmpty {
            throw DIDError.failue("Invalid " + hint + " value.")
        }
        
        return try DID(value)
    }

    class func getDate(_ dic: Dictionary<String, Any>, _ name: String, _ optional: Bool, _ ref: Date?, _ hint: String) throws -> Date? {
        let vn = dic[name]
        if vn == nil {
            if optional {
                return ref
            }
            else {
                throw DIDError.failue("Missing \(hint).")
            }
        }
        if !(vn is String) {
            throw DIDError.failue("Invalid \(hint) value.")
        }
        let value: String = String("\(vn as! String)")
        
        if value == "" {
            throw DIDError.failue("Invalid \(hint) value.")
        }
        
        let formatter = Foundation.DateFormatter()
        formatter.dateFormat = Constants.DATE_FORMAT
        let date: Date  = formatter.date(from: value) ?? Date()
        
        return date
    }

    class func getDidUrl(_ dic: Dictionary<String, Any>, _ name: String, _ ref: DID, _ hint: String) throws -> DIDURL{
        let vn = dic[name]
        if vn == nil {
            throw DIDError.failue("Invalid " + hint + " value.")
        }
        if !(vn is String) {
            throw DIDError.failue("Invalid " + hint + " value.")
        }
        let value: String = vn as! String

        if value.isEmpty {
            throw DIDError.failue("Invalid " + hint + " value.")
        }
        let index = value.index(value.startIndex, offsetBy: 1)
        let fragment: String = value.substring(to: index)
        if fragment == "#" {
            return try DIDURL(ref, value)
        }
        return try DIDURL(value)
    }

    class func getString(_ dic: Dictionary<String, Any>, _ name: String, _ optional: Bool, _ ref: String?, _ hint: String) throws -> String {
        let vn = dic[name]
        if vn == nil {
            if (optional) { return ref! }
        }

        if !(vn is String) {
            throw DIDError.failue("Invalid " + hint)
        }
        let value: String = vn as! String

        if value.isEmpty {
            throw DIDError.failue("Invalid " + hint + value)
        }
        return value
    }
    
    class func format(_ date: Date) -> String{
        var formatter = Foundation.DateFormatter()
        formatter = Foundation.DateFormatter()
        formatter.dateFormat = "YYYY-MM-dd'T'HH:mm:ss'Z'"
        formatter.timeZone = TimeZone.init(secondsFromGMT: 0)
        let localDate = formatter.string(from: date)
        return localDate
    }
}
