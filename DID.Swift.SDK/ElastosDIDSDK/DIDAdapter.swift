import Foundation

public protocol DIDAdapter {
    func createIdTransaction(_ payload: String,
                             _ memo: String?) throws
}
