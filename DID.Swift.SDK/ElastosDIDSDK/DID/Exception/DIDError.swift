import Foundation

public enum DIDError: Error {
    case failue(_ _desc: String?)
    case illegalArgument(_ _desc: String?)
    case didStoreError(_desc: String?)
    case malFormedDIDError(_desc: String?)
    case malFormedDocumentError(_desc: String?)
    case malformedCredentialError(_desc: String?)
    case didResolveError(_desc: String?)
    case didDeactivatedError(_desc: String?)
    case didExpiredError(_desc: String?)
    case transactionError(_desc: String?)
}

extension DIDError {
    static func des(_ error: DIDError) -> String {
        switch error {
        case .failue(let _desc):
            return _desc ?? "Operation failed"
        case .illegalArgument(let _desc):
            return _desc ?? "Operation failed"
        case .didStoreError(let _desc):
            return _desc ?? "Operation failed"
        case .malFormedDIDError(let _desc):
            return _desc ?? "Operation failed"
        case .malFormedDocumentError(let _desc):
            return _desc ?? "Operation failed"
        case .malformedCredentialError(let _desc):
            return _desc ?? "Operation failed"
        case .didResolveError(let _desc):
            return _desc ?? "Operation failed"
        case .didDeactivatedError(let _desc):
            return _desc ?? "Operation failed"
        case .didExpiredError(let _desc):
            return _desc ?? "Operation failed"
        case .transactionError(let _desc):
            return _desc ?? "Operation failed"
        }
    }
}
