

import Foundation


public enum DIDStoreError: Error {
    case failue(_ des: String?)
}

extension DIDStoreError {

    static func des(_ error: DIDStoreError) -> String {
        switch error {
        case .failue(let err):
            return err ?? "Operation failed"
        }
    }
}
