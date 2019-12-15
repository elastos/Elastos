
import Foundation

public enum MalformedDIDURLError: Error {
    case failue(_ des: String?)
}

extension MalformedDIDURLError {

    static func des(_ error: MalformedDIDURLError) -> String {
        switch error {
        case .failue(let err):
            return err ?? "Operation failed"
        }
    }
}
