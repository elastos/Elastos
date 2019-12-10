import Foundation

public enum MalformedDocumentError: Error {
    case failue(_ des: String?)
}

extension MalformedDocumentError {

    static func des(_ error: MalformedDocumentError) -> String {
        switch error {
        case .failue(let err):
            return err ?? "Operation failed"
        }
    }
}
