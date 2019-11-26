
import Foundation

public class SPV {
    
    public class func create(_ walletDir: String, _ walletId: String, _ network: String, _ resolver: String) -> OpaquePointer {
        
        return walletDir.withCString { cwalletDir -> OpaquePointer in
            return walletId.withCString { cwalletId -> OpaquePointer in
                return network.withCString { cnetwork -> OpaquePointer in
                    return resolver.withCString { cre -> OpaquePointer in
                        return SpvDidAdapter_Create(cwalletDir, cwalletId, cnetwork, cre)
                    }
                }
            }
        }
    }
    
    public class func destroy(_ handle: OpaquePointer) {
        SpvDidAdapter_Destroy(handle)
    }
    
    public class func createIdTransaction(_ handle: OpaquePointer, _ password: String, _ payload: String, _ memo: String?) throws -> Bool {
        
        let rc = payload.withCString { cpayload -> Int32 in
            return (memo?.withCString{ cmemo -> Int32 in
                return password.withCString { cpassword -> Int32 in
                    return SpvDidAdapter_CreateIdTransaction(handle, cpayload, cmemo, cpassword) }
                })!
        }
        return rc == 0
    }
    
    public class func resolve(_ handle: OpaquePointer,_ did: String) throws -> String? {
        let cstr = did.withCString { cdid -> UnsafePointer<Int8> in
            return SpvDidAdapter_Resolve(handle, cdid)
        }
        return String(cString: cstr)
    }
}
