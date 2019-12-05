
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
    
    public class func createIdTransaction(_ handle: OpaquePointer, _ password: String, _ payload: String, _ memo: String?) throws -> Bool {

        if memo == nil {
            let rc = SpvDidAdapter_CreateIdTransaction(handle, payload, nil, password)
            return rc == 0
        }
        else {
            let cmemo = memo!.toUnsafePointerInt8()
            let rc = SpvDidAdapter_CreateIdTransaction(handle, payload, cmemo, password)
            return rc == 0
        }
    }
    
    public class func resolve(_ handle: OpaquePointer,_ did: String) throws -> String? {
        //        let re = SpvDidAdapter_Resolve(handle, "did:elastos:im4wF5ZqiWFB1ATd2JxuxW6HHzR5Ks3LUS")
        let re = SpvDidAdapter_Resolve(handle, did)
        guard re != nil else { return nil }
        return String(cString: re)
    }
}
