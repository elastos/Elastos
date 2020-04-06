import Foundation
public enum IDChainRequestOperation: Int, CustomStringConvertible {
    case CREATE = 0
    case UPDATE = 1
    case DEACTIVATE

    func toString() -> String {
        let desc: String
        switch self.rawValue {
        case 0:
            desc = "create"
        case 1:
            desc = "update"
        default:
            desc = "deactivate"
        }
        return desc;
    }

    static func valueOf(_ str: String) -> IDChainRequestOperation {
        let operation: IDChainRequestOperation

        switch str.uppercased() {
        case "CREATE":
            operation = .CREATE

        case "UPDATE":
            operation = .UPDATE

        case "DEACTIVATE":
            operation = .DEACTIVATE

        default:
            operation = .DEACTIVATE
        }
        return operation
    }

    public var description: String {
        return toString()
    }
}
