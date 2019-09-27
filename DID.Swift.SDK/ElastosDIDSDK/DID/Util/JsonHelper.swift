import Foundation

class JsonHelper {

    class func getDid(_ dic: Dictionary<String, Any>, _ name: String, _ optional: Bool, _ ref: DID, _ hint: String) throws -> DID {
        let vn = dic[name]
        if vn == nil {
            if (optional) { return ref }
            else { // TODO:

            }
        }

        if !(vn is String) {
            // TODO: ERROR
        }
        let value: String = vn as! String

        if value.isEmpty {
            // TODO: ERROR
        }
        return try DID(value)
    }

    class func getDate(_ dic: Dictionary<String, Any>, _ name: String, _ optional: Bool, _ ref: Date?, _ hint: String) -> Date {
        let vn = dic[name]
        if vn == nil {
            if optional {
                return ref ?? Date()
            }
            else {
                // TODO: ERROR
            }
        }
        if !(vn is String) {
            // TODO: ERROR
        }
        let value: String = vn as! String

        if value.isEmpty {
            // TODO: ERROR
        }
        // TODO: KEY- VALUE
        let dateFormatter = DateFormatter()
        dateFormatter.dateFormat = Constants.DATE_FORMAT
        dateFormatter.timeZone = NSTimeZone(name: "UTC") as TimeZone?
        return dateFormatter.date(from: value)!
    }

    class func getDidUrl(_ dic: Dictionary<String, Any>, _ name: String, _ ref: DID, _ hint: String) throws -> DIDURL{
        let vn = dic[name]
        if vn == nil {
            // TODO: ERROR
        }
        if !(vn is String) {
            // TODO: ERROR
        }
        let value: String = vn as! String

        if value.isEmpty {
            // TODO: ERROR
        }
        let count: Int = value.count - 1

        let index = value.index(value.endIndex, offsetBy: -count)
        return try DIDURL(ref, String(value.suffix(from: index)))
    }

    class func getString(_ dic: Dictionary<String, Any>, _ name: String, _ optional: Bool, _ ref: String?, _ hint: String) throws -> String {
        let vn = dic[name]
        if vn == nil {
            if (optional) { return ref! }
        }

        if !(vn is String) {
            // TODO: ERROR
        }
        let value: String = vn as! String

        if value.isEmpty {
            // TODO: ERROR
        }
        return value
    }
    
    class func format(_ date: Date) -> String{
        let formatter = DateFormatter()
        formatter.locale = Locale.init(identifier: "zh_CN")
        formatter.dateFormat = "yyyy-MM-dd HH:mm:ss"
        let date = formatter.string(from: date)
        // TODO: change to utc
        return date
    }
}
