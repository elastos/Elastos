import Foundation

public enum ResolveResultStatus: Int, CustomStringConvertible {
    case STATUS_VALID = 0
    case STATUS_EXPIRED = 1
    case STATUS_DEACTIVATED = 2
    case STATUS_NOT_FOUND = 3

    func toString() -> String {
        let desc: String
        switch self.rawValue {
        case 0:
            desc = "valid"
        case 1:
            desc = "expired"
        case 2:
            desc = "deactivated"
        case 3:
            desc = "notfound"
        default:
            desc = "notfound"
        }
        return desc;
    }

    public var description: String {
        return toString()
    }
}
