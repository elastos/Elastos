
import Foundation

public enum DIDResolveError: Error {
    case failue(_ des: String?)
}

extension DIDResolveError {

    static func des(_ error: DIDResolveError) -> String {
        switch error {
        case .failue(let err):
            return err ?? "Operation failed"
        }
    }
}
