
import Foundation

public protocol DIDHistory {

    func getDid() -> DID

    func getsStatus() -> ResolveResultStatus

    func getAllTransactions() -> [DIDTransaction]

    func getTransactionCount() -> Int

}
