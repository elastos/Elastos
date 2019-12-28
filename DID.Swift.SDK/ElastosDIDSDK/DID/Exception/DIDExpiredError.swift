
import Foundation

public enum DIDExpiredError: Error {
    case failue(_ des: String?)
}

extension DIDExpiredError {

    static func des(_ error: DIDExpiredError) -> String {
        switch error {
        case .failue(let err):
            return err ?? "Operation failed"
        }
    }
}
