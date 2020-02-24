import Foundation

protocol DIDResolver {
    func resolve(_ requestId: String, _ did: String, _ all: Bool) throws -> Data
}
