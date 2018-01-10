import Foundation

public enum CarrierError: Error {
    case InvalidArgument
    case InternalError(errno: Int)
}

extension CarrierError: LocalizedError {
    public var errorDescription: String? {
        switch self {
        case .InvalidArgument:
            return "Invalid argument"
        case .InternalError(let errno):
            return String(format: "Carrier Internal Error 0x%X", errno)
        }
    }
}

extension CarrierError: CustomNSError {
    public var errorCode: Int {
        switch self {
        case .InvalidArgument:
            return 1
        case .InternalError(let errno):
            return errno
        }
    }
}
