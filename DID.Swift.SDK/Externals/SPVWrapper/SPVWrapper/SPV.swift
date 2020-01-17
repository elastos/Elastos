
import Foundation

public class SPV {
    
    public class func create(_ walletDir: String, _ walletId: String, _ network: String, _ resolver: String) -> OpaquePointer {
        return SpvDidAdapter_Create(walletDir, walletId, network, resolver)
    }
    
    public class func destroy(_ handle: OpaquePointer) {
        SpvDidAdapter_Destroy(handle)
    }
    
    public class func isAvailable(_ handle: OpaquePointer) -> Bool {
        let rc = SpvDidAdapter_IsAvailable(handle)
        return rc == 1
    }
    
    public class func createIdTransaction(_ handle: OpaquePointer, _ password: String, _ payload: String, _ memo: String?) -> String? {
        let re = SpvDidAdapter_CreateIdTransaction(handle, payload, memo, password)
        if re != nil {
           return String(cString: re!)
        }
        return nil
    }
}
