import Foundation

public enum DIDError: Error {
    case unknownFailure (_ desc: String? = nil)
    case illegalArgument(_ desc: String? = nil)

    case malformedDID   (_ desc: String? = nil)
    case malformedDIDURL(_ desc: String? = nil)
    case malformedDocument  (_ desc: String? = nil)
    case malformedCredential(_ desc: String? = nil)

    case didStoreError  (_ desc: String? = nil)

    case didResolveError(_ desc: String? = nil)
    case didDeactivated (_ desc: String? = nil)
    case didExpired     (_ desc: String? = nil)
    case transactionError(_ desc: String? = nil)

    case invalidState   (_ desc: String? = nil)
}

extension DIDError {
    static func des(_ error: DIDError) -> String {
        switch error {
        case .unknownFailure(let _desc):
            return _desc ?? "Unkown Failure"
        case .illegalArgument(let _desc):
            return _desc ?? "Invalid arguments"

        case .malformedDID(let _desc):
            return _desc ?? "Malformed DID string"
        case .malformedDIDURL(let _desc):
            return _desc ?? "Malformed DIDURL string"
        case .malformedDocument(let _desc):
            return _desc ?? "Malformed DID document"
        case .malformedCredential(let _desc):
            return _desc ?? "Malformed Verifiable credential"

        case .didStoreError(let _desc):
            return _desc ?? "Unkoown DIDStore error"

        case .didResolveError(let _desc):
            return _desc ?? "DID resolve failure"
        case .didDeactivated(let _desc):
            return _desc ?? "DID was deactivated"
        case .didExpired(let _desc):
            return _desc ?? "DID was expired"

        case .transactionError(let _desc):
            return _desc ?? "DIDTransaction failure"

        case .invalidState(let desc):
            return desc ?? "Invalid state internal"
        }
    }
}
