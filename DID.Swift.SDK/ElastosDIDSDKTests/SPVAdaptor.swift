
import Foundation
import ElastosDIDSDK
import SPVAdapter

public typealias PasswordCallback = (_ walletDir: String, _ walletId: String) -> String?
public class SPVAdaptor: DIDAdapter {

    var walletDir: String!
    var walletId: String!
    var network: String!
    var handle: OpaquePointer!
    public var passwordCallback: PasswordCallback?
    
    public init(_ walletDir: String, _ walletId: String, _ network: String, _ resolver: String, _ passwordCallback: @escaping PasswordCallback) {
        
       handle = SPV.create(walletDir, walletId, network, resolver)
        print(handle)
        self.walletDir = walletDir
        self.walletId = walletId
        self.network = network
        self.passwordCallback = passwordCallback
    }
    
    public func destroy() {
        SPV.destroy(handle)
        handle = nil
    }
    
    public func isAvailable() throws -> Bool {
       return SPV.isAvailable(handle)
    }
    
    public func createIdTransaction(_ payload: String, _ memo: String?) throws -> String? {
        let password = passwordCallback!(walletDir, walletId)
        if password == nil {
            return nil
        }
        let re = try SPV.createIdTransaction(handle, password!, payload, memo)
        return re
    }
    
    public func resolve(_ requestId: String, _ did: String, _ all: Bool) throws -> String? {
        return try SPV.resolve(requestId, did, all)
    }
}
