
import Foundation

public class SPV {
    
    public class func create(_ walletDir: String, _ walletId: String, _ network: String, _ resolver: String) -> OpaquePointer {
        return SpvDidAdapter_Create(walletDir, walletId, network)
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

    public typealias CSpvTransactionCallbackHandler = (String, Int, String?) -> Void

    public class func createIdTransactionEx(_ handle: OpaquePointer, _ payload: String, _ memo: String?, _ confirms: Int, _ passwd: String, callback: @escaping CSpvTransactionCallbackHandler) {

        let cCallback: CSpvTransactionCallback = { (ctxid, cstatus, cmsg, ccontext) in

            let ectxt = Unmanaged<AnyObject>.fromOpaque(ccontext!)
                .takeRetainedValue() as! [AnyObject?]
            let handler = ectxt[1] as! CSpvTransactionCallbackHandler
            let status: Int = Int(cstatus)
            var txid: String = ""
            if ctxid != nil {
                txid = String(cString: ctxid!)
            }
            var msg: String? = nil
            if cmsg != nil {
                msg = String(cString: cmsg!)
            }
            handler(txid, status, msg)
        }

        let econtext: [AnyObject?] = [self, callback as AnyObject]
        let unmanaged = Unmanaged.passRetained(econtext as AnyObject)
        let cctxt = unmanaged.toOpaque()

        SpvDidAdapter_CreateIdTransactionEx(handle, payload, memo, confirms, cCallback, cctxt, passwd)
    }
}
