
import Foundation

public enum DIDDeactivatedError: Error {
    case failue(_ des: String?)
}

extension DIDDeactivatedError {

    static func des(_ error: DIDDeactivatedError) -> String {
        switch error {
        case .failue(let err):
            return err ?? "Operation failed"
        }
    }
}
