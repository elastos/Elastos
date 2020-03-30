import Foundation

public protocol DIDAdapter {
    typealias TransactionCallback = (String, Int, String?) -> Void

    func createIdTransaction(_ payload: String,
                             _ memo: String?,
                             _ confirms: Int,
                             _ callback: @escaping TransactionCallback)
}
