
import Foundation

public protocol DIDTransaction {

    func getDid() -> DID

    func getTransactionId() -> String

    func getTimestamp() -> Date

    func getOperation() -> IDChainRequestOperation

    func getDocument() -> DIDDocument
}
