import Foundation

public enum DIDError: Error {
    case unknownFailure (_ des: String? = nil)
    case illegalArgument(_ des: String? = nil)

    case malformedMeta  (_ des: String? = nil)
    case malformedDID   (_ des: String? = nil)
    case malformedDIDURL(_ des: String? = nil)
    case malformedDocument  (_ des: String? = nil)
    case malformedCredential(_ des: String? = nil)

    case didStoreError  (_ des: String? = nil)

    case didResolveError(_ des: String? = nil)
    case didDeactivated (_ des: String? = nil)
    case didExpired     (_ des: String? = nil)
    case transactionError(_ des: String? = nil)

    case invalidState   (_ des: String? = nil)
}

extension DIDError {
    static func desription(_ error: DIDError) -> String {
        switch error {
        case .unknownFailure(let des):
            return des ?? "Unkown Failure"
        case .illegalArgument(let des):
            return des ?? "Invalid arguments"

        case .malformedMeta(let des):
            return des ?? "Malformed metadata"
        case .malformedDID(let des):
            return des ?? "Malformed DID string"
        case .malformedDIDURL(let des):
            return des ?? "Malformed DIDURL string"
        case .malformedDocument(let des):
            return des ?? "Malformed DID document"
        case .malformedCredential(let des):
            return des ?? "Malformed Verifiable credential"

        case .didStoreError(let des):
            return des ?? "Unkoown DIDStore error"

        case .didResolveError(let des):
            return des ?? "DID resolve failure"
        case .didDeactivated(let des):
            return des ?? "DID was deactivated"
        case .didExpired(let des):
            return des ?? "DID was expired"

        case .transactionError(let des):
            return des ?? "DIDTransaction failure"

        case .invalidState(let des):
            return des ?? "Invalid state internal"
        }
    }
}
