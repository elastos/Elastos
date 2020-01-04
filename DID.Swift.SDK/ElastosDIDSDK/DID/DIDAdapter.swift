import Foundation

public protocol DIDAdapter {
    func createIdTransaction(_ payload: String, _ memo: String?) throws -> String?
    func resolve(_ requestId: String, _ did: String, _ all: Bool) throws -> String?
}
