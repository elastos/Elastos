import Foundation

public enum MalformedCredentialError: Error {
    case failue(_ des: String?)
}

extension MalformedCredentialError {

    static func des(_ error: MalformedCredentialError) -> String {
        switch error {
        case .failue(let err):
            return err ?? "Operation failed"
        }
    }
}
