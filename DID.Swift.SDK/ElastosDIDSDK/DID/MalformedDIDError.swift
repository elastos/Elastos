import Foundation

public enum MalformedDIDError: Error {
    case failue(_ des: String?)
}

extension MalformedDIDError {

    static func des(_ error: MalformedDIDError) -> String {
        switch error {
        case .failue(let err):
            return err ?? "Operation failed"
        }
    }
}
