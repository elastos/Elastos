

import Foundation
import ElastosDIDSDK

class FakeConsoleAdaptor: DIDAdaptor {
    
    func createIdTransaction(_ payload: String, _ memo: String?) throws -> Bool {
        let json = JsonHelper.handleString(payload) as! OrderedDictionary<String, Any>
        let header = json["header"] as! OrderedDictionary<String, Any>
        print("Operation: \(header["operation"] ?? "")")
        let _payload = json["payload"] as! String
        let buffer: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: 100)
        let cp = _payload.withCString { re -> UnsafePointer<Int8> in
            return re
        }
        //        let _ = base64_url_decode(buffer, cp)
//        print("        \(String(cString: buffer))")
        return true
    }
    
    func resolve(_ did: String) throws -> String? {
        print("Operation: resolve")
        print("        \(did)")
        return nil
    }
}
