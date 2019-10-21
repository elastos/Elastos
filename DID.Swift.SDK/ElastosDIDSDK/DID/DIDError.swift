import Foundation

public enum DIDError: Error {
    case failue(_ des: String?)
    case illegalArgument(_ des: String?)
}

extension DIDError {
    static func des(_ error: DIDError) -> String {
        switch error {
        case .failue(let err):
            return err ?? "Operation failed"
        case .illegalArgument(let err):
            return err ?? "Operation failed"
        }
    }
}
