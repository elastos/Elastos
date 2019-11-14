

import Foundation

typealias PasswordCallback = (_ walletDir: String, _ walletId: String) -> String?
class SPVAdaptor: DIDAdaptor {
    var walletDir: String!
    var walletId: String!
    var network: String!
    var handle: OpaquePointer!
    public var passwordCallback: PasswordCallback?
    
    public init(_ walletDir: String, _ walletId: String, _ network: String, _ resolver: String, _ passwordCallback: @escaping PasswordCallback) {
        handle = walletDir.withCString { cwalletDir -> OpaquePointer in
            return walletId.withCString { cwalletId -> OpaquePointer in
                return network.withCString { cnetwork -> OpaquePointer in
                    return resolver.withCString { cre -> OpaquePointer in
                        return SpvDidAdapter_Create(cwalletDir, cwalletId, cnetwork, cre)
                    }
                }
            }
        }
        print(handle)
        self.walletDir = walletDir
        self.walletId = walletId
        self.network = network
        self.passwordCallback = passwordCallback
    }
    
    public func destroy() {
        SpvDidAdapter_Destroy(handle)
        handle = nil
    }
    
    func createIdTransaction(_ payload: String, _ memo: String?) throws -> Bool {
        let password = passwordCallback!(walletDir, walletId)
        if password == nil {
            return false
        }
        
        let rc = payload.withCString { cpayload -> Int32 in
            return (memo?.withCString{ cmemo -> Int32 in
                return password!.withCString { cpassword -> Int32 in
                    return SpvDidAdapter_CreateIdTransaction(handle, cpayload, cmemo, cpassword) }
                })!
        }
        return rc == 0
    }
    
    func resolve(_ did: String) throws -> String? {
        return nil
    }
}
